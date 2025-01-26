import serial
import struct
from enum import Enum
import time
import zlib
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
    if len(response) < BIN_MODE_RESPONSE_LENGTH:
        raise ValueError("Response too short")

    header1, header0, status, command_id = struct.unpack('<BBBB', response[:4])
    
    if header1 != BINARY_HEADER_2 or header0 != BINARY_HEADER_1:
        raise ValueError("Invalid response headers")

    crc_received = struct.unpack('<I', response[-4:])[0]
    crc_calculated = zlib.crc32(response[:-4]) & 0xFFFFFFFF
    if crc_received != crc_calculated:
        raise ValueError("CRC mismatch")

    return status, command_id


def send_binary_packet(serial_port, packet):
    with serial.Serial(serial_port, baudrate=115200, timeout=1) as ser:
        print(f"Sending binary packet to {serial_port}...")
        ser.write(packet)
        response = ser.read(BIN_MODE_RESPONSE_LENGTH)
        print("Packet sent and response received!")
        return response


def sync_time(serial_port):
    current_time_us = int(time.time() * MICROSECONDS_IN_SECOND_COUNT)  # Current epoch time in microseconds
    payload = struct.pack('<Q', current_time_us)    # 64-bit payload

    packet = create_binary_packet(CommandType.WRITE, CommandID.SYNC_TIME, payload)
    response = send_binary_packet(serial_port, packet)

    status, command_id = parse_response(response)
    if command_id != CommandID.SYNC_TIME.value:
        raise ValueError("Mismatched command ID in response")

    print(f"Sync time status: {status}")


def main():
    pico_serial_port = find_pico_device()

    sync_time(pico_serial_port)


if __name__ == "__main__":
    main()
