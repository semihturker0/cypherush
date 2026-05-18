// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#include "ui/MainChatWindow.h"

#include <utility>

#include <QDateTime>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QKeySequence>
#include <QScrollArea>
#include <QScrollBar>
#include <QShortcut>
#include <QSplitter>
#include <QTimer>
#include <QVBoxLayout>

namespace cypherush {

MainChatWindow::MainChatWindow(std::shared_ptr<MessagingService> msg,
                               std::shared_ptr<AuthenticationService> auth,
                               QWidget* parent)
    : QWidget(parent)
    , m_messagingService(std::move(msg))
    , m_authService(std::move(auth)) {
    resize(900, 600);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // --- Top bar ---------------------------------------------------------
    auto* topBar = new QWidget(this);
    topBar->setFixedHeight(50);
    topBar->setStyleSheet("background-color: #0A1525;");
    auto* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(16, 0, 16, 0);

    auto* logo = new QLabel(QStringLiteral("CYPHERUSH"), topBar);
    logo->setStyleSheet(
        "font-size: 16px; font-weight: 700; color: #88E8C5; "
        "letter-spacing: 3px; background: transparent;");
    topLayout->addWidget(logo);
    topLayout->addStretch();

    m_usernameLabel = new QLabel(topBar);
    m_usernameLabel->setStyleSheet(
        "color: #9BA8B8; background: transparent;");
    topLayout->addWidget(m_usernameLabel);

    m_logoutButton = new QPushButton(QStringLiteral("Cikis"), topBar);
    m_logoutButton->setProperty("secondary", true);
    m_logoutButton->setCursor(Qt::PointingHandCursor);
    m_logoutButton->setStyleSheet("padding: 5px 12px;");
    topLayout->addSpacing(12);
    topLayout->addWidget(m_logoutButton);

    root->addWidget(topBar);

    // --- Splitter: sidebar | chat ---------------------------------------
    auto* splitter = new QSplitter(Qt::Horizontal, this);

    auto* sidebar = new QWidget(splitter);
    sidebar->setMinimumWidth(220);
    sidebar->setMaximumWidth(320);
    auto* sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(12, 12, 12, 12);
    sideLayout->setSpacing(8);

    auto* contactsTitle = new QLabel(QStringLiteral("Kisiler"), sidebar);
    contactsTitle->setStyleSheet(
        "font-size: 12px; font-weight: 600; color: #9BA8B8;");
    sideLayout->addWidget(contactsTitle);

    auto* addRow = new QHBoxLayout();
    m_addContactInput = new QLineEdit(sidebar);
    m_addContactInput->setPlaceholderText(
        QStringLiteral("Kullanici adi ile kisi ekle"));
    m_addContactButton = new QPushButton(QStringLiteral("Ekle"), sidebar);
    m_addContactButton->setCursor(Qt::PointingHandCursor);
    addRow->addWidget(m_addContactInput);
    addRow->addWidget(m_addContactButton);
    sideLayout->addLayout(addRow);

    m_contactListWidget = new QListWidget(sidebar);
    m_contactListWidget->setStyleSheet(
        "QListWidget { background-color: #1A2640; border: 1px solid "
        "#2C3E5E; border-radius: 6px; }"
        "QListWidget::item { padding: 8px; }"
        "QListWidget::item:selected { background-color: #243152; "
        "border-left: 2px solid #88E8C5; color: #F0EEE6; }");
    sideLayout->addWidget(m_contactListWidget);

    auto* chatArea = new QWidget(splitter);
    auto* chatLayout = new QVBoxLayout(chatArea);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(0);

    m_chatHeaderLabel = new QLabel(QStringLiteral("Kisi secin..."),
                                   chatArea);
    m_chatHeaderLabel->setFixedHeight(45);
    m_chatHeaderLabel->setStyleSheet(
        "font-size: 14px; font-weight: 600; color: #F0EEE6; "
        "padding-left: 16px; background-color: #14203A; "
        "border-bottom: 1px solid #2C3E5E;");
    chatLayout->addWidget(m_chatHeaderLabel);

    m_chatScrollArea = new QScrollArea(chatArea);
    m_chatScrollArea->setWidgetResizable(true);
    m_chatScrollArea->setStyleSheet(
        "QScrollArea { border: none; background-color: #0E1A2B; }");
    m_chatMessagesContainer = new QWidget(m_chatScrollArea);
    m_chatMessagesLayout = new QVBoxLayout(m_chatMessagesContainer);
    m_chatMessagesLayout->setContentsMargins(16, 16, 16, 16);
    m_chatMessagesLayout->setSpacing(8);
    m_chatMessagesLayout->addStretch();
    m_chatScrollArea->setWidget(m_chatMessagesContainer);
    chatLayout->addWidget(m_chatScrollArea, 1);

    auto* inputBar = new QWidget(chatArea);
    inputBar->setStyleSheet("border-top: 1px solid #2C3E5E;");
    auto* inputLayout = new QHBoxLayout(inputBar);
    inputLayout->setContentsMargins(12, 10, 12, 10);
    m_messageInput = new QLineEdit(inputBar);
    m_messageInput->setPlaceholderText(QStringLiteral("Mesaj yaz..."));
    m_sendButton = new QPushButton(QStringLiteral("Gonder"), inputBar);
    m_sendButton->setCursor(Qt::PointingHandCursor);
    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_sendButton);
    chatLayout->addWidget(inputBar);

