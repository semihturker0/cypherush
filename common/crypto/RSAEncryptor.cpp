// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#include "crypto/RSAEncryptor.h"

#include <cryptopp/filters.h>
#include <cryptopp/oaep.h>
#include <cryptopp/osrng.h>
#include <cryptopp/queue.h>
#include <cryptopp/sha.h>

#include "exceptions/AppExceptions.h"

namespace cypherush {

namespace {

std::vector<uint8_t> saveKey(const CryptoPP::CryptoMaterial& key) {
    CryptoPP::ByteQueue queue;
    key.Save(queue);
    std::vector<uint8_t> der(static_cast<std::size_t>(queue.MaxRetrievable()));
    queue.Get(der.data(), der.size());
    return der;
}

void loadKey(CryptoPP::CryptoMaterial& key, const std::vector<uint8_t>& der) {
    CryptoPP::ByteQueue queue;
    queue.Put(der.data(), der.size());
    queue.MessageEnd();
    key.Load(queue);
}

} // namespace

RSAEncryptor::RSAEncryptor() = default;

RSAEncryptor::RSAEncryptor(const std::vector<uint8_t>& publicKeyDer) {
    setPublicKey(publicKeyDer);
}

RSAEncryptor::RSAEncryptor(const std::vector<uint8_t>& publicKeyDer,
                           const std::vector<uint8_t>& privateKeyDer) {
    setPublicKey(publicKeyDer);
    setPrivateKey(privateKeyDer);
}

RSAEncryptor::~RSAEncryptor() = default;

std::vector<uint8_t> RSAEncryptor::encrypt(const std::vector<uint8_t>& data) {
    using namespace CryptoPP;
    try {
        RSAES<OAEP<SHA256>>::Encryptor encryptor(m_publicKey);

        if (data.size() > encryptor.FixedMaxPlaintextLength()) {
            throw EncryptionException(
                "RSA-OAEP encrypt: plaintext exceeds RSA block size");
        }

        AutoSeededRandomPool prng;
        std::string cipher;
        StringSource(
            data.data(), data.size(), true,
            new PK_EncryptorFilter(prng, encryptor, new StringSink(cipher)));

        return std::vector<uint8_t>(cipher.begin(), cipher.end());
    } catch (const EncryptionException&) {
        throw;
    } catch (const Exception& e) {
        throw EncryptionException(std::string("RSA-OAEP encrypt failed: ") +
                                  e.what());
    }
}

std::vector<uint8_t> RSAEncryptor::decrypt(const std::vector<uint8_t>& data) {
    using namespace CryptoPP;
    if (!m_hasPrivateKey) {
        throw KeyNotFoundException(
            "RSA-OAEP decrypt: no private key available");
    }
    try {
        RSAES<OAEP<SHA256>>::Decryptor decryptor(m_privateKey);

        AutoSeededRandomPool prng;
        std::string recovered;
        StringSource(
            data.data(), data.size(), true,
            new PK_DecryptorFilter(prng, decryptor,
                                   new StringSink(recovered)));

        return std::vector<uint8_t>(recovered.begin(), recovered.end());
    } catch (const Exception& e) {
        throw DecryptionException(std::string("RSA-OAEP decrypt failed: ") +
                                  e.what());
    }
}

bool RSAEncryptor::hasPrivateKey() const {
    return m_hasPrivateKey;
}

void RSAEncryptor::setPublicKey(const std::vector<uint8_t>& der) {
    try {
        loadKey(m_publicKey, der);
    } catch (const CryptoPP::Exception& e) {
        throw InvalidKeyException(std::string("RSA public key load failed: ") +
                                  e.what());
    }
}

void RSAEncryptor::setPrivateKey(const std::vector<uint8_t>& der) {
    try {
        loadKey(m_privateKey, der);
        m_publicKey = CryptoPP::RSA::PublicKey(m_privateKey);
        m_hasPrivateKey = true;
    } catch (const CryptoPP::Exception& e) {
        throw InvalidKeyException(std::string("RSA private key load failed: ") +
                                  e.what());
    }
}

std::vector<uint8_t> RSAEncryptor::exportPublicKey() const {
    return saveKey(m_publicKey);
}

std::vector<uint8_t> RSAEncryptor::exportPrivateKey() const {
    if (!m_hasPrivateKey) {
        throw KeyNotFoundException(
            "RSA exportPrivateKey: no private key available");
    }
    return saveKey(m_privateKey);
}

} // namespace cypherush
