import serial
import struct
from enum import Enum
import time
import zlib
import argparse
from utils import find_pico_device
import logging
import cstruct

MICROSECONDS_IN_SECOND_COUNT = 1_000_000

UART_BAUD_RATE = 115200

BINARY_HEADER_1 = 0xAA
BINARY_HEADER_2 = 0xBB

BIN_MODE_PAYLOAD_LENGTH_FIELD_SIZE_BYTES = 4
BIN_MODE_RESPONSE_HEADER_SIZE_BYTES = 4 + BIN_MODE_PAYLOAD_LENGTH_FIELD_SIZE_BYTES
BIN_MODE_CRC_32_SIZE_BYTES = 4


class CommandType(Enum):
    WRITE = 0x01
    READ = 0x02


class CommandID(Enum):
    SYNC_TIME = 0x01
    GET_TIME_REPORT = 0x02
    GET_CURRENT_SESSION_ID = 0x03
    NEW_SESSION = 0x04
    SET_MEDIUM_THRESHOLD = 0x05
    SET_LONG_THRESHOLD = 0x06


class DateTime(cstruct.CStruct):
    __byte_order__ = cstruct.LITTLE_ENDIAN
    __struct__ = """
        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
    """

    year: int
    month: int
    day: int
    hour: int
    minute: int
    second: int


class TimeTrackingEntry(cstruct.CStruct):
    __byte_order__ = cstruct.LITTLE_ENDIAN
    __struct__ = """
        uint64_t start_time_us;
        uint64_t work_time_us;
        uint64_t meeting_time_us;
        uint8_t tracking_work;  // Replaced bool with uint8_t
        uint8_t tracking_meetings;  // Replaced bool with uint8_t
        uint8_t medium_threshold_reached;  // Replaced bool with uint8_t
        uint8_t long_threshold_reached;  // Replaced bool with uint8_t
        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        uint8_t reserved[4];
    """

    start_time_us: int
    work_time_us: int
    meeting_time_us: int
    tracking_work: bool
    tracking_meetings: bool
    medium_threshold_reached: bool
    long_threshold_reached: bool
    tracking_date: DateTime


def create_binary_packet(command_type, command_id, payload):
    packet = bytearray()
    packet.append(BINARY_HEADER_1)
    packet.append(BINARY_HEADER_2)
    packet.append(command_type.value)
    packet.append(command_id.value)

    payload_length = len(payload)
    packet.extend(struct.pack('<I', payload_length))
    packet.extend(payload)

    crc32 = zlib.crc32(packet) & 0xFFFFFFFF
    packet.extend(struct.pack('<I', crc32))

    return packet


def parse_response(response):
    if len(response) < BIN_MODE_RESPONSE_HEADER_SIZE_BYTES:
        raise ValueError("Response too short")

    header1, header0, status, command_id = struct.unpack('<BBBB', response[:4])
    
    if header1 != BINARY_HEADER_2 or header0 != BINARY_HEADER_1:
        raise ValueError("Invalid response headers")

    payload_length = struct.unpack('<I', response[4:8])[0]
    if len(response) != BIN_MODE_RESPONSE_HEADER_SIZE_BYTES + payload_length + BIN_MODE_CRC_32_SIZE_BYTES:
        raise ValueError(f"Invalid response length: expected {BIN_MODE_RESPONSE_HEADER_SIZE_BYTES + payload_length + BIN_MODE_CRC_32_SIZE_BYTES}, got {len(response)}")

    crc_received = struct.unpack('<I', response[-4:])[0]
    crc_calculated = zlib.crc32(response[:-4]) & 0xFFFFFFFF
    if crc_received != crc_calculated:
        raise ValueError("CRC mismatch")

    return status, command_id


