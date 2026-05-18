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
#include <optional>
#include <string>
#include <vector>

#include <QAbstractSocket>
#include <QByteArray>
#include <QJsonObject>
#include <QObject>
#include <QTcpSocket>

#include "models/KeyPair.h"
#include "network/NetworkMessage.h"

namespace cypherush {

class ChatClient : public QObject {
    Q_OBJECT

public:
    ChatClient(const std::string& host, quint16 port,
               QObject* parent = nullptr);
    ~ChatClient() override;

    void connectToServer();
    void disconnectFromServer();
    bool isConnected() const;
    bool isAuthenticated() const;

    void setHost(const std::string& host);
    void setPort(quint16 port);

    void setKeyPair(KeyPair kp);
    const KeyPair& getKeyPair() const;

    void registerUser(const std::string& username,
                      const std::string& password);
    void loginUser(const std::string& username,
                   const std::string& password);
    void requestPublicKey(const std::string& username);
    void sendEncryptedMessage(const std::string& recipientUsername,
                              const std::vector<uint8_t>& recipientPublicKey,
                              const std::string& plaintext);

signals:
    void connected();
    void disconnected();
    void authSucceeded(const std::string& userId,
                       const std::string& username);
    void authFailed(const std::string& reason);
    void publicKeyReceived(const std::string& username,
                           const std::vector<uint8_t>& publicKey);
    void messageReceived(const std::string& senderUsername,
                         const std::string& decryptedContent,
                         qint64 timestamp);
    void errorOccurred(const std::string& code, const std::string& message);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);

private:
    void handleMessage(const NetworkMessage& msg);
    void sendMessage(NetworkMessage::Type type, const QJsonObject& payload);

    QTcpSocket* m_socket;
    std::string m_host;
    quint16 m_port;
    QByteArray m_readBuffer;
    KeyPair m_keyPair;
    std::optional<std::string> m_currentUserId;
    std::optional<std::string> m_currentUsername;
};

} // namespace cypherush
