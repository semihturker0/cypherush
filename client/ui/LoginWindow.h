#pragma once

#include <memory>
#include <string>

#include <QString>
#include <QWidget>

#include "ChatClient.h"
#include "services/AuthenticationService.h"

class QLineEdit;
class QPushButton;
class QLabel;

namespace cypherush {

class LoginWindow : public QWidget {
    Q_OBJECT

public:
    LoginWindow(std::shared_ptr<AuthenticationService> auth,
                std::shared_ptr<ChatClient> client,
                QWidget* parent = nullptr);
    ~LoginWindow() override;

    void reset();

signals:
    void loginSucceeded(const std::string& userId,
                        const std::string& username);

private slots:
    void onSubmit();
    void onServiceConnected();
    void onAuthSucceeded(const std::string& userId,
                         const std::string& username);
    void onRegistrationFailed(const std::string& reason);
    void onLoginFailed(const std::string& reason);

private:
    enum class Mode { Login, Register };

    void setMode(Mode m);
    void showStatus(const QString& text, bool isError);
    void performPendingAuth();

    std::shared_ptr<AuthenticationService> m_authService;
    std::shared_ptr<ChatClient> m_client;

    QLineEdit* m_usernameEdit = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
    QPushButton* m_loginTab = nullptr;
    QPushButton* m_registerTab = nullptr;
    QPushButton* m_submitButton = nullptr;
    QLabel* m_statusLabel = nullptr;

    QString m_serverAddress;
    Mode m_mode = Mode::Login;
    bool m_pendingAuth = false;
};

} // namespace cypherush
