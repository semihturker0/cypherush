// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#pragma once

#include <memory>
#include <optional>
#include <string>

#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include "data/MessageRepository.h"
#include "data/UserRepository.h"
#include "network/NetworkMessage.h"

namespace cypherush {

struct ClientSession {
    QTcpSocket* socket = nullptr;
    QByteArray readBuffer;
    std::optional<std::string> authenticatedUsername;
};

class ChatServer : public QObject {
    Q_OBJECT

public:
    ChatServer(std::shared_ptr<UserRepository> userRepo,
               std::shared_ptr<MessageRepository> messageRepo,
               quint16 port = 55555,
               QObject* parent = nullptr);
    ~ChatServer() override;

    bool start();
    void stop();

private slots:
    void onNewConnection();
    void onClientReadyRead();
    void onClientDisconnected();

private:
    void handleMessage(QTcpSocket* socket, const NetworkMessage& msg);

    void handleRegister(QTcpSocket* socket, const QJsonObject& payload);
    void handleLogin(QTcpSocket* socket, const QJsonObject& payload);
    void handleGetPublicKey(QTcpSocket* socket, const QJsonObject& payload);
    void handleSendMessage(QTcpSocket* socket, const QJsonObject& payload);

    void sendMessage(QTcpSocket* socket, NetworkMessage::Type type,
                     const QJsonObject& payload);
    void sendAuthSuccess(QTcpSocket* socket, const std::string& userId,
                         const std::string& username);
    void sendAuthFailure(QTcpSocket* socket, const std::string& reason);
    void sendError(QTcpSocket* socket, const std::string& code,
                   const std::string& message);

    QTcpServer* m_server = nullptr;
    QHash<QTcpSocket*, ClientSession> m_sessions;
    std::shared_ptr<UserRepository> m_userRepo;
    std::shared_ptr<MessageRepository> m_messageRepo;
    quint16 m_port;
};

} // namespace cypherush
