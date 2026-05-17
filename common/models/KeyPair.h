#pragma once

#include <cstdint>
#include <vector>

namespace cypherush {

class KeyPair {
public:
    KeyPair();
    KeyPair(std::vector<uint8_t> publicKey, std::vector<uint8_t> privateKey);
    ~KeyPair();

    const std::vector<uint8_t>& getPublicKey() const;
    const std::vector<uint8_t>& getPrivateKey() const;
    bool hasPrivateKey() const;

private:
    std::vector<uint8_t> m_publicKey;
    std::vector<uint8_t> m_privateKey;
};

} // namespace cypherush
