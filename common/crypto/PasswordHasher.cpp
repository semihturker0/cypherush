// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#include "crypto/PasswordHasher.h"

#include <cstdint>
#include <sstream>
#include <vector>

#include <cryptopp/base64.h>
#include <cryptopp/filters.h>
#include <cryptopp/misc.h>
#include <cryptopp/osrng.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/secblock.h>
#include <cryptopp/sha.h>

#include "exceptions/AppExceptions.h"

namespace cypherush {

namespace {

constexpr std::size_t SALT_SIZE = 16;
constexpr std::size_t HASH_SIZE = 32;
constexpr unsigned int ITERATIONS = 100000;
const char* const ALGO_TAG = "pbkdf2_sha256";

std::string base64Encode(const std::vector<uint8_t>& data) {
    std::string out;
    CryptoPP::StringSource(
        data.data(), data.size(), true,
        new CryptoPP::Base64Encoder(new CryptoPP::StringSink(out),
                                    /*insertLineBreaks=*/false));
    return out;
}

std::vector<uint8_t> base64Decode(const std::string& text) {
    std::string out;
    CryptoPP::StringSource(
        text, true,
        new CryptoPP::Base64Decoder(new CryptoPP::StringSink(out)));
    return std::vector<uint8_t>(out.begin(), out.end());
}

std::vector<uint8_t> pbkdf2(const std::string& password,
                            const std::vector<uint8_t>& salt,
                            unsigned int iterations,
                            std::size_t length) {
    CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> pbkdf;
    std::vector<uint8_t> derived(length);
    pbkdf.DeriveKey(
        derived.data(), derived.size(), 0x00,
        reinterpret_cast<const CryptoPP::byte*>(password.data()),
        password.size(),
        salt.data(), salt.size(),
        iterations, 0.0);
    return derived;
}

} // namespace

std::string PasswordHasher::hashPassword(const std::string& password) {
    CryptoPP::AutoSeededRandomPool prng;
    std::vector<uint8_t> salt(SALT_SIZE);
    prng.GenerateBlock(salt.data(), salt.size());

    const std::vector<uint8_t> hash =
        pbkdf2(password, salt, ITERATIONS, HASH_SIZE);

    std::ostringstream oss;
    oss << ALGO_TAG << '$' << ITERATIONS << '$'
        << base64Encode(salt) << '$' << base64Encode(hash);
    return oss.str();
}

bool PasswordHasher::verifyPassword(const std::string& password,
                                    const std::string& storedHash) {
    try {
        std::vector<std::string> parts;
        std::string token;
        std::istringstream iss(storedHash);
        while (std::getline(iss, token, '$')) {
            parts.push_back(token);
        }

        if (parts.size() != 4 || parts[0] != ALGO_TAG) {
            return false;
        }

        const unsigned int iterations =
            static_cast<unsigned int>(std::stoul(parts[1]));
        const std::vector<uint8_t> salt = base64Decode(parts[2]);
        const std::vector<uint8_t> expected = base64Decode(parts[3]);

        if (expected.empty()) {
            return false;
        }

        const std::vector<uint8_t> actual =
            pbkdf2(password, salt, iterations, expected.size());

        return CryptoPP::VerifyBufsEqual(actual.data(), expected.data(),
                                         expected.size());
    } catch (...) {
        return false;
    }
}

} // namespace cypherush
