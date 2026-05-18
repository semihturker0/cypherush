// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#include "crypto/AESEncryptor.h"

#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/gcm.h>
#include <cryptopp/osrng.h>

#include "exceptions/AppExceptions.h"

namespace cypherush {

AESEncryptor::AESEncryptor()
    : m_key(generateRandomKey()) {
}

AESEncryptor::AESEncryptor(const std::vector<uint8_t>& key)
    : m_key(key) {
    if (m_key.size() != KEY_SIZE) {
        throw InvalidKeyException(
            "AESEncryptor: key must be exactly 32 bytes (AES-256)");
    }
}

AESEncryptor::~AESEncryptor() = default;

std::vector<uint8_t> AESEncryptor::encrypt(const std::vector<uint8_t>& data) {
    using namespace CryptoPP;
    try {
        AutoSeededRandomPool prng;

        std::vector<uint8_t> iv(IV_SIZE);
        prng.GenerateBlock(iv.data(), iv.size());

        GCM<AES>::Encryption enc;
        enc.SetKeyWithIV(m_key.data(), m_key.size(), iv.data(), iv.size());

        std::string cipher;
        StringSource(
            data.data(), data.size(), true,
            new AuthenticatedEncryptionFilter(
                enc, new StringSink(cipher), false,
                static_cast<int>(TAG_SIZE)));

        std::vector<uint8_t> result;
        result.reserve(IV_SIZE + cipher.size());
        result.insert(result.end(), iv.begin(), iv.end());
        result.insert(result.end(), cipher.begin(), cipher.end());
        return result;
    } catch (const Exception& e) {
        throw EncryptionException(std::string("AES-GCM encrypt failed: ") +
                                  e.what());
    }
}

std::vector<uint8_t> AESEncryptor::decrypt(const std::vector<uint8_t>& data) {
    using namespace CryptoPP;
    if (data.size() < IV_SIZE + TAG_SIZE) {
        throw DecryptionException(
            "AES-GCM decrypt: input shorter than IV + tag");
    }
    try {
        GCM<AES>::Decryption dec;
        dec.SetKeyWithIV(m_key.data(), m_key.size(), data.data(), IV_SIZE);

        std::string recovered;
        AuthenticatedDecryptionFilter df(
            dec, new StringSink(recovered),
            AuthenticatedDecryptionFilter::DEFAULT_FLAGS,
            static_cast<int>(TAG_SIZE));
        df.Put(data.data() + IV_SIZE, data.size() - IV_SIZE);
        df.MessageEnd();

        if (!df.GetLastResult()) {
            throw DecryptionException(
                "AES-GCM decrypt: authentication tag verification failed");
        }
        return std::vector<uint8_t>(recovered.begin(), recovered.end());
    } catch (const HashVerificationFilter::HashVerificationFailed&) {
        throw DecryptionException(
            "AES-GCM decrypt: authentication tag verification failed");
    } catch (const Exception& e) {
        throw DecryptionException(std::string("AES-GCM decrypt failed: ") +
                                  e.what());
    }
}

std::vector<uint8_t> AESEncryptor::getKey() const {
    return m_key;
}

std::vector<uint8_t> AESEncryptor::generateRandomKey() {
    CryptoPP::AutoSeededRandomPool prng;
    std::vector<uint8_t> key(KEY_SIZE);
    prng.GenerateBlock(key.data(), key.size());
    return key;
}

} // namespace cypherush
