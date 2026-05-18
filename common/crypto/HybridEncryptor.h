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
#include "crypto/RSAEncryptor.h"

namespace cypherush {

// Hybrid encryption: a fresh AES-256-GCM data key per message, itself
// wrapped with RSA-OAEP. Wire format produced by encrypt():
//   [4-byte big-endian uint32: rsaEncryptedKey length]
//   [rsaEncryptedKey]
//   [aesOutput == IV || ciphertext || tag]
class HybridEncryptor : public IEncryptor {
public:
    explicit HybridEncryptor(RSAEncryptor rsa);
    ~HybridEncryptor() override;

    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) override;

private:
    RSAEncryptor m_rsaEncryptor;
};

} // namespace cypherush
