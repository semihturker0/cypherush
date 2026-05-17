#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "models/KeyPair.h"

namespace cypherush {

// Static utility for RSA key generation and password-protected persistence.
class KeyManager {
public:
    KeyManager() = delete;

    static KeyPair generateRsaKeyPair(unsigned int bits = 2048);

    static void savePublicKey(const std::vector<uint8_t>& publicKeyDer,
                              const std::string& filePath);
    static std::vector<uint8_t> loadPublicKey(const std::string& filePath);

    static void savePrivateKey(const std::vector<uint8_t>& privateKeyDer,
                               const std::string& filePath,
                               const std::string& password);
    static std::vector<uint8_t> loadPrivateKey(const std::string& filePath,
                                                const std::string& password);
};

} // namespace cypherush
