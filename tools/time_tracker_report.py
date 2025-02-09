import serial
import struct
import argparse
import time
import zlib
from enum import Enum
from utils import find_pico_device

MICROSECONDS_IN_SECOND_COUNT = 1_000_000

BINARY_HEADER_1 = 0xAA
BINARY_HEADER_2 = 0xBB

BIN_MODE_RESPONSE_HEADER_SIZE_BYTES = 4
BIN_MODE_PAYLOAD_LENGTH_FIELD_SIZE_BYTES = 4
BIN_MODE_CRC_32_SIZE_BYTES = 4
BIN_MODE_RESPONSE_LENGTH = BIN_MODE_RESPONSE_HEADER_SIZE_BYTES + \
                           BIN_MODE_PAYLOAD_LENGTH_FIELD_SIZE_BYTES + \
                           BIN_MODE_CRC_32_SIZE_BYTES


class CommandType(Enum):
    WRITE = 0x01
    READ = 0x02


class CommandID(Enum):
    SYNC_TIME = 0x01
    GET_TIME_REPORT = 0x02


def create_binary_packet(command_type, command_id, payload=b""):
    # Creates a binary packet with headers, payload, and CRC32 checksum
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
    # Parses and validates a binary response
    if len(response) < BIN_MODE_RESPONSE_LENGTH:
        raise ValueError("Response too short")

    header1, header0, status, command_id = struct.unpack('<BBBB', response[:4])
    
    if header1 != BINARY_HEADER_2 or header0 != BINARY_HEADER_1:
        raise ValueError("Invalid response headers")

    crc_received = struct.unpack('<I', response[-4:])[0]
    crc_calculated = zlib.crc32(response[:-4]) & 0xFFFFFFFF
    if crc_received != crc_calculated:
        raise ValueError("CRC mismatch")

    payload_length = struct.unpack('<I', response[4:8])[0]
    payload = response[8:-4] if payload_length > 0 else b""

    return status, command_id, payload


def send_binary_packet(serial_port, packet):
    # Sends a binary packet and receives the response from the device
    with serial.Serial(serial_port, baudrate=115200, timeout=1) as ser:
        print(f"Sending binary packet to {serial_port}...")
        ser.write(packet)

        response_header = ser.read(BIN_MODE_RESPONSE_HEADER_SIZE_BYTES + BIN_MODE_PAYLOAD_LENGTH_FIELD_SIZE_BYTES)

        if len(response_header) < (BIN_MODE_RESPONSE_HEADER_SIZE_BYTES + BIN_MODE_PAYLOAD_LENGTH_FIELD_SIZE_BYTES):
            raise ValueError("Response header too short")

        payload_length = struct.unpack('<I', response_header[4:8])[0]
        response_payload = ser.read(payload_length)
        response_crc = ser.read(BIN_MODE_CRC_32_SIZE_BYTES)
        response = response_header + response_payload + response_crc

        print(f"Packet sent and response received! Payload Length: {payload_length} bytes")
        return response


def sync_time(serial_port):
    # Synchronizes the time with the device
    current_time_us = int(time.time() * MICROSECONDS_IN_SECOND_COUNT)  # Current time in microseconds
    payload = struct.pack('<Q', current_time_us)

    packet = create_binary_packet(CommandType.WRITE, CommandID.SYNC_TIME, payload)
    response = send_binary_packet(serial_port, packet)

    status, command_id, _ = parse_response(response)
    if command_id != CommandID.SYNC_TIME.value:
        raise ValueError("Mismatched command ID in response")

    print(f"Sync time status: {status}")


def get_time_report(serial_port):
    # Requests the current time from the device
    packet = create_binary_packet(CommandType.READ, CommandID.SYNC_TIME)
    response = send_binary_packet(serial_port, packet)

    status, command_id, payload = parse_response(response)
    if command_id != CommandID.SYNC_TIME.value:
        raise ValueError("Mismatched command ID in response")

    if len(payload) != 8:
        raise ValueError("Invalid payload size for time report")

    current_time_us = struct.unpack('<Q', payload)[0]
    current_time_s = current_time_us / MICROSECONDS_IN_SECOND_COUNT

    print(f"Device current time: {current_time_s:.6f} seconds")


def main():
    # Command-line interface for time synchronization and retrieval
    parser = argparse.ArgumentParser(description="Time Tracker Report Tool")
    parser.add_argument("--command", "-c", choices=["sync_time", "get_time_report"], required=True,
                        help="Command to execute (sync_time or get_time_report)")

    args = parser.parse_args()
    pico_serial_port = find_pico_device()

    if args.command == "sync_time":
        sync_time(pico_serial_port)
    elif args.command == "get_time_report":
        get_time_report(pico_serial_port)


if __name__ == "__main__":
    main()
