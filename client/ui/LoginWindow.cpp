#include "ui/LoginWindow.h"

#include <utility>

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

namespace cypherush {

namespace {

void repolish(QWidget* w) {
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
}

} // namespace

LoginWindow::LoginWindow(std::shared_ptr<AuthenticationService> auth,
                         std::shared_ptr<ChatClient> client,
                         QWidget* parent)
    : QWidget(parent)
    , m_authService(std::move(auth))
    , m_client(std::move(client)) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addStretch();

    auto* logo = new QLabel(QStringLiteral("CYPHERUSH"), this);
    logo->setAlignment(Qt::AlignCenter);
    logo->setStyleSheet(
        "font-size: 28px; font-weight: 700; color: #88E8C5; "
        "letter-spacing: 4px;");
    root->addWidget(logo);

    auto* tagline =
        new QLabel(QStringLiteral("Speak in cipher, hear in hush"), this);
    tagline->setAlignment(Qt::AlignCenter);
    tagline->setStyleSheet("font-size: 11px; color: #9BA8B8;");
    root->addWidget(tagline);

    root->addSpacing(40);

    auto* card = new QFrame(this);
    card->setObjectName("loginCard");
    card->setMaximumWidth(420);
    card->setMinimumWidth(420);

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 28, 28, 28);
    cardLayout->setSpacing(12);

    auto* tabRow = new QHBoxLayout();
    m_loginTab = new QPushButton(QStringLiteral("Giris Yap"), card);
    m_registerTab = new QPushButton(QStringLiteral("Kayit Ol"), card);
    for (QPushButton* tab : {m_loginTab, m_registerTab}) {
        tab->setCheckable(true);
        tab->setCursor(Qt::PointingHandCursor);
    }
    tabRow->addWidget(m_loginTab);
    tabRow->addWidget(m_registerTab);
    cardLayout->addLayout(tabRow);

    cardLayout->addSpacing(16);

    m_usernameEdit = new QLineEdit(card);
    m_usernameEdit->setPlaceholderText(QStringLiteral("Kullanici adi"));
    cardLayout->addWidget(m_usernameEdit);

    m_passwordEdit = new QLineEdit(card);
    m_passwordEdit->setPlaceholderText(QStringLiteral("Parola"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    cardLayout->addWidget(m_passwordEdit);

    auto* passwordNote = new QLabel(
        QStringLiteral(
            "Parolaniz PBKDF2 + salt ile guvenle saklanir"),
        card);
    passwordNote->setStyleSheet(
        "color: #A6ADC8; font-size: 10px; background: transparent;");
    cardLayout->addWidget(passwordNote);

    m_statusLabel = new QLabel(card);
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setVisible(false);
    cardLayout->addWidget(m_statusLabel);

    cardLayout->addSpacing(8);

    m_submitButton = new QPushButton(card);
    m_submitButton->setCursor(Qt::PointingHandCursor);
    m_submitButton->setDefault(true);
    m_submitButton->setAutoDefault(true);
    cardLayout->addWidget(m_submitButton);

    cardLayout->addSpacing(8);

    m_advancedToggle =
        new QPushButton(QStringLiteral("Gelismis ayarlar  v"), card);
    m_advancedToggle->setCursor(Qt::PointingHandCursor);
    m_advancedToggle->setFlat(true);
    m_advancedToggle->setStyleSheet(
        "QPushButton { background: transparent; border: none; "
        "color: #A6ADC8; font-size: 11px; padding: 2px; "
        "text-align: left; }"
        "QPushButton:hover { color: #CBA6F7; }");
    cardLayout->addWidget(m_advancedToggle);

    m_advancedSettingsWidget = new QWidget(card);
    auto* advLayout = new QVBoxLayout(m_advancedSettingsWidget);
    advLayout->setContentsMargins(0, 4, 0, 0);
    m_serverEdit = new QLineEdit(m_advancedSettingsWidget);
    m_serverEdit->setPlaceholderText(QStringLiteral("127.0.0.1"));
    m_serverEdit->setText(QStringLiteral("127.0.0.1"));
    advLayout->addWidget(m_serverEdit);
    m_advancedSettingsWidget->setVisible(false);
    cardLayout->addWidget(m_advancedSettingsWidget);

    connect(m_advancedToggle, &QPushButton::clicked, this, [this]() {
        const bool show = !m_advancedSettingsWidget->isVisible();
        m_advancedSettingsWidget->setVisible(show);
        m_advancedToggle->setText(
            show ? QStringLiteral("Gelismis ayarlar  ^")
                 : QStringLiteral("Gelismis ayarlar  v"));
    });

    auto* cardWrap = new QHBoxLayout();
    cardWrap->addStretch();
    cardWrap->addWidget(card);
    cardWrap->addStretch();
    root->addLayout(cardWrap);

    root->addStretch();

    connect(m_loginTab, &QPushButton::clicked, this,
            [this]() { setMode(Mode::Login); });
    connect(m_registerTab, &QPushButton::clicked, this,
            [this]() { setMode(Mode::Register); });
    connect(m_submitButton, &QPushButton::clicked, this,
            &LoginWindow::onSubmit);

    connect(m_usernameEdit, &QLineEdit::returnPressed, this,
            &LoginWindow::onSubmit);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this,
            &LoginWindow::onSubmit);
    connect(m_serverEdit, &QLineEdit::returnPressed, this,
            &LoginWindow::onSubmit);

    m_serverEdit->setFocusPolicy(Qt::StrongFocus);
    m_usernameEdit->setFocusPolicy(Qt::StrongFocus);
    m_passwordEdit->setFocusPolicy(Qt::StrongFocus);
    m_submitButton->setFocusPolicy(Qt::StrongFocus);

    setTabOrder(m_usernameEdit, m_passwordEdit);
    setTabOrder(m_passwordEdit, m_submitButton);

    connect(m_authService.get(), &AuthenticationService::connected, this,
            &LoginWindow::onServiceConnected);
    connect(m_authService.get(), &AuthenticationService::authSucceeded,
            this, &LoginWindow::onAuthSucceeded);
    connect(m_authService.get(),
            &AuthenticationService::registrationFailed, this,
            &LoginWindow::onRegistrationFailed);
    connect(m_authService.get(), &AuthenticationService::loginFailed,
            this, &LoginWindow::onLoginFailed);

    setMode(Mode::Login);
}

