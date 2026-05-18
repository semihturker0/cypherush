// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#include "crypto/KeyManager.h"

#include <fstream>

#include <cryptopp/osrng.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/queue.h>
#include <cryptopp/rsa.h>
#include <cryptopp/sha.h>

#include "crypto/AESEncryptor.h"
#include "exceptions/AppExceptions.h"

namespace cypherush {

namespace {

constexpr std::size_t SALT_SIZE = 16;
constexpr std::size_t DERIVED_KEY_SIZE = 32;
constexpr unsigned int PBKDF2_ITERATIONS = 100000;

std::vector<uint8_t> saveKey(const CryptoPP::CryptoMaterial& key) {
    CryptoPP::ByteQueue queue;
    key.Save(queue);
    std::vector<uint8_t> der(static_cast<std::size_t>(queue.MaxRetrievable()));
    queue.Get(der.data(), der.size());
    return der;
}

std::vector<uint8_t> deriveKey(const std::string& password,
                               const std::vector<uint8_t>& salt) {
    CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> pbkdf;
    std::vector<uint8_t> derived(DERIVED_KEY_SIZE);
    pbkdf.DeriveKey(
        derived.data(), derived.size(), 0x00,
        reinterpret_cast<const CryptoPP::byte*>(password.data()),
        password.size(),
        salt.data(), salt.size(),
        PBKDF2_ITERATIONS, 0.0);
    return derived;
}

std::vector<uint8_t> readFile(const std::string& filePath) {
    std::ifstream in(filePath, std::ios::binary);
    if (!in) {
        throw FileStorageException("Cannot open file for reading: " +
                                   filePath);
    }
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(in),
                                std::istreambuf_iterator<char>());
}

void writeFile(const std::string& filePath,
                const std::vector<uint8_t>& data) {
    std::ofstream out(filePath, std::ios::binary | std::ios::trunc);
    if (!out) {
        throw FileStorageException("Cannot open file for writing: " +
                                   filePath);
    }
    out.write(reinterpret_cast<const char*>(data.data()),
              static_cast<std::streamsize>(data.size()));
    if (!out) {
        throw FileStorageException("Failed while writing file: " + filePath);
    }
}

} // namespace

KeyPair KeyManager::generateRsaKeyPair(unsigned int bits) {
    try {
        CryptoPP::AutoSeededRandomPool prng;

        CryptoPP::InvertibleRSAFunction params;
        params.GenerateRandomWithKeySize(prng, bits);

        CryptoPP::RSA::PrivateKey privateKey(params);
        CryptoPP::RSA::PublicKey publicKey(params);

        return KeyPair(saveKey(publicKey), saveKey(privateKey));
    } catch (const CryptoPP::Exception& e) {
        throw CryptoException(std::string("RSA key generation failed: ") +
                              e.what());
    }
}

void KeyManager::savePublicKey(const std::vector<uint8_t>& publicKeyDer,
                               const std::string& filePath) {
    writeFile(filePath, publicKeyDer);
}

std::vector<uint8_t> KeyManager::loadPublicKey(const std::string& filePath) {
    return readFile(filePath);
}

void KeyManager::savePrivateKey(const std::vector<uint8_t>& privateKeyDer,
                                const std::string& filePath,
                                const std::string& password) {
    try {
        CryptoPP::AutoSeededRandomPool prng;
        std::vector<uint8_t> salt(SALT_SIZE);
        prng.GenerateBlock(salt.data(), salt.size());

        const std::vector<uint8_t> derived = deriveKey(password, salt);
        AESEncryptor aes(derived);
        const std::vector<uint8_t> cipher = aes.encrypt(privateKeyDer);

        std::vector<uint8_t> blob;
        blob.reserve(salt.size() + cipher.size());
        blob.insert(blob.end(), salt.begin(), salt.end());
        blob.insert(blob.end(), cipher.begin(), cipher.end());

        writeFile(filePath, blob);
    } catch (const FileStorageException&) {
        throw;
    } catch (const AppException& e) {
        throw FileStorageException(
            std::string("Private key encryption failed: ") + e.what());
    }
}

std::vector<uint8_t> KeyManager::loadPrivateKey(const std::string& filePath,
                                                const std::string& password) {
    const std::vector<uint8_t> blob = readFile(filePath);
    if (blob.size() < SALT_SIZE) {
        throw FileStorageException("Private key file too short: " + filePath);
    }

    const std::vector<uint8_t> salt(blob.begin(), blob.begin() + SALT_SIZE);
    const std::vector<uint8_t> cipher(blob.begin() + SALT_SIZE, blob.end());

    const std::vector<uint8_t> derived = deriveKey(password, salt);

    try {
        AESEncryptor aes(derived);
        return aes.decrypt(cipher);
    } catch (const DecryptionException& e) {
        throw InvalidCredentialsException(
            std::string("Private key decryption failed (wrong password?): ") +
            e.what());
    }
}

} // namespace cypherush
