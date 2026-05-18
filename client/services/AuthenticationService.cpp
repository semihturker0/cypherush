// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#include "services/AuthenticationService.h"

#include <filesystem>
#include <utility>

#include "crypto/KeyManager.h"
#include "exceptions/AppExceptions.h"
#include "models/KeyPair.h"

namespace cypherush {

namespace {

std::string keyPath(const QString& dir, const std::string& username,
                    const char* suffix) {
    return dir.toStdString() + "/" + username + suffix;
}

} // namespace

AuthenticationService::AuthenticationService(
    std::shared_ptr<ChatClient> client, QString keyStorageDir,
    QObject* parent)
    : QObject(parent)
    , m_client(std::move(client))
    , m_keyStorageDir(std::move(keyStorageDir)) {
    connect(m_client.get(), &ChatClient::connected, this,
            &AuthenticationService::onClientConnected);
    connect(m_client.get(), &ChatClient::disconnected, this,
            &AuthenticationService::onClientDisconnected);
    connect(m_client.get(), &ChatClient::authSucceeded, this,
            &AuthenticationService::onClientAuthSucceeded);
    connect(m_client.get(), &ChatClient::authFailed, this,
            &AuthenticationService::onClientAuthFailed);

    std::error_code ec;
    std::filesystem::create_directories(m_keyStorageDir.toStdString(), ec);
}

AuthenticationService::~AuthenticationService() = default;

void AuthenticationService::connectToServer() {
    m_client->connectToServer();
}

void AuthenticationService::disconnectFromServer() {
    m_client->disconnectFromServer();
}

bool AuthenticationService::isConnected() const {
    return m_client->isConnected();
}

bool AuthenticationService::isAuthenticated() const {
    return m_client->isAuthenticated();
}

void AuthenticationService::registerNewUser(const std::string& username,
                                            const std::string& password) {
    m_pendingOp = PendingOp::Register;
    m_pendingUsername = username;
    m_pendingPassword = password;

    KeyPair kp = KeyManager::generateRsaKeyPair(2048);
    m_client->setKeyPair(kp);
    m_client->registerUser(username, password);
}

void AuthenticationService::loginExistingUser(const std::string& username,
                                              const std::string& password) {
    m_pendingOp = PendingOp::Login;
    m_pendingUsername = username;
    m_pendingPassword = password;

    const std::string publicPath =
        keyPath(m_keyStorageDir, username, "_public.key");
    const std::string privatePath =
        keyPath(m_keyStorageDir, username, "_private.key");

    if (!std::filesystem::exists(publicPath) ||
        !std::filesystem::exists(privatePath)) {
        m_pendingOp = PendingOp::None;
        emit loginFailed(
            "Bu kullanici icin yerel anahtar bulunamadi, once kayit "
            "olunmali");
        return;
    }

    std::vector<uint8_t> publicKey;
    std::vector<uint8_t> privateKey;
    try {
        publicKey = KeyManager::loadPublicKey(publicPath);
        privateKey = KeyManager::loadPrivateKey(privatePath, password);
    } catch (const InvalidCredentialsException&) {
        m_pendingOp = PendingOp::None;
        emit loginFailed("Yanlis parola");
        return;
    } catch (const FileStorageException&) {
        m_pendingOp = PendingOp::None;
        emit loginFailed("Anahtar dosyasi okunamadi");
        return;
    }

    KeyPair kp(publicKey, privateKey);
    m_client->setKeyPair(kp);
    m_client->loginUser(username, password);
}

void AuthenticationService::logout() {
    m_pendingOp = PendingOp::None;
    disconnectFromServer();
}

void AuthenticationService::onClientConnected() {
    emit connected();
}

void AuthenticationService::onClientDisconnected() {
    emit disconnected();
}

void AuthenticationService::onClientAuthSucceeded(
    const std::string& userId, const std::string& username) {
    if (m_pendingOp == PendingOp::Register) {
        KeyManager::savePublicKey(
            m_client->getKeyPair().getPublicKey(),
            keyPath(m_keyStorageDir, username, "_public.key"));
        KeyManager::savePrivateKey(
            m_client->getKeyPair().getPrivateKey(),
            keyPath(m_keyStorageDir, username, "_private.key"),
            m_pendingPassword);
    }
    m_pendingOp = PendingOp::None;
    emit authSucceeded(userId, username);
}

void AuthenticationService::onClientAuthFailed(const std::string& reason) {
    if (m_pendingOp == PendingOp::Register) {
        emit registrationFailed(reason);
    } else if (m_pendingOp == PendingOp::Login) {
        emit loginFailed(reason);
    }
    m_pendingOp = PendingOp::None;
}

} // namespace cypherush
