#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include <vector>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>
#include <stdexcept>

class Crypto {
public:
    static std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data, const std::string& key) {
        // Generate random IV
        unsigned char iv[AES_BLOCK_SIZE];
        if (RAND_bytes(iv, AES_BLOCK_SIZE) != 1) {
            throw std::runtime_error("Failed to generate random IV");
        }
        
        // Set up encryption context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create encryption context");
        }
        
        // Initialize encryption operation
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, 
                              reinterpret_cast<const unsigned char*>(key.c_str()), iv) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize encryption");
        }
        
        // Allocate output buffer (data + padding + IV)
        std::vector<uint8_t> encrypted;
        encrypted.resize(data.size() + AES_BLOCK_SIZE + AES_BLOCK_SIZE);
        
        // Copy IV to first block of output
        std::memcpy(encrypted.data(), iv, AES_BLOCK_SIZE);
        
        int outlen1 = 0, outlen2 = 0;
        
        // Encrypt data
        if (EVP_EncryptUpdate(ctx, encrypted.data() + AES_BLOCK_SIZE, &outlen1, 
                             data.data(), static_cast<int>(data.size())) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed during encryption");
        }
        
        // Finalize encryption
        if (EVP_EncryptFinal_ex(ctx, encrypted.data() + AES_BLOCK_SIZE + outlen1, &outlen2) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to finalize encryption");
        }
        
        // Resize output to actual size
        encrypted.resize(AES_BLOCK_SIZE + outlen1 + outlen2);
        
        EVP_CIPHER_CTX_free(ctx);
        return encrypted;
    }
    
    static std::vector<uint8_t> decrypt(const std::vector<uint8_t>& encrypted, const std::string& key) {
        if (encrypted.size() <= AES_BLOCK_SIZE) {
            throw std::runtime_error("Encrypted data too short");
        }
        
        // Extract IV from first block
        unsigned char iv[AES_BLOCK_SIZE];
        std::memcpy(iv, encrypted.data(), AES_BLOCK_SIZE);
        
        // Set up decryption context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create decryption context");
        }
        
        // Initialize decryption operation
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                             reinterpret_cast<const unsigned char*>(key.c_str()), iv) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize decryption");
        }
        
        // Allocate output buffer
        std::vector<uint8_t> decrypted;
        decrypted.resize(encrypted.size());
        
        int outlen1 = 0, outlen2 = 0;
        
        // Decrypt data
        if (EVP_DecryptUpdate(ctx, decrypted.data(), &outlen1,
                             encrypted.data() + AES_BLOCK_SIZE, 
                             static_cast<int>(encrypted.size() - AES_BLOCK_SIZE)) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed during decryption");
        }
        
        // Finalize decryption
        if (EVP_DecryptFinal_ex(ctx, decrypted.data() + outlen1, &outlen2) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to finalize decryption");
        }
        
        // Resize output to actual size
        decrypted.resize(outlen1 + outlen2);
        
        EVP_CIPHER_CTX_free(ctx);
        return decrypted;
    }
    
    static std::string generateKey(int length = 32) {
        std::vector<unsigned char> key(length);
        if (RAND_bytes(key.data(), length) != 1) {
            throw std::runtime_error("Failed to generate random key");
        }
        
        std::string keyStr(reinterpret_cast<char*>(key.data()), length);
        return keyStr;
    }
};

#endif // CRYPTO_H 