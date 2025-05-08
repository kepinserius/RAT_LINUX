# Educational RAT - Technical Documentation

This document provides an overview of the technical design and architecture of the educational RAT project.

## Architecture Overview

The project consists of two main components:

1. **Server** - The command and control server that manages connections to clients and sends commands
2. **Client** - The client component that runs on target systems and receives commands from the server

Both components share common code for networking, encryption, and protocol handling.

## Communication Protocol

### Packet Structure

All communication uses a custom binary protocol with the following packet structure:

1. **Packet Header**:

   - `magic` (uint32_t) - Magic number (0x52415420 = "RAT ")
   - `command` (uint16_t) - Command code
   - `payload_length` (uint32_t) - Length of following payload
   - `sequence` (uint32_t) - Sequence number
   - `checksum` (uint32_t) - CRC32 checksum of payload

2. **Payload**:
   - Command-specific data

### Response Structure

Responses from clients use this structure:

1. **Response Header**:

   - `status` (uint16_t) - Response status code
   - `payload_length` (uint32_t) - Length of following payload
   - `sequence` (uint32_t) - Matching sequence number from request
   - `checksum` (uint32_t) - CRC32 checksum of payload

2. **Payload**:
   - Command-specific response data

### Commands

The protocol supports the following commands:

| Command            | Code | Description               |
| ------------------ | ---- | ------------------------- |
| PING               | 0x01 | Check if client is alive  |
| SHELL_EXEC         | 0x02 | Execute shell command     |
| FILE_LIST          | 0x03 | List files in directory   |
| FILE_UPLOAD        | 0x04 | Upload file to client     |
| FILE_DOWNLOAD      | 0x05 | Download file from client |
| SCREENSHOT         | 0x06 | Take screenshot           |
| KEYLOG_START       | 0x07 | Start keylogger           |
| KEYLOG_STOP        | 0x08 | Stop keylogger            |
| KEYLOG_DUMP        | 0x09 | Get keylogger data        |
| PERSISTENCE        | 0x0A | Install persistence       |
| REMOVE_PERSISTENCE | 0x0B | Remove persistence        |
| SYSTEM_INFO        | 0x0C | Get system information    |
| PROCESS_LIST       | 0x0D | List running processes    |
| PROCESS_KILL       | 0x0E | Kill a process            |
| WEBCAM_CAPTURE     | 0x0F | Capture image from webcam |
| MIC_RECORD         | 0x10 | Record from microphone    |
| EXIT               | 0xFF | Exit client               |

### Response Status Codes

| Status                  | Code | Description                   |
| ----------------------- | ---- | ----------------------------- |
| SUCCESS                 | 0    | Command executed successfully |
| ERROR_GENERAL           | 1    | General error                 |
| ERROR_NOT_IMPLEMENTED   | 2    | Command not implemented       |
| ERROR_PERMISSION_DENIED | 3    | Permission denied             |
| ERROR_NOT_FOUND         | 4    | Resource not found            |

## Security Features

### Encryption

All communication is encrypted using AES-256-CBC with the following process:

1. A random 256-bit key is generated on connection establishment
2. Each packet's payload is encrypted before transmission
3. The IV (Initialization Vector) is prepended to the encrypted payload
4. The CRC32 checksum provides data integrity verification

### Connection Handling

The server manages multiple client connections with these features:

1. Thread-safe client management
2. Automatic removal of disconnected clients
3. Command broadcasting to all connected clients
4. Unique client identification based on random strings

## Core Components

### Common

- `protocol.h` - Defines the communication protocol structures and constants
- `crypto.h` - Provides encryption and decryption functionality
- `network.h` - Handles network communication
- `utils.h` - Utility functions for checksums, random strings, etc.

### Server

- `server.h/cpp` - Manages client connections and command dispatch
- `main.cpp` - Server entry point with command-line interface

### Client

- `client.h/cpp` - Handles connections to the server and command execution
- `main.cpp` - Client entry point with reconnection logic

## Implementation Details

### Server Implementation

The server uses the following approach:

1. Listens for incoming connections on the specified port
2. Creates a `ClientHandler` object for each connected client
3. Spawns a thread to handle commands from each client
4. Provides a command-line interface for controlling clients

### Client Implementation

The client uses the following approach:

1. Connects to the server using the specified address and port
2. Sends system information to the server
3. Listens for commands from the server in a separate thread
4. Executes commands and sends responses back to the server
5. Automatically reconnects if the connection is lost

## Educational Notes

This project is deliberately simplified for educational purposes. In a real-world scenario:

1. Advanced obfuscation would be used to hide the client's presence
2. Anti-analysis techniques would be implemented to evade detection
3. More sophisticated persistence mechanisms would be used
4. Additional evasion techniques would be employed

These features are intentionally omitted to keep the project educational and to avoid creating actual malware.
