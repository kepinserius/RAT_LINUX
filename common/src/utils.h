#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <cstdint>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <random>
#include <functional>

namespace Utils
{
    // Generate random string of specified length
    inline std::string generateRandomString(size_t length)
    {
        static const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<int> distribution(0, sizeof(charset) - 2);

        std::string result;
        result.reserve(length);

        for (size_t i = 0; i < length; ++i)
        {
            result += charset[distribution(generator)];
        }

        return result;
    }

    // Get current timestamp as string
    inline std::string getTimestamp()
    {
        auto now = std::time(nullptr);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    // Simple CRC32 checksum calculation
    inline uint32_t calculateCRC32(const std::vector<uint8_t> &data)
    {
        uint32_t crc = 0xFFFFFFFF;
        for (auto &byte : data)
        {
            crc ^= byte;
            for (int i = 0; i < 8; i++)
            {
                crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
            }
        }
        return ~crc;
    }

    // Convert hexadecimal string to bytes
    inline std::vector<uint8_t> hexToBytes(const std::string &hex)
    {
        std::vector<uint8_t> bytes;
        for (size_t i = 0; i < hex.length(); i += 2)
        {
            std::string byteString = hex.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
            bytes.push_back(byte);
        }
        return bytes;
    }

    // Convert bytes to hexadecimal string
    inline std::string bytesToHex(const std::vector<uint8_t> &data)
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (auto &byte : data)
        {
            ss << std::setw(2) << static_cast<int>(byte);
        }
        return ss.str();
    }
}

#endif // UTILS_H