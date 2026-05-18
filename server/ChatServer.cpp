// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#include "ChatServer.h"

#include <QDateTime>
#include <QDebug>
#include <QHostAddress>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QUuid>
#include <utility>

#include "crypto/PasswordHasher.h"
#include "exceptions/AppExceptions.h"
#include "models/Message.h"
#include "models/User.h"
#include "network/MessageProtocol.h"

namespace cypherush {

namespace {

QString bytesToBase64(const std::vector<uint8_t>& bytes) {
    const QByteArray ba(reinterpret_cast<const char*>(bytes.data()),
                        static_cast<int>(bytes.size()));
    return QString::fromLatin1(ba.toBase64());
}

std::vector<uint8_t> base64ToBytes(const QString& text) {
    const QByteArray decoded = QByteArray::fromBase64(text.toLatin1());
    return std::vector<uint8_t>(decoded.begin(), decoded.end());
}

QJsonObject parsePayload(const QByteArray& bytes) {
    return QJsonDocument::fromJson(bytes).object();
}

} // namespace

ChatServer::ChatServer(std::shared_ptr<UserRepository> userRepo,
                       std::shared_ptr<MessageRepository> messageRepo,
                       quint16 port, QObject* parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_userRepo(std::move(userRepo))
    , m_messageRepo(std::move(messageRepo))
    , m_port(port) {
    connect(m_server, &QTcpServer::newConnection, this,
            &ChatServer::onNewConnection);
}

ChatServer::~ChatServer() {
    stop();
}

bool ChatServer::start() {
    if (!m_server->listen(QHostAddress::Any, m_port)) {
        qWarning() << "ChatServer failed to listen on port" << m_port
                   << ":" << m_server->errorString();
        return false;
    }
    qInfo() << "ChatServer listening on port" << m_port;
    return true;
}

void ChatServer::stop() {
    m_server->close();
    for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
        QTcpSocket* socket = it.key();
        if (socket) {
            socket->disconnectFromHost();
            socket->deleteLater();
        }
    }
    m_sessions.clear();
}

void ChatServer::onNewConnection() {
    while (m_server->hasPendingConnections()) {
        QTcpSocket* socket = m_server->nextPendingConnection();

        ClientSession session;
        session.socket = socket;
        m_sessions[socket] = session;

        connect(socket, &QTcpSocket::readyRead, this,
                &ChatServer::onClientReadyRead);
        connect(socket, &QTcpSocket::disconnected, this,
                &ChatServer::onClientDisconnected);

        qInfo() << "New client connected:"
                << socket->peerAddress().toString();
    }
}

void ChatServer::onClientReadyRead() {
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket || !m_sessions.contains(socket)) {
        return;
    }

    ClientSession& session = m_sessions[socket];
    session.readBuffer += socket->readAll();

    try {
        while (true) {
            std::optional<NetworkMessage> msg =
                MessageProtocol::tryParse(session.readBuffer);
            if (!msg.has_value()) {
                break;
            }
            handleMessage(socket, *msg);
        }
    } catch (const AppException& e) {
        sendError(socket, "PROTOCOL_ERROR", e.what());
        socket->disconnectFromHost();
    }
}

void ChatServer::onClientDisconnected() {
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        return;
    }
    qInfo() << "Client disconnected:" << socket->peerAddress().toString();
    m_sessions.remove(socket);
    socket->deleteLater();
}

void ChatServer::handleMessage(QTcpSocket* socket,
                               const NetworkMessage& msg) {
    switch (msg.getType()) {
        case NetworkMessage::Type::Register:
            handleRegister(socket, parsePayload(msg.getPayload()));
            break;
        case NetworkMessage::Type::Login:
            handleLogin(socket, parsePayload(msg.getPayload()));
            break;
        case NetworkMessage::Type::GetPublicKey:
            handleGetPublicKey(socket, parsePayload(msg.getPayload()));
            break;
        case NetworkMessage::Type::SendMessage:
            handleSendMessage(socket, parsePayload(msg.getPayload()));
            break;
        case NetworkMessage::Type::Ping: {
            NetworkMessage pong(NetworkMessage::Type::Pong, QByteArray());
            socket->write(MessageProtocol::serialize(pong));
            break;
        }
        default:
            sendError(socket, "UNSUPPORTED",
                      "Unsupported message type");
            break;
    }
}

void ChatServer::handleRegister(QTcpSocket* socket,
                                const QJsonObject& payload) {
    const std::string username =
        payload["username"].toString().toStdString();
    const std::string password =
        payload["password"].toString().toStdString();

    if (m_userRepo->findByUsername(username).has_value()) {
        sendAuthFailure(socket, "Username already taken");
        return;
    }

    const std::string hashedPassword =
        PasswordHasher::hashPassword(password);
    const std::string id =
        QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    const std::vector<uint8_t> publicKey =
        base64ToBytes(payload["publicKey"].toString());

    User user(id, username, hashedPassword, publicKey);
    m_userRepo->save(user);

    m_sessions[socket].authenticatedUsername = username;
    sendAuthSuccess(socket, id, username);
}

void ChatServer::handleLogin(QTcpSocket* socket,
                             const QJsonObject& payload) {
    const std::string username =
        payload["username"].toString().toStdString();
    const std::string password =
        payload["password"].toString().toStdString();

    const std::optional<User> user = m_userRepo->findByUsername(username);
    if (!user.has_value()) {
        sendAuthFailure(socket, "User not found");
        return;
    }

    if (!PasswordHasher::verifyPassword(password,
                                        user->getHashedPassword())) {
        sendAuthFailure(socket, "Invalid credentials");
        return;
    }

    m_sessions[socket].authenticatedUsername = username;
    sendAuthSuccess(socket, user->getId(), username);
}

void ChatServer::handleGetPublicKey(QTcpSocket* socket,
                                    const QJsonObject& payload) {
    if (!m_sessions[socket].authenticatedUsername.has_value()) {
        sendAuthFailure(socket, "Not authenticated");
        return;
    }

    const std::string target =
        payload["username"].toString().toStdString();
    const std::optional<User> user = m_userRepo->findByUsername(target);
    if (!user.has_value()) {
        sendAuthFailure(socket, "User not found");
        return;
    }

    QJsonObject response;
    response["username"] = QString::fromStdString(target);
    response["publicKey"] = bytesToBase64(user->getPublicKey());
    sendMessage(socket, NetworkMessage::Type::PublicKeyResponse, response);
}

void ChatServer::handleSendMessage(QTcpSocket* socket,
                                   const QJsonObject& payload) {
    const std::optional<std::string>& auth =
        m_sessions[socket].authenticatedUsername;
    if (!auth.has_value()) {
        sendAuthFailure(socket, "Not authenticated");
        return;
    }

    const std::string senderUsername = auth.value();
    const std::string recipientUsername =
        payload["recipientUsername"].toString().toStdString();

    const std::optional<User> sender =
        m_userRepo->findByUsername(senderUsername);
    const std::optional<User> recipient =
        m_userRepo->findByUsername(recipientUsername);
    if (!recipient.has_value() || !sender.has_value()) {
        sendAuthFailure(socket, "User not found");
        return;
    }

    const std::vector<uint8_t> encryptedContent =
        base64ToBytes(payload["encryptedData"].toString());
    const int64_t timestamp = QDateTime::currentMSecsSinceEpoch();
    const std::string messageId =
        QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();

    Message message(messageId, sender->getId(), recipient->getId(),
                    timestamp, {}, encryptedContent, {});
    m_messageRepo->save(message);

    // === DEMO LOG: proof that the server only ever sees ciphertext ===
    {
        const QByteArray cipher(
            reinterpret_cast<const char*>(encryptedContent.data()),
            static_cast<int>(encryptedContent.size()));
        const QByteArray cipherPreview = cipher.left(32);
        QString hexDump;
        for (int i = 0; i < cipherPreview.size(); ++i) {
            hexDump += QString("%1 ")
                           .arg(static_cast<uint8_t>(cipherPreview[i]), 2,
                                16, QChar('0'))
                           .toUpper();
            if ((i + 1) % 16 == 0 && i < cipherPreview.size() - 1) {
                hexDump += "\n                       ";
            }
        }

        qInfo().noquote() << "";
        qInfo().noquote()
            << QString("[E2E DEMO] %1 -> %2 (%3 bytes ciphertext)")
                   .arg(QString::fromStdString(senderUsername))
                   .arg(QString::fromStdString(recipientUsername))
                   .arg(encryptedContent.size());
        qInfo().noquote()
            << "[E2E DEMO] Server SADECE bu byte'lari gorur:";
        qInfo().noquote() << "                       " + hexDump;
        qInfo().noquote() << "[E2E DEMO] Plaintext content: <YOK - "
                             "server'da decrypt edilemez>";
        qInfo().noquote() << "";
    }
    // === /DEMO LOG ===

    for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
        const ClientSession& peer = it.value();
        if (peer.authenticatedUsername.has_value() &&
            peer.authenticatedUsername.value() == recipientUsername) {
            QJsonObject incoming;
            incoming["senderUsername"] =
                QString::fromStdString(senderUsername);
            incoming["encryptedData"] = bytesToBase64(encryptedContent);
            incoming["timestamp"] = static_cast<qint64>(timestamp);
            sendMessage(it.key(), NetworkMessage::Type::IncomingMessage,
                        incoming);
        }
    }
}

void ChatServer::sendMessage(QTcpSocket* socket, NetworkMessage::Type type,
                             const QJsonObject& payload) {
    const QByteArray jsonBytes =
        QJsonDocument(payload).toJson(QJsonDocument::Compact);
    NetworkMessage nm(type, jsonBytes);
    socket->write(MessageProtocol::serialize(nm));
}

void ChatServer::sendAuthSuccess(QTcpSocket* socket,
                                 const std::string& userId,
                                 const std::string& username) {
    QJsonObject payload;
    payload["userId"] = QString::fromStdString(userId);
    payload["username"] = QString::fromStdString(username);
    sendMessage(socket, NetworkMessage::Type::AuthSuccess, payload);
}

void ChatServer::sendAuthFailure(QTcpSocket* socket,
                                 const std::string& reason) {
    QJsonObject payload;
    payload["reason"] = QString::fromStdString(reason);
    sendMessage(socket, NetworkMessage::Type::AuthFailure, payload);
}

void ChatServer::sendError(QTcpSocket* socket, const std::string& code,
                           const std::string& message) {
    QJsonObject payload;
    payload["code"] = QString::fromStdString(code);
    payload["message"] = QString::fromStdString(message);
    sendMessage(socket, NetworkMessage::Type::Error, payload);
}

} // namespace cypherush
