// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#include "ChatClient.h"

#include <utility>

#include <QDebug>
#include <QHostAddress>
#include <QJsonDocument>

#include "crypto/HybridEncryptor.h"
#include "crypto/RSAEncryptor.h"
#include "exceptions/AppExceptions.h"
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

} // namespace

ChatClient::ChatClient(const std::string& host, quint16 port,
                       QObject* parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_host(host)
    , m_port(port) {
    connect(m_socket, &QTcpSocket::connected, this,
            &ChatClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this,
            &ChatClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this,
            &ChatClient::onReadyRead);
    connect(m_socket, &QAbstractSocket::errorOccurred, this,
            &ChatClient::onSocketError);
}

ChatClient::~ChatClient() = default;

void ChatClient::connectToServer() {
    m_socket->connectToHost(QString::fromStdString(m_host), m_port);
}

void ChatClient::disconnectFromServer() {
    m_socket->disconnectFromHost();
}

bool ChatClient::isConnected() const {
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

bool ChatClient::isAuthenticated() const {
    return m_currentUserId.has_value();
}

void ChatClient::setHost(const std::string& host) {
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        qWarning() << "ChatClient::setHost ignored: socket not unconnected";
        return;
    }
    m_host = host;
}

void ChatClient::setPort(quint16 port) {
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        qWarning() << "ChatClient::setPort ignored: socket not unconnected";
        return;
    }
    m_port = port;
}

void ChatClient::setKeyPair(KeyPair kp) {
    m_keyPair = std::move(kp);
}

const KeyPair& ChatClient::getKeyPair() const {
    return m_keyPair;
}

void ChatClient::registerUser(const std::string& username,
                              const std::string& password) {
    QJsonObject payload;
    payload["username"] = QString::fromStdString(username);
    payload["password"] = QString::fromStdString(password);
    payload["publicKey"] = bytesToBase64(m_keyPair.getPublicKey());
    sendMessage(NetworkMessage::Type::Register, payload);
}

void ChatClient::loginUser(const std::string& username,
                           const std::string& password) {
    QJsonObject payload;
    payload["username"] = QString::fromStdString(username);
    payload["password"] = QString::fromStdString(password);
    sendMessage(NetworkMessage::Type::Login, payload);
}

void ChatClient::requestPublicKey(const std::string& username) {
    QJsonObject payload;
    payload["username"] = QString::fromStdString(username);
    sendMessage(NetworkMessage::Type::GetPublicKey, payload);
}

void ChatClient::sendEncryptedMessage(
    const std::string& recipientUsername,
    const std::vector<uint8_t>& recipientPublicKey,
    const std::string& plaintext) {
    RSAEncryptor rsaForRecipient(recipientPublicKey);  // public only
    HybridEncryptor hybrid(std::move(rsaForRecipient));

    const std::vector<uint8_t> plainBytes(plaintext.begin(),
                                          plaintext.end());
    const std::vector<uint8_t> encryptedBytes = hybrid.encrypt(plainBytes);

    QJsonObject payload;
    payload["recipientUsername"] =
        QString::fromStdString(recipientUsername);
    payload["encryptedData"] = bytesToBase64(encryptedBytes);
    sendMessage(NetworkMessage::Type::SendMessage, payload);
}

void ChatClient::onConnected() {
    emit connected();
}

void ChatClient::onDisconnected() {
    m_currentUserId.reset();
    m_currentUsername.reset();
    emit disconnected();
}

void ChatClient::onReadyRead() {
    m_readBuffer += m_socket->readAll();
    try {
        while (true) {
            std::optional<NetworkMessage> msg =
                MessageProtocol::tryParse(m_readBuffer);
            if (!msg.has_value()) {
                break;
            }
            handleMessage(*msg);
        }
    } catch (const AppException& e) {
        emit errorOccurred("protocol_error", e.what());
    }
}

void ChatClient::onSocketError(QAbstractSocket::SocketError) {
    emit errorOccurred("socket_error",
                       m_socket->errorString().toStdString());
}

void ChatClient::handleMessage(const NetworkMessage& msg) {
    const QJsonObject payload =
        QJsonDocument::fromJson(msg.getPayload()).object();

    switch (msg.getType()) {
        case NetworkMessage::Type::AuthSuccess: {
            const std::string userId =
                payload["userId"].toString().toStdString();
            const std::string username =
                payload["username"].toString().toStdString();
            m_currentUserId = userId;
            m_currentUsername = username;
            emit authSucceeded(userId, username);
            break;
        }
        case NetworkMessage::Type::AuthFailure: {
            emit authFailed(payload["reason"].toString().toStdString());
            break;
        }
        case NetworkMessage::Type::PublicKeyResponse: {
            const std::string username =
                payload["username"].toString().toStdString();
            const std::vector<uint8_t> publicKey =
                base64ToBytes(payload["publicKey"].toString());
            emit publicKeyReceived(username, publicKey);
            break;
        }
        case NetworkMessage::Type::IncomingMessage: {
            const std::string senderUsername =
                payload["senderUsername"].toString().toStdString();
            const qint64 timestamp =
                payload["timestamp"].toInteger();
            const std::vector<uint8_t> encryptedData =
                base64ToBytes(payload["encryptedData"].toString());
            try {
                // Decryption only needs our private key; the public half is
                // derived from it inside RSAEncryptor::setPrivateKey.
                RSAEncryptor rsa;
                rsa.setPrivateKey(m_keyPair.getPrivateKey());
                HybridEncryptor hybrid(std::move(rsa));
                const std::vector<uint8_t> plain =
                    hybrid.decrypt(encryptedData);
                emit messageReceived(
                    senderUsername,
                    std::string(plain.begin(), plain.end()), timestamp);
            } catch (const AppException& e) {
                emit errorOccurred("decrypt_failed", e.what());
            }
            break;
        }
        case NetworkMessage::Type::Error: {
            emit errorOccurred(
                payload["code"].toString().toStdString(),
                payload["message"].toString().toStdString());
            break;
        }
        default:
            break;
    }
}

void ChatClient::sendMessage(NetworkMessage::Type type,
                             const QJsonObject& payload) {
    const QByteArray jsonBytes =
        QJsonDocument(payload).toJson(QJsonDocument::Compact);
    NetworkMessage nm(type, jsonBytes);
    m_socket->write(MessageProtocol::serialize(nm));
}

} // namespace cypherush
