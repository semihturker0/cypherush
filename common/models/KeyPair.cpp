#include "models/KeyPair.h"

#include <algorithm>
#include <utility>

namespace cypherush {

KeyPair::KeyPair() = default;

KeyPair::KeyPair(std::vector<uint8_t> publicKey, std::vector<uint8_t> privateKey)
    : m_publicKey(std::move(publicKey))
    , m_privateKey(std::move(privateKey)) {
}

KeyPair::~KeyPair() {
    // Memory hygiene: wipe private key material before release.
    std::fill(m_privateKey.begin(), m_privateKey.end(), 0);
}

const std::vector<uint8_t>& KeyPair::getPublicKey() const {
    return m_publicKey;
}

const std::vector<uint8_t>& KeyPair::getPrivateKey() const {
    return m_privateKey;
}

bool KeyPair::hasPrivateKey() const {
    return !m_privateKey.empty();
}

} // namespace cypherush
