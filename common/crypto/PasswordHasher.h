#pragma once

#include <string>

namespace cypherush {

// Static utility for PBKDF2-HMAC-SHA256 password hashing.
class PasswordHasher {
public:
    PasswordHasher() = delete;

    static std::string hashPassword(const std::string& password);
    static bool verifyPassword(const std::string& password,
                               const std::string& storedHash);
};

} // namespace cypherush
