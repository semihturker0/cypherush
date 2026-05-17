#include "models/Contact.h"

#include <utility>

namespace cypherush {

Contact::Contact() = default;

Contact::Contact(std::string userId,
                 std::string displayName,
                 std::vector<uint8_t> publicKey)
    : m_userId(std::move(userId))
    , m_displayName(std::move(displayName))
    , m_publicKey(std::move(publicKey)) {
}

Contact::~Contact() = default;

const std::string& Contact::getUserId() const {
    return m_userId;
}

const std::string& Contact::getDisplayName() const {
    return m_displayName;
}

const std::vector<uint8_t>& Contact::getPublicKey() const {
    return m_publicKey;
}

void Contact::setDisplayName(const std::string& displayName) {
    m_displayName = displayName;
}

} // namespace cypherush
