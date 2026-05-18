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

class Message {
public:
    Message();
    Message(std::string id,
            std::string senderId,
            std::string recipientId,
            int64_t timestamp,
            std::vector<uint8_t> encryptedAesKey,
            std::vector<uint8_t> encryptedContent,
            std::vector<uint8_t> iv);
    ~Message();

    const std::string& getId() const;
    const std::string& getSenderId() const;
    const std::string& getRecipientId() const;
    int64_t getTimestamp() const;
    const std::vector<uint8_t>& getEncryptedAesKey() const;
    const std::vector<uint8_t>& getEncryptedContent() const;
    const std::vector<uint8_t>& getIv() const;

private:
    std::string m_id;
    std::string m_senderId;
    std::string m_recipientId;
    int64_t m_timestamp = 0;
    std::vector<uint8_t> m_encryptedAesKey;
    std::vector<uint8_t> m_encryptedContent;
    std::vector<uint8_t> m_iv;
};

} // namespace cypherush
