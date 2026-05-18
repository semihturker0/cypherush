// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#pragma once

#include <cstdint>
#include <vector>

#include "crypto/IEncryptor.h"

namespace cypherush {

// AES-256-GCM authenticated encryption.
// Wire format produced by encrypt(): [12-byte IV][ciphertext || 16-byte tag].
class AESEncryptor : public IEncryptor {
public:
    static constexpr std::size_t KEY_SIZE = 32;  // AES-256
    static constexpr std::size_t IV_SIZE = 12;   // GCM nonce
    static constexpr std::size_t TAG_SIZE = 16;  // GCM auth tag

    AESEncryptor();
    explicit AESEncryptor(const std::vector<uint8_t>& key);
    ~AESEncryptor() override;

    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) override;

    std::vector<uint8_t> getKey() const;

    static std::vector<uint8_t> generateRandomKey();

private:
    std::vector<uint8_t> m_key;
};

} // namespace cypherush