LoginWindow::~LoginWindow() = default;

void LoginWindow::reset() {
    m_passwordEdit->clear();
    showStatus(QString(), false);
    m_pendingAuth = false;
    setMode(Mode::Login);
}

void LoginWindow::setMode(Mode m) {
    m_mode = m;

    m_loginTab->setChecked(m == Mode::Login);
    m_registerTab->setChecked(m == Mode::Register);
    m_loginTab->setProperty("secondary", m != Mode::Login);
    m_registerTab->setProperty("secondary", m != Mode::Register);
    repolish(m_loginTab);
    repolish(m_registerTab);

    const QString action = (m == Mode::Register)
                               ? QStringLiteral("Kayit Ol")
                               : QStringLiteral("Giris Yap");
    m_submitButton->setText(action);
    m_submitButton->setEnabled(true);
}

void LoginWindow::showStatus(const QString& text, bool isError) {
    m_statusLabel->setText(text);
    m_statusLabel->setVisible(!text.isEmpty());
    m_statusLabel->setProperty("error", isError);
    repolish(m_statusLabel);
}

void LoginWindow::onSubmit() {
    const QString username = m_usernameEdit->text().trimmed();
    const QString password = m_passwordEdit->text();
    if (username.isEmpty() || password.isEmpty()) {
        showStatus(QStringLiteral("Kullanici adi ve parola bos olamaz"),
                   true);
        return;
    }

    m_submitButton->setEnabled(false);
    m_submitButton->setText(QStringLiteral("Isleniyor..."));
    showStatus(QStringLiteral("Sunucuya baglaniliyor..."), false);
    m_pendingAuth = true;

    if (!m_client->isConnected()) {
        m_client->setHost(m_serverEdit->text().trimmed().toStdString());
        m_client->setPort(55555);
        m_authService->connectToServer();
    } else {
        performPendingAuth();
    }
}

void LoginWindow::performPendingAuth() {
    if (!m_pendingAuth) {
        return;
    }
    m_pendingAuth = false;

    const std::string username =
        m_usernameEdit->text().trimmed().toStdString();
    const std::string password = m_passwordEdit->text().toStdString();

    showStatus(QStringLiteral("Kimlik dogrulaniyor..."), false);
    if (m_mode == Mode::Register) {
        m_authService->registerNewUser(username, password);
    } else {
        m_authService->loginExistingUser(username, password);
    }
}

void LoginWindow::onServiceConnected() {
    if (m_pendingAuth) {
        performPendingAuth();
    }
}

void LoginWindow::onAuthSucceeded(const std::string& userId,
                                  const std::string& username) {
    showStatus(QString(), false);
    emit loginSucceeded(userId, username);
}

void LoginWindow::onRegistrationFailed(const std::string& reason) {
    showStatus(QString::fromStdString(reason), true);
    setMode(m_mode);
}

void LoginWindow::onLoginFailed(const std::string& reason) {
    showStatus(QString::fromStdString(reason), true);
    setMode(m_mode);
}

} // namespace cypherush
