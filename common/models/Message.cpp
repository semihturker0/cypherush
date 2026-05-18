// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#include "models/Message.h"

#include <utility>

namespace cypherush {

Message::Message() = default;

Message::Message(std::string id,
                 std::string senderId,
                 std::string recipientId,
                 int64_t timestamp,
                 std::vector<uint8_t> encryptedAesKey,
                 std::vector<uint8_t> encryptedContent,
                 std::vector<uint8_t> iv)
    : m_id(std::move(id))
    , m_senderId(std::move(senderId))
    , m_recipientId(std::move(recipientId))
    , m_timestamp(timestamp)
    , m_encryptedAesKey(std::move(encryptedAesKey))
    , m_encryptedContent(std::move(encryptedContent))
    , m_iv(std::move(iv)) {
}

Message::~Message() = default;

const std::string& Message::getId() const {
    return m_id;
}

const std::string& Message::getSenderId() const {
    return m_senderId;
}

const std::string& Message::getRecipientId() const {
    return m_recipientId;
}

int64_t Message::getTimestamp() const {
    return m_timestamp;
}

const std::vector<uint8_t>& Message::getEncryptedAesKey() const {
    return m_encryptedAesKey;
}

const std::vector<uint8_t>& Message::getEncryptedContent() const {
    return m_encryptedContent;
}

const std::vector<uint8_t>& Message::getIv() const {
    return m_iv;
}

} // namespace cypherush
