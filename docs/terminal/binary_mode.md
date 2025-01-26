# Terminal Binary Mode

Terminal Binary Mode provides an efficient, structured protocol for exchanging commands and data between the terminal and the device. It uses binary packets to achieve low overhead and high-speed communication.

---

## Overview of Binary Mode

Binary Mode is activated when the terminal detects a specific header byte sequence. Once active, the terminal operates in Binary Mode until a complete packet is processed or the communication is reset. Binary packets contain a structured header, optional payload, and a CRC32 checksum for validation.

---

## Packet Structure

Binary packets are structured as follows:

| Byte(s) | Description                  |
|---------|------------------------------|
| 0       | **Header 1**: Start byte (fixed value) |
| 1       | **Header 2**: Second start byte (fixed value) |
| 2       | **Command Type**: Specifies whether the operation is a `READ` or `WRITE`. |
| 3       | **Command ID**: Identifies the specific command being executed. |
| 4–7     | **Payload Length**: Length of the payload in bytes (little-endian). |
| 8–N     | **Payload**: Command-specific data (optional, varies by command). |
| N+1–N+4 | **CRC32**: Checksum for error detection (covers header and payload). |

---

## Command Payload Structure

Each command can define its own payload structure. Here’s the general outline:

1. **Write Mode (`Command Type = WRITE`)**:
   - Sends data from the terminal to the device.
   - Payload contains command-specific information.

2. **Read Mode (`Command Type = READ`)**:
   - Requests data from the device.
   - Typically does not require a payload.

### Example: `SYNC_TIME` Command Payload (Write Mode)

| Byte(s) | Description                 |
|---------|-----------------------------|
| 0–7     | Timestamp (64-bit, little-endian). |

---

### Response Structure

Responses sent by the device follow the same basic structure as requests but without a payload unless the command specifies one.

| Byte(s) | Description                  |
|---------|------------------------------|
| 0       | **Header 1**: Start byte. |
| 1       | **Header 2**: Second start byte. |
| 2       | **Status**: Command execution status (`SUCCESS`, `INVALID_PAYLOAD`, etc.). |
| 3       | **Command ID**: ID of the command being responded to. |
| 4–7     | **Payload Length**: Length of the payload (usually `0x00 0x00 0x00 0x00` if no payload). |
| 8–11    | **CRC32**: Checksum covering the response.

### Common Response Status Codes

| Status Code | Meaning                     |
|-------------|-----------------------------|
| `0x00`      | `SUCCESS`: Command executed successfully. |
| `0x01`      | `INVALID_PAYLOAD`: Payload size or format is invalid. |
| `0x02`      | `UNSUPPORTED_CMP_TYPE`: Unsupported command type (e.g., `READ` for `SYNC_TIME`). |

---

## Commands and Descriptions

The following commands are currently supported:

### 1. `SYNC_TIME`
Synchronizes the device's internal time with the provided timestamp.

- **Command Type**: `WRITE`
- **Command ID**: `0x01`
- **Payload**:
    - **Bytes 0–7**: Timestamp in microseconds (64-bit, little-endian).

- **Response**
    - **Success**: The device sets its internal time and returns a success response
    - **Failure**: Returns an error status if the payload is invalid or the command type is unsupported

## Example Workflow

### Synchronizing Time

1. **Send Request**
    - Command: `SYNC_TIME`
    - Command Type: `WRITE`
    - Payload: Timestamp (e.g., `0x00 0x00 0x1A 0xE1 0x4E 0x6C 0x00 0x00` for `1700000000000` microseconds)

2. **Device Processes**
    - Validates the header, payload, and CRC32
    - Sets its internal clock to the provided timestamp

3. **Receive Response**
    - Success Response:
     ```
      Header 1: 0xAA
      Header 2: 0xBB
      Status: 0x00 (SUCCESS)
      Command ID: 0x01 (SYNC_TIME)
      Payload Length: 0x00 0x00 0x00 0x00
      CRC32: Calculated checksum.
     ```

---

This document outlines the structure, functionality, and use cases of Terminal Binary Mode, providing a clear understanding for developers. For advanced scenarios, consult the implementation details or reach out to support.