def send_binary_packet(serial_port, packet):
    with serial.Serial(serial_port, baudrate=UART_BAUD_RATE, timeout=1000) as ser:
        ser.write(packet)
        response = ser.read(BIN_MODE_RESPONSE_HEADER_SIZE_BYTES)
        
        if len(response) < BIN_MODE_RESPONSE_HEADER_SIZE_BYTES:
            raise ValueError(f"Response too short: expected at least {BIN_MODE_RESPONSE_HEADER_SIZE_BYTES} bytes, got {len(response)}")

        payload_length = struct.unpack('<I', response[4:8])[0]
        expected_response_length = BIN_MODE_RESPONSE_HEADER_SIZE_BYTES + payload_length + BIN_MODE_CRC_32_SIZE_BYTES

        response += ser.read(expected_response_length - len(response))
        
        if len(response) != expected_response_length:
            raise ValueError(f"Incomplete response received: expected {expected_response_length} bytes, got {len(response)}")

        return response


# ---------------------------------------------------------------------------- #
#                               Commands Handling                              #
# ---------------------------------------------------------------------------- #

def sync_time(serial_port):
    current_time_us = int(time.time() * MICROSECONDS_IN_SECOND_COUNT)  # Current epoch time in microseconds
    payload = struct.pack('<Q', current_time_us)    # 64-bit payload

    packet = create_binary_packet(CommandType.WRITE, CommandID.SYNC_TIME, payload)
    response = send_binary_packet(serial_port, packet)

    status, command_id = parse_response(response)
    if command_id != CommandID.SYNC_TIME.value:
        raise ValueError("Mismatched command ID in response")

    log.info(f"Sync time status: {status}")


def get_time_report(serial_port, session_id=None):
    payload = struct.pack('<I', session_id) if session_id is not None else struct.pack('<I', 0xFFFFFFFF)

    packet = create_binary_packet(CommandType.READ, CommandID.GET_TIME_REPORT, payload)
    response = send_binary_packet(serial_port, packet)

    status, command_id = parse_response(response)
    if command_id != CommandID.GET_TIME_REPORT.value:
        raise ValueError("Mismatched command ID in response")

    if status != 0:
        raise ValueError(f"Failed to get time report: {status}")

    payload_length = struct.unpack('<I', response[4:8])[0]
    time_report_data = response[8:8 + payload_length]
    parse_time_report_response(time_report_data)


def parse_time_report_response(response):
    if len(response) < TimeTrackingEntry.size:
        raise ValueError("Response too short to contain TimeTrackingEntry_t")

    entry = TimeTrackingEntry()
    entry.unpack(response[:TimeTrackingEntry.sizeof()])

    work_time_s = entry.work_time_us // 1_000_000
    meeting_time_s = entry.meeting_time_us // 1_000_000
    total_time_s = work_time_s + meeting_time_s

    work_hours, work_minutes, work_seconds = work_time_s // 3600, (work_time_s % 3600) // 60, work_time_s % 60
    meeting_hours, meeting_minutes, meeting_seconds = meeting_time_s // 3600, (meeting_time_s % 3600) // 60, meeting_time_s % 60
    total_hours, total_minutes, total_seconds = total_time_s // 3600, (total_time_s % 3600) // 60, total_time_s % 60

    log.info(f"Work Time: {work_hours}h {work_minutes}m {work_seconds}s")
    log.info(f"Meetings Time: {meeting_hours}h {meeting_minutes}m {meeting_seconds}s")
    log.info(f"Total Time: {total_hours}h {total_minutes}m {total_seconds}s")


def get_current_session_id(serial_port):
    packet = create_binary_packet(CommandType.READ, CommandID.GET_CURRENT_SESSION_ID, b'')
    response = send_binary_packet(serial_port, packet)

    status, command_id = parse_response(response)
    if command_id != CommandID.GET_CURRENT_SESSION_ID.value:
        raise ValueError("Mismatched command ID in response")

    if status != 0:
        raise ValueError(f"Failed to get current session ID: {status}")

    payload_length = struct.unpack('<I', response[4:8])[0]
    session_id_data = response[8:8 + payload_length]
    session_id = struct.unpack('<I', session_id_data)[0]
    log.info(f"Current session ID: {session_id}")