    splitter->addWidget(sidebar);
    splitter->addWidget(chatArea);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({220, 680});
    root->addWidget(splitter, 1);

    m_contactAddTimeoutTimer = new QTimer(this);
    m_contactAddTimeoutTimer->setSingleShot(true);

    // --- Wiring ----------------------------------------------------------
    connect(m_addContactButton, &QPushButton::clicked, this,
            &MainChatWindow::onAddContact);
    connect(m_addContactInput, &QLineEdit::returnPressed, this,
            &MainChatWindow::onAddContact);
    connect(m_sendButton, &QPushButton::clicked, this,
            &MainChatWindow::onSendMessage);
    connect(m_messageInput, &QLineEdit::returnPressed, this,
            &MainChatWindow::onSendMessage);
    connect(m_logoutButton, &QPushButton::clicked, this,
            &MainChatWindow::onLogoutClicked);
    connect(m_contactListWidget, &QListWidget::itemClicked, this,
            &MainChatWindow::onContactSelected);
    connect(m_contactAddTimeoutTimer, &QTimer::timeout, this,
            &MainChatWindow::onContactAddTimeout);

    auto* escShortcut =
        new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escShortcut, &QShortcut::activated, this,
            [this]() { m_messageInput->clear(); });

    connect(m_messagingService.get(),
            &MessagingService::contactKeyResolved, this,
            &MainChatWindow::onContactKeyResolved);
    connect(m_messagingService.get(),
            &MessagingService::messageReceived, this,
            &MainChatWindow::onIncomingMessage);
    connect(m_messagingService.get(),
            &MessagingService::messageSendFailed, this,
            &MainChatWindow::onSendFailed);
}

MainChatWindow::~MainChatWindow() = default;

void MainChatWindow::setUsername(const std::string& username) {
    m_currentUsername = username;
    m_usernameLabel->setText(
        QStringLiteral("Hos geldin, %1")
            .arg(QString::fromStdString(username)));
}

QListWidgetItem* MainChatWindow::contactItem(const QString& name) const {
    for (int i = 0; i < m_contactListWidget->count(); ++i) {
        QListWidgetItem* item = m_contactListWidget->item(i);
        if (item->text() == name) {
            return item;
        }
    }
    return nullptr;
}

void MainChatWindow::refreshHeader() {
    m_chatHeaderLabel->setText(m_activeContact.isEmpty()
                                   ? QStringLiteral("Kisi secin...")
                                   : m_activeContact);
}

void MainChatWindow::showHeaderNotice(const QString& text, int ms) {
    m_chatHeaderLabel->setText(text);
    QTimer::singleShot(ms, this, [this]() { refreshHeader(); });
}

void MainChatWindow::onAddContact() {
    const QString name = m_addContactInput->text().trimmed();
    if (name.isEmpty()) {
        showHeaderNotice(QStringLiteral("Kullanici adi bos olamaz"), 2000);
        return;
    }
    if (name.toStdString() == m_currentUsername) {
        showHeaderNotice(QStringLiteral("Kendinizi ekleyemezsiniz"), 2000);
        return;
    }
    if (contactItem(name) != nullptr) {
        showHeaderNotice(QStringLiteral("Bu kisi zaten ekli"), 2000);
        return;
    }

    m_pendingContactAdds.insert(name);
    m_addContactInput->setEnabled(false);
    m_addContactButton->setText(QStringLiteral("Ekleniyor..."));
    m_addContactButton->setEnabled(false);
    m_messagingService->requestContactKey(name.toStdString());
    m_contactAddTimeoutTimer->start(4000);
}

void MainChatWindow::onContactKeyResolved(const std::string& username,
                                          bool success) {
    const QString uname = QString::fromStdString(username);
    if (!m_pendingContactAdds.contains(uname)) {
        return;
    }
    m_pendingContactAdds.remove(uname);
    m_contactAddTimeoutTimer->stop();
    m_addContactInput->setEnabled(true);
    m_addContactButton->setText(QStringLiteral("Ekle"));
    m_addContactButton->setEnabled(true);

    if (success) {
        m_contactListWidget->addItem(uname);
        if (!m_chatHistory.contains(uname)) {
            m_chatHistory.insert(uname, {});
        }
        m_addContactInput->clear();
        if (m_activeContact.isEmpty()) {
            QListWidgetItem* item = contactItem(uname);
            m_contactListWidget->setCurrentItem(item);
            onContactSelected(item);
        }
    } else {
        showHeaderNotice(
            QStringLiteral("Kullanici '%1' bulunamadi").arg(uname), 2000);
    }
}

