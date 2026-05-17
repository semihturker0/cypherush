#pragma once

#include <memory>
#include <string>

#include <QObject>
#include <QString>

#include "ChatClient.h"

namespace cypherush {

class AuthenticationService : public QObject {
    Q_OBJECT

public:
    AuthenticationService(std::shared_ptr<ChatClient> client,
                          QString keyStorageDir = "keys",
                          QObject* parent = nullptr);
    ~AuthenticationService() override;

    void connectToServer();
    void disconnectFromServer();
    bool isConnected() const;
    bool isAuthenticated() const;

    void registerNewUser(const std::string& username,
                         const std::string& password);
    void loginExistingUser(const std::string& username,
                           const std::string& password);
    void logout();

signals:
    void connected();
    void disconnected();
    void authSucceeded(const std::string& userId,
                       const std::string& username);
    void registrationFailed(const std::string& reason);
    void loginFailed(const std::string& reason);

private slots:
    void onClientConnected();
    void onClientDisconnected();
    void onClientAuthSucceeded(const std::string& userId,
                               const std::string& username);
    void onClientAuthFailed(const std::string& reason);

private:
    enum class PendingOp { None, Register, Login };

    std::shared_ptr<ChatClient> m_client;
    QString m_keyStorageDir;
    PendingOp m_pendingOp = PendingOp::None;
    std::string m_pendingUsername;
    std::string m_pendingPassword;
};

} // namespace cypherush