def new_session(serial_port):
    packet = create_binary_packet(CommandType.WRITE, CommandID.NEW_SESSION, b'')
    response = send_binary_packet(serial_port, packet)

    status, command_id = parse_response(response)
    if command_id != CommandID.NEW_SESSION.value:
        raise ValueError("Mismatched command ID in response")

    if status != 0:
        raise ValueError(f"Failed to create new session: {status}")


def set_threshold(serial_port, threshold, type="medium"):
    if type == "medium":
        command_id = CommandID.SET_MEDIUM_THRESHOLD
    elif type == "long":
        command_id = CommandID.SET_LONG_THRESHOLD
    else:
        raise ValueError("Invalid threshold type. Use 'medium' or 'long'.")

    payload = struct.pack('<I', threshold)
    packet = create_binary_packet(CommandType.WRITE, command_id, payload)
    response = send_binary_packet(serial_port, packet)

    status, command_id = parse_response(response)
    if command_id != command_id.value:
        raise ValueError("Mismatched command ID in response")

    if status != 0:
        raise ValueError(f"Failed to set {type} threshold: {status}")


def set_threshold_command(serial_port, threshold_type, time_value):
    try:
        hours, minutes = map(int, time_value.split(","))
    except ValueError:
        raise ValueError("Invalid time format. Use 'H,M' where H is hours and M is minutes.")

    if hours < 0 or minutes < 0 or minutes >= 60:
        raise ValueError("Invalid time input. Hours must be >= 0, and minutes must be between 0 and 59.")

    threshold_us = (hours * 3600 + minutes * 60) * MICROSECONDS_IN_SECOND_COUNT
    set_threshold(serial_port, threshold_us, type=threshold_type)
    log.info(f"Set {threshold_type} threshold to {hours} hours and {minutes} minutes ({threshold_us} microseconds).")

# ---------------------------------------------------------------------------- #

def parse_arguments():
    parser = argparse.ArgumentParser(description="3-key Time Report Tool")
    subparsers = parser.add_subparsers(dest="command", required=True, help="Available commands")

    subparsers.add_parser("sync_time", help="Synchronize time with the device")

    get_time_report_parser = subparsers.add_parser("get_time_report", help="Get time report")
    get_time_report_parser.add_argument("-s", "--session_id", type=int, default=None,
                                        help="Session ID for the 'get_time_report' command (optional)")

    subparsers.add_parser("get_current_session_id", help="Get the current session ID")

    subparsers.add_parser("new_session", help="Start a new session")

    set_threshold_parser = subparsers.add_parser("set_threshold", help="Set a threshold")
    set_threshold_parser.add_argument("threshold_type", choices=["medium", "long"],
                                      help="Threshold type (medium or long)")
    set_threshold_parser.add_argument("time_value", help="Threshold time in 'H,M' format (e.g., '3,50')")

    return parser.parse_args()


def configure_logging():
    logging.basicConfig(filename="time_report.log", level=logging.INFO, 
                        format="%(asctime)s - %(levelname)s - %(message)s")
    global log
    log = logging.getLogger()

    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.INFO)
    console_formatter = logging.Formatter("%(asctime)s - %(levelname)s - %(message)s")
    console_handler.setFormatter(console_formatter)
    log.addHandler(console_handler)


def main():
    configure_logging()
    args = parse_arguments()
    pico_serial_port = find_pico_device()

    if args.command == "sync_time":
        sync_time(pico_serial_port)
    elif args.command == "get_time_report":
        get_time_report(pico_serial_port, args.session_id)
    elif args.command == "get_current_session_id":
        get_current_session_id(pico_serial_port)
    elif args.command == "new_session":
        new_session(pico_serial_port)
    elif args.command == "set_threshold":
        set_threshold_command(pico_serial_port, args.threshold_type, args.time_value)


if __name__ == "__main__":
    main()
