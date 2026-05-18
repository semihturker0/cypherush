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

#include <cryptopp/rsa.h>

#include "crypto/IEncryptor.h"

namespace cypherush {

// RSA-2048 with OAEP-SHA256 padding.
class RSAEncryptor : public IEncryptor {
public:
    RSAEncryptor();
    explicit RSAEncryptor(const std::vector<uint8_t>& publicKeyDer);
    RSAEncryptor(const std::vector<uint8_t>& publicKeyDer,
                 const std::vector<uint8_t>& privateKeyDer);
    ~RSAEncryptor() override;

    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) override;

    bool hasPrivateKey() const;

    void setPublicKey(const std::vector<uint8_t>& der);
    void setPrivateKey(const std::vector<uint8_t>& der);

    std::vector<uint8_t> exportPublicKey() const;
    std::vector<uint8_t> exportPrivateKey() const;

private:
    CryptoPP::RSA::PublicKey m_publicKey;
    CryptoPP::RSA::PrivateKey m_privateKey;
    bool m_hasPrivateKey = false;
};

} // namespace cypherush
