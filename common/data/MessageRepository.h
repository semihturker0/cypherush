// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#pragma once

#include <optional>
#include <string>
#include <vector>

#include <QJsonObject>

#include "data/EncryptedFileStorage.h"
#include "data/IRepository.h"
#include "models/Message.h"

namespace cypherush {

class MessageRepository : public IRepository<Message> {
public:
    explicit MessageRepository(EncryptedFileStorage storage);
    ~MessageRepository() override;

    void save(const Message& entity) override;
    std::optional<Message> findById(const std::string& id) override;
    std::vector<Message> findAll() override;
    void remove(const std::string& id) override;
    bool exists(const std::string& id) override;

    std::vector<Message> findBySender(const std::string& senderId);
    std::vector<Message> findByRecipient(const std::string& recipientId);
    std::vector<Message> findConversation(const std::string& userA,
                                          const std::string& userB);

private:
    void ensureLoaded();
    void persist();
    static QJsonObject messageToJson(const Message& message);
    static Message messageFromJson(const QJsonObject& obj);

    EncryptedFileStorage m_storage;
    std::vector<Message> m_cache;
    bool m_loaded = false;
};

} // namespace cypherush