void MainChatWindow::onContactAddTimeout() {
    if (m_pendingContactAdds.isEmpty()) {
        return;
    }
    m_pendingContactAdds.clear();
    m_addContactInput->setEnabled(true);
    m_addContactButton->setText(QStringLiteral("Ekle"));
    m_addContactButton->setEnabled(true);
    showHeaderNotice(QStringLiteral("Zaman asimi"), 2000);
}

void MainChatWindow::onContactSelected(QListWidgetItem* item) {
    if (item == nullptr) {
        return;
    }
    m_activeContact = item->text();

    QFont f = item->font();
    f.setBold(false);
    item->setFont(f);

    refreshHeader();
    clearMessageView();
    for (const ChatMessageData& m : m_chatHistory.value(m_activeContact)) {
        renderMessage(m.content, m.timestamp, m.isOutgoing);
    }
    scrollToBottom();
}

void MainChatWindow::onSendMessage() {
    const QString text = m_messageInput->text().trimmed();
    if (text.isEmpty()) {
        return;
    }
    if (m_activeContact.isEmpty()) {
        showHeaderNotice(QStringLiteral("Once bir kisi secin"), 2000);
        return;
    }

    const qint64 ts = QDateTime::currentSecsSinceEpoch();
    m_messagingService->sendMessage(m_activeContact.toStdString(),
                                    text.toStdString());
    m_chatHistory[m_activeContact].append({text, ts, true});
    renderMessage(text, ts, true);
    m_messageInput->clear();
    scrollToBottom();
}

void MainChatWindow::onIncomingMessage(const std::string& sender,
                                       const std::string& content,
                                       qint64 timestamp) {
    const QString senderQ = QString::fromStdString(sender);
    const QString contentQ = QString::fromStdString(content);

    if (contactItem(senderQ) == nullptr) {
        m_contactListWidget->addItem(senderQ);
        m_chatHistory.insert(senderQ, {});
    }
    m_chatHistory[senderQ].append({contentQ, timestamp, false});

    if (m_activeContact == senderQ) {
        renderMessage(contentQ, timestamp, false);
        scrollToBottom();
    } else {
        QListWidgetItem* item = contactItem(senderQ);
        if (item != nullptr) {
            QFont f = item->font();
            f.setBold(true);
            item->setFont(f);
        }
    }
}

void MainChatWindow::onSendFailed(const std::string&,
                                  const std::string& reason) {
    showHeaderNotice(
        QStringLiteral("Mesaj hatasi: %1")
            .arg(QString::fromStdString(reason)),
        3000);
}

void MainChatWindow::onLogoutClicked() {
    m_authService->logout();
    emit logoutRequested();
}

void MainChatWindow::clearMessageView() {
    QLayoutItem* item = nullptr;
    while ((item = m_chatMessagesLayout->takeAt(0)) != nullptr) {
        if (item->widget() != nullptr) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    m_chatMessagesLayout->addStretch();
}

void MainChatWindow::renderMessage(const QString& content,
                                   qint64 timestamp, bool isOutgoing) {
    auto* wrap = new QWidget(m_chatMessagesContainer);
    auto* wrapLayout = new QHBoxLayout(wrap);
    wrapLayout->setContentsMargins(0, 0, 0, 0);

    auto* bubble = new QFrame(wrap);
    const int viewportW = m_chatScrollArea->viewport()->width();
    bubble->setMaximumWidth(qMax(220, viewportW / 2));
    bubble->setStyleSheet(
        QStringLiteral(
            "QFrame { background-color: %1; border-radius: 16px; }")
            .arg(isOutgoing ? "#88E8C5" : "#A89BE5"));
    auto* bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setContentsMargins(14, 10, 14, 10);
    bubbleLayout->setSpacing(2);

    auto* contentLabel = new QLabel(content, bubble);
    contentLabel->setWordWrap(true);
    contentLabel->setStyleSheet(
        "color: #0E1A2B; background: transparent; font-size: 13px;");
    bubbleLayout->addWidget(contentLabel);

    auto* timeLabel = new QLabel(
        QDateTime::fromSecsSinceEpoch(timestamp).toString("HH:mm"),
        bubble);
    timeLabel->setStyleSheet(
        "color: rgba(14, 26, 43, 0.6); background: transparent; "
        "font-size: 10px;");
    timeLabel->setAlignment(Qt::AlignRight);
    bubbleLayout->addWidget(timeLabel);

    if (isOutgoing) {
        wrapLayout->addStretch();
        wrapLayout->addWidget(bubble);
    } else {
        wrapLayout->addWidget(bubble);
        wrapLayout->addStretch();
    }

    m_chatMessagesLayout->insertWidget(m_chatMessagesLayout->count() - 1,
                                       wrap);
}

void MainChatWindow::scrollToBottom() {
    QTimer::singleShot(0, this, [this]() {
        QScrollBar* bar = m_chatScrollArea->verticalScrollBar();
        bar->setValue(bar->maximum());
    });
}

} // namespace cypherush
