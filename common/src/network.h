#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>

#include "protocol.h"
#include "crypto.h"
#include "utils.h"

class Connection
{
private:
    int socket_fd;
    std::string remote_address;
    int remote_port;
    std::string encryption_key;
    bool is_encrypted;
    std::mutex send_mutex;
    std::mutex recv_mutex;

public:
    Connection(int fd, const std::string &addr, int port, bool encrypted = false)
        : socket_fd(fd), remote_address(addr), remote_port(port), is_encrypted(encrypted)
    {

        if (is_encrypted)
        {
            encryption_key = Crypto::generateKey();
        }
    }

    ~Connection()
    {
        if (socket_fd >= 0)
        {
            close(socket_fd);
        }
    }

    bool isConnected() const
    {
        if (socket_fd < 0)
            return false;

        // Try to peek at the socket to see if it's still connected
        char buffer;
        int result = recv(socket_fd, &buffer, 1, MSG_PEEK | MSG_DONTWAIT);

        if (result == 0)
        {
            // Connection closed
            return false;
        }
        else if (result < 0)
        {
            // Check if error is because there's no data (which is fine)
            return (errno == EAGAIN || errno == EWOULDBLOCK);
        }

        return true;
    }

    void disconnect()
    {
        if (socket_fd >= 0)
        {
            close(socket_fd);
            socket_fd = -1;
        }
    }

    // Send raw data over socket
    bool sendRaw(const std::vector<uint8_t> &data)
    {
        std::lock_guard<std::mutex> lock(send_mutex);

        size_t total_sent = 0;
        size_t remaining = data.size();

        while (total_sent < data.size())
        {
            ssize_t sent = send(socket_fd, data.data() + total_sent, remaining, 0);

            if (sent <= 0)
            {
                if (errno == EINTR)
                    continue; // Interrupted, try again
                return false; // Error in sending
            }

            total_sent += sent;
            remaining -= sent;
        }

        return true;
    }

    // Receive raw data from socket
    std::vector<uint8_t> recvRaw(size_t length)
    {
        std::lock_guard<std::mutex> lock(recv_mutex);

        std::vector<uint8_t> data(length);
        size_t total_received = 0;

        while (total_received < length)
        {
            ssize_t received = recv(socket_fd, data.data() + total_received,
                                    length - total_received, 0);

            if (received <= 0)
            {
                if (errno == EINTR)
                    continue; // Interrupted, try again
                throw std::runtime_error("Connection closed during receive");
            }

            total_received += received;
        }

        return data;
    }

    // Send packet with command and payload
    bool sendPacket(uint16_t command, const std::vector<uint8_t> &payload)
    {
        static uint32_t sequence = 0;

        // Encrypt payload if encryption is enabled
        std::vector<uint8_t> final_payload;
        if (is_encrypted && !payload.empty())
        {
            final_payload = Crypto::encrypt(payload, encryption_key);
        }
        else
        {
            final_payload = payload;
        }

        // Create packet header
        PacketHeader header;
        header.magic = PROTOCOL_MAGIC;
        header.command = command;
        header.payload_length = final_payload.size();
        header.sequence = ++sequence;
        header.checksum = Utils::calculateCRC32(final_payload);

        // Combine header and payload
        std::vector<uint8_t> packet(sizeof(header) + final_payload.size());
        std::memcpy(packet.data(), &header, sizeof(header));

        if (!final_payload.empty())
        {
            std::memcpy(packet.data() + sizeof(header), final_payload.data(), final_payload.size());
        }

        return sendRaw(packet);
    }

    // Receive packet and return command and payload
    std::pair<uint16_t, std::vector<uint8_t>> recvPacket()
    {
        // Receive header
        auto header_data = recvRaw(sizeof(PacketHeader));
        if (header_data.size() < sizeof(PacketHeader))
        {
            throw std::runtime_error("Failed to receive packet header");
        }

        // Parse header
        PacketHeader header;
        std::memcpy(&header, header_data.data(), sizeof(header));

        // Validate magic number
        if (header.magic != PROTOCOL_MAGIC)
        {
            throw std::runtime_error("Invalid packet magic number");
        }

        // Receive payload if any
        std::vector<uint8_t> payload;
        if (header.payload_length > 0)
        {
            payload = recvRaw(header.payload_length);

            // Validate checksum
            uint32_t checksum = Utils::calculateCRC32(payload);
            if (checksum != header.checksum)
            {
                throw std::runtime_error("Packet checksum validation failed");
            }

            // Decrypt payload if encryption is enabled
            if (is_encrypted)
            {
                payload = Crypto::decrypt(payload, encryption_key);
            }
        }

        return std::make_pair(header.command, payload);
    }

    // Send response
    bool sendResponse(uint16_t status, uint32_t sequence, const std::vector<uint8_t> &payload)
    {
        // Encrypt payload if encryption is enabled
        std::vector<uint8_t> final_payload;
        if (is_encrypted && !payload.empty())
        {
            final_payload = Crypto::encrypt(payload, encryption_key);
        }
        else
        {
            final_payload = payload;
        }

        // Create response header
        ResponseHeader header;
        header.status = status;
        header.payload_length = final_payload.size();
        header.sequence = sequence;
        header.checksum = Utils::calculateCRC32(final_payload);

        // Combine header and payload
        std::vector<uint8_t> packet(sizeof(header) + final_payload.size());
        std::memcpy(packet.data(), &header, sizeof(header));

        if (!final_payload.empty())
        {
            std::memcpy(packet.data() + sizeof(header), final_payload.data(), final_payload.size());
        }

        return sendRaw(packet);
    }

    // Receive response
    std::pair<uint16_t, std::vector<uint8_t>> recvResponse()
    {
        // Receive header
        auto header_data = recvRaw(sizeof(ResponseHeader));
        if (header_data.size() < sizeof(ResponseHeader))
        {
            throw std::runtime_error("Failed to receive response header");
        }

        // Parse header
        ResponseHeader header;
        std::memcpy(&header, header_data.data(), sizeof(header));

        // Receive payload if any
        std::vector<uint8_t> payload;
        if (header.payload_length > 0)
        {
            payload = recvRaw(header.payload_length);

            // Validate checksum
            uint32_t checksum = Utils::calculateCRC32(payload);
            if (checksum != header.checksum)
            {
                throw std::runtime_error("Response checksum validation failed");
            }

            // Decrypt payload if encryption is enabled
            if (is_encrypted)
            {
                payload = Crypto::decrypt(payload, encryption_key);
            }
        }

        return std::make_pair(header.status, payload);
    }

    // Getters
    const std::string &getRemoteAddress() const { return remote_address; }
    int getRemotePort() const { return remote_port; }
    int getSocketFd() const { return socket_fd; }
    const std::string &getEncryptionKey() const { return encryption_key; }
    bool isEncrypted() const { return is_encrypted; }
};

#endif // NETWORK_H