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
#include <string>
#include <vector>

namespace cypherush {

class User {
public:
    User();
    User(std::string id,
         std::string username,
         std::string hashedPassword,
         std::vector<uint8_t> publicKey);
    ~User();

    const std::string& getId() const;
    const std::string& getUsername() const;
    const std::string& getHashedPassword() const;
    const std::vector<uint8_t>& getPublicKey() const;

    void setUsername(const std::string& username);
    void setHashedPassword(const std::string& hashedPassword);
    void setPublicKey(const std::vector<uint8_t>& publicKey);

private:
    std::string m_id;
    std::string m_username;
    std::string m_hashedPassword;
    std::vector<uint8_t> m_publicKey;
};

} // namespace cypherush
