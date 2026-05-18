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
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <QObject>

#include "ChatClient.h"

namespace cypherush {

class MessagingService : public QObject {
    Q_OBJECT

public:
    MessagingService(std::shared_ptr<ChatClient> client,
                     QObject* parent = nullptr);
    ~MessagingService() override;

    void sendMessage(const std::string& recipientUsername,
                      const std::string& plaintext);
    void requestContactKey(const std::string& username);

signals:
    void messageReceived(const std::string& senderUsername,
                         const std::string& content, qint64 timestamp);
    void messageSendFailed(const std::string& recipientUsername,
                           const std::string& reason);
    void contactKeyResolved(const std::string& username, bool success);

private slots:
    void onPublicKeyReceived(const std::string& username,
                             const std::vector<uint8_t>& publicKey);
    void onClientMessageReceived(const std::string& sender,
                                 const std::string& content,
                                 qint64 timestamp);
    void onClientErrorOccurred(const std::string& code,
                               const std::string& message);
    void onClientAuthFailed(const std::string& reason);

private:
    struct PendingSend {
        std::string recipient;
        std::string plaintext;
    };

    std::shared_ptr<ChatClient> m_client;
    std::unordered_map<std::string, std::vector<uint8_t>> m_publicKeyCache;
    std::vector<PendingSend> m_pendingSends;
    std::list<std::string> m_pendingContactKeyRequests;
};

} // namespace cypherush
