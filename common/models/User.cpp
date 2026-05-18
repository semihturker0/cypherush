// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#include "models/User.h"

#include <utility>

namespace cypherush {

User::User() = default;

User::User(std::string id,
           std::string username,
           std::string hashedPassword,
           std::vector<uint8_t> publicKey)
    : m_id(std::move(id))
    , m_username(std::move(username))
    , m_hashedPassword(std::move(hashedPassword))
    , m_publicKey(std::move(publicKey)) {
}

User::~User() = default;

const std::string& User::getId() const {
    return m_id;
}

const std::string& User::getUsername() const {
    return m_username;
}

const std::string& User::getHashedPassword() const {
    return m_hashedPassword;
}

const std::vector<uint8_t>& User::getPublicKey() const {
    return m_publicKey;
}

void User::setUsername(const std::string& username) {
    m_username = username;
}

void User::setHashedPassword(const std::string& hashedPassword) {
    m_hashedPassword = hashedPassword;
}

void User::setPublicKey(const std::vector<uint8_t>& publicKey) {
    m_publicKey = publicKey;
}

} // namespace cypherush
