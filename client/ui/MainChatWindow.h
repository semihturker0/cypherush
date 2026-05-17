#pragma once

#include <memory>
#include <string>

#include <QHash>
#include <QList>
#include <QSet>
#include <QString>
#include <QWidget>

#include "services/AuthenticationService.h"
#include "services/MessagingService.h"

class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QScrollArea;
class QTimer;
class QVBoxLayout;

namespace cypherush {

struct ChatMessageData {
    QString content;
    qint64 timestamp = 0;
    bool isOutgoing = false;
};

class MainChatWindow : public QWidget {
    Q_OBJECT

public:
    MainChatWindow(std::shared_ptr<MessagingService> msg,
                   std::shared_ptr<AuthenticationService> auth,
                   QWidget* parent = nullptr);
    ~MainChatWindow() override;

    void setUsername(const std::string& username);

signals:
    void logoutRequested();

private slots:
    void onAddContact();
    void onContactKeyResolved(const std::string& username, bool success);
    void onContactAddTimeout();
    void onContactSelected(QListWidgetItem* item);
    void onSendMessage();
    void onIncomingMessage(const std::string& sender,
                           const std::string& content, qint64 timestamp);
    void onSendFailed(const std::string& recipient,
                      const std::string& reason);
    void onLogoutClicked();

private:
    void renderMessage(const QString& content, qint64 timestamp,
                       bool isOutgoing);
    void clearMessageView();
    void scrollToBottom();
    void showHeaderNotice(const QString& text, int ms);
    void refreshHeader();
    QListWidgetItem* contactItem(const QString& name) const;

    std::shared_ptr<MessagingService> m_messagingService;
    std::shared_ptr<AuthenticationService> m_authService;
    std::string m_currentUsername;
    QString m_activeContact;
    QHash<QString, QList<ChatMessageData>> m_chatHistory;
    QSet<QString> m_pendingContactAdds;

    QLabel* m_usernameLabel = nullptr;
    QPushButton* m_logoutButton = nullptr;
    QListWidget* m_contactListWidget = nullptr;
    QLineEdit* m_addContactInput = nullptr;
    QPushButton* m_addContactButton = nullptr;
    QLabel* m_chatHeaderLabel = nullptr;
    QScrollArea* m_chatScrollArea = nullptr;
    QWidget* m_chatMessagesContainer = nullptr;
    QVBoxLayout* m_chatMessagesLayout = nullptr;
    QLineEdit* m_messageInput = nullptr;
    QPushButton* m_sendButton = nullptr;
    QTimer* m_contactAddTimeoutTimer = nullptr;
};

} // namespace cypherush
