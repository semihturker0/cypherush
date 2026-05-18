// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#include "data/EncryptedFileStorage.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <utility>

#include "crypto/AESEncryptor.h"
#include "exceptions/AppExceptions.h"

namespace cypherush {

EncryptedFileStorage::EncryptedFileStorage(std::string filePath,
                                           std::vector<uint8_t> key)
    : m_filePath(std::move(filePath))
    , m_key(std::move(key)) {
    if (m_key.size() != AESEncryptor::KEY_SIZE) {
        throw InvalidKeyException(
            "EncryptedFileStorage: key must be exactly 32 bytes (AES-256)");
    }
}

EncryptedFileStorage::~EncryptedFileStorage() = default;

void EncryptedFileStorage::writeAll(const std::vector<uint8_t>& plaintext) {
    AESEncryptor aes(m_key);
    const std::vector<uint8_t> cipher = aes.encrypt(plaintext);

    std::ofstream out(m_filePath, std::ios::binary | std::ios::trunc);
    if (!out) {
        throw FileStorageException(
            "EncryptedFileStorage: cannot open file for writing: " +
            m_filePath);
    }
    out.write(reinterpret_cast<const char*>(cipher.data()),
              static_cast<std::streamsize>(cipher.size()));
    if (!out) {
        throw FileStorageException(
            "EncryptedFileStorage: failed while writing file: " + m_filePath);
    }
}

std::vector<uint8_t> EncryptedFileStorage::readAll() {
    if (!fileExists()) {
        return {};  // first run — not an error
    }

    std::ifstream in(m_filePath, std::ios::binary);
    if (!in) {
        throw FileStorageException(
            "EncryptedFileStorage: cannot open file for reading: " +
            m_filePath);
    }
    std::vector<uint8_t> cipher{std::istreambuf_iterator<char>(in),
                                std::istreambuf_iterator<char>()};
    if (cipher.empty()) {
        return {};
    }

    try {
        AESEncryptor aes(m_key);
        return aes.decrypt(cipher);
    } catch (const DecryptionException& e) {
        throw DataCorruptionException(
            std::string("EncryptedFileStorage: corrupt or tampered file: ") +
            e.what());
    }
}

bool EncryptedFileStorage::fileExists() const {
    std::error_code ec;
    return std::filesystem::exists(m_filePath, ec);
}

void EncryptedFileStorage::deleteFile() {
    std::error_code ec;
    std::filesystem::remove(m_filePath, ec);
}

} // namespace cypherush
