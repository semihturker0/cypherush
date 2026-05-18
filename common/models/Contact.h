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

class Contact {
public:
    Contact();
    Contact(std::string userId,
            std::string displayName,
            std::vector<uint8_t> publicKey);
    ~Contact();

    const std::string& getUserId() const;
    const std::string& getDisplayName() const;
    const std::vector<uint8_t>& getPublicKey() const;

    void setDisplayName(const std::string& displayName);

private:
    std::string m_userId;
    std::string m_displayName;
    std::vector<uint8_t> m_publicKey;
};

} // namespace cypherush
