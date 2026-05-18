// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#include "crypto/HybridEncryptor.h"

#include <utility>

#include "crypto/AESEncryptor.h"
#include "exceptions/AppExceptions.h"

namespace cypherush {

HybridEncryptor::HybridEncryptor(RSAEncryptor rsa)
    : m_rsaEncryptor(std::move(rsa)) {
}

HybridEncryptor::~HybridEncryptor() = default;

std::vector<uint8_t> HybridEncryptor::encrypt(const std::vector<uint8_t>& data) {
    AESEncryptor aes;  // fresh random 32-byte data key
    const std::vector<uint8_t> aesOutput = aes.encrypt(data);
    const std::vector<uint8_t> aesKey = aes.getKey();
    const std::vector<uint8_t> rsaEncryptedKey = m_rsaEncryptor.encrypt(aesKey);

    const uint32_t keyLen = static_cast<uint32_t>(rsaEncryptedKey.size());

    std::vector<uint8_t> result;
    result.reserve(4 + rsaEncryptedKey.size() + aesOutput.size());
    result.push_back(static_cast<uint8_t>((keyLen >> 24) & 0xFF));
    result.push_back(static_cast<uint8_t>((keyLen >> 16) & 0xFF));
    result.push_back(static_cast<uint8_t>((keyLen >> 8) & 0xFF));
    result.push_back(static_cast<uint8_t>(keyLen & 0xFF));
    result.insert(result.end(), rsaEncryptedKey.begin(), rsaEncryptedKey.end());
    result.insert(result.end(), aesOutput.begin(), aesOutput.end());
    return result;
}

std::vector<uint8_t> HybridEncryptor::decrypt(const std::vector<uint8_t>& data) {
    if (data.size() < 4) {
        throw DecryptionException(
            "Hybrid decrypt: input shorter than length prefix");
    }

    const uint32_t rsaKeyLength =
        (static_cast<uint32_t>(data[0]) << 24) |
        (static_cast<uint32_t>(data[1]) << 16) |
        (static_cast<uint32_t>(data[2]) << 8) |
        static_cast<uint32_t>(data[3]);

    if (data.size() < static_cast<std::size_t>(4) + rsaKeyLength) {
        throw DecryptionException(
            "Hybrid decrypt: truncated RSA-wrapped key");
    }

    const std::vector<uint8_t> rsaEncryptedKey(
        data.begin() + 4, data.begin() + 4 + rsaKeyLength);
    const std::vector<uint8_t> aesOutput(
        data.begin() + 4 + rsaKeyLength, data.end());

    // RSA private-key absence surfaces as KeyNotFoundException here.
    const std::vector<uint8_t> aesKey =
        m_rsaEncryptor.decrypt(rsaEncryptedKey);

    try {
        AESEncryptor aes(aesKey);
        return aes.decrypt(aesOutput);
    } catch (const DecryptionException&) {
        throw;
    } catch (const InvalidKeyException& e) {
        throw DecryptionException(
            std::string("Hybrid decrypt: recovered AES key invalid: ") +
            e.what());
    }
}

} // namespace cypherush
