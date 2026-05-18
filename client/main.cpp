// Cypherush client.
//   (no flag) : GUI application (splash -> login -> chat)
//   --test    : headless services-layer E2E regression (needs a server
//               running at 127.0.0.1:55555)

#include <filesystem>
#include <functional>
#include <memory>
#include <string>

#include <QApplication>
#include <QDateTime>
#include <QEventLoop>
#include <QIcon>
#include <QMainWindow>
#include <QPropertyAnimation>
#include <QRegularExpression>
#include <QStackedWidget>
#include <QStringList>
#include <QTextStream>
#include <QTimer>

#include "ChatClient.h"
#include "services/AuthenticationService.h"
#include "services/MessagingService.h"
#include "services/ServerConfig.h"
#include "ui/LoginWindow.h"
#include "ui/MainChatWindow.h"
#include "ui/SplashScreen.h"

namespace {

const char* const kGlobalStyle = R"(
QMainWindow {
    background-color: #1E1E2E;
    color: #CDD6F4;
    font-family: "Segoe UI", "Helvetica Neue", sans-serif;
    font-size: 13px;
}
QMainWindow QWidget {
    background-color: #1E1E2E;
    color: #CDD6F4;
}
QPushButton {
    background-color: #94E2D5;
    color: #1E1E2E;
    border: none;
    border-radius: 6px;
    padding: 10px 18px;
    font-weight: 600;
}
QPushButton:hover { background-color: #A6E9DE; }
QPushButton:disabled { background-color: #45475A; color: #A6ADC8; }
QPushButton[secondary="true"] {
    background-color: transparent;
    color: #CDD6F4;
    border: 1px solid #45475A;
}
QPushButton[secondary="true"]:hover { border-color: #CBA6F7; color: #CBA6F7; }
QPushButton:checked { background-color: #94E2D5; color: #1E1E2E; }
QLineEdit {
    background-color: #313244;
    border: 1px solid #45475A;
    border-radius: 6px;
    padding: 9px 12px;
    color: #CDD6F4;
    font-size: 13px;
}
QLineEdit:focus { border-color: #94E2D5; }
QLabel#statusLabel { color: #A6ADC8; font-size: 11px; }
QLabel#statusLabel[error="true"] { color: #F38BA8; }
QFrame#loginCard {
    background-color: #313244;
    border-radius: 8px;
    border: 1px solid #45475A;
}
)";

using SlotFn = std::function<void()>;
using Connector = std::function<QMetaObject::Connection(SlotFn)>;

bool waitFor(const Connector& connector, int ms = 5000) {
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    bool got = false;
    QMetaObject::Connection c = connector([&]() {
        got = true;
        loop.quit();
    });
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(ms);
    loop.exec();

    QObject::disconnect(c);
    return got;
}

void cleanupTestArtifacts() {
    std::error_code ec;
    std::filesystem::remove_all("test_keys_alice", ec);
    std::filesystem::remove_all("test_keys_bob", ec);
    std::filesystem::remove("users.dat", ec);
    std::filesystem::remove("messages.dat", ec);
}

int runE2ETest() {
    QTextStream out(stdout);

    using cypherush::AuthenticationService;
    using cypherush::ChatClient;
    using cypherush::MessagingService;

    auto aliceClient = std::make_shared<ChatClient>("127.0.0.1", 55555);
    auto bobClient = std::make_shared<ChatClient>("127.0.0.1", 55555);

    AuthenticationService aliceAuth(aliceClient, "test_keys_alice");
    AuthenticationService bobAuth(bobClient, "test_keys_bob");
    MessagingService aliceMsg(aliceClient);
    MessagingService bobMsg(bobClient);

    const QString suffix =
        QString::number(QDateTime::currentMSecsSinceEpoch());
    const std::string aliceUsername = ("alice_" + suffix).toStdString();
    const std::string bobUsername = ("bob_" + suffix).toStdString();

    std::string failReason;
    QObject::connect(&aliceAuth,
                     &AuthenticationService::registrationFailed,
                     [&](const std::string& r) { failReason = r; });
    QObject::connect(&bobAuth,
                     &AuthenticationService::registrationFailed,
                     [&](const std::string& r) { failReason = r; });

    out << "[..] Connecting clients..." << Qt::endl;
    aliceClient->connectToServer();
    if (!waitFor([&](SlotFn s) {
            return QObject::connect(&aliceAuth,
                                    &AuthenticationService::connected,
                                    &aliceAuth, s);
        })) {
        out << "[FAIL] Alice could not connect" << Qt::endl;
        cleanupTestArtifacts();
        return 1;
    }
    bobClient->connectToServer();
    if (!waitFor([&](SlotFn s) {
            return QObject::connect(
                &bobAuth, &AuthenticationService::connected, &bobAuth, s);
        })) {
        out << "[FAIL] Bob could not connect" << Qt::endl;
        cleanupTestArtifacts();
        return 1;
    }
    out << "[OK] Both clients connected" << Qt::endl;

    aliceAuth.registerNewUser(aliceUsername, "AlicePass!");
    if (!waitFor([&](SlotFn s) {
            return QObject::connect(
                &aliceAuth, &AuthenticationService::authSucceeded,
                &aliceAuth, s);
        })) {
        out << "[FAIL] Alice registration failed (" << failReason.c_str()
            << ")" << Qt::endl;
        cleanupTestArtifacts();
        return 1;
    }
    bobAuth.registerNewUser(bobUsername, "BobPass!");
    if (!waitFor([&](SlotFn s) {
            return QObject::connect(
                &bobAuth, &AuthenticationService::authSucceeded, &bobAuth,
                s);
        })) {
        out << "[FAIL] Bob registration failed (" << failReason.c_str()
            << ")" << Qt::endl;
        cleanupTestArtifacts();
        return 1;
    }
    out << "[OK] Both registered" << Qt::endl;

    std::string receivedMessage;
    QObject::connect(&bobMsg, &MessagingService::messageReceived,
                     [&](const std::string&, const std::string& content,
                         qint64) { receivedMessage = content; });

    const std::string testMessage =
        "Merhaba Bob, servisler katmanindan!";
    aliceMsg.sendMessage(bobUsername, testMessage);
    if (!waitFor([&](SlotFn s) {
            return QObject::connect(
                &bobMsg, &MessagingService::messageReceived, &bobMsg, s);
        })) {
        out << "[FAIL] Bob did not receive the message" << Qt::endl;
        cleanupTestArtifacts();
        return 1;
    }
    if (receivedMessage != testMessage) {
        out << "[FAIL] Decrypted message mismatch" << Qt::endl;
        cleanupTestArtifacts();
        return 1;
    }
    out << "[OK] E2E encrypted message delivered and decrypted"
        << Qt::endl;
    out << "[OK] Services layer functional" << Qt::endl;

    aliceAuth.disconnectFromServer();
    bobAuth.disconnectFromServer();
    cleanupTestArtifacts();
    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("Cypherush"));

    const QIcon appIcon(QStringLiteral(":/icons/cypherush.svg"));
    app.setWindowIcon(appIcon);

    const QStringList args = app.arguments();
    if (args.size() >= 3 && args[1] == QStringLiteral("--setup")) {
        const QString ip = args[2].trimmed();
        const QRegularExpression ipPattern(
            QStringLiteral(R"(^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$)"));
        if (!ipPattern.match(ip).hasMatch()) {
            QTextStream(stderr) << "Invalid IP: " << ip << "\n";
            return 1;
        }
        if (cypherush::ServerConfig::saveServerAddress(ip)) {
            QTextStream(stdout)
                << "Config saved to: "
                << cypherush::ServerConfig::configFilePath() << "\n";
            return 0;
        }
        QTextStream(stderr) << "Failed to save config\n";
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--test") {
            return runE2ETest();
        }
    }

    app.setStyleSheet(QString::fromUtf8(kGlobalStyle));

    using cypherush::AuthenticationService;
    using cypherush::ChatClient;
    using cypherush::LoginWindow;
    using cypherush::MainChatWindow;
    using cypherush::MessagingService;
    using cypherush::SplashScreen;

    auto client = std::make_shared<ChatClient>("127.0.0.1", 55555);
    auto auth = std::make_shared<AuthenticationService>(client, "keys");
    auto msg = std::make_shared<MessagingService>(client);

    auto* mainWindow = new QMainWindow();
    mainWindow->setWindowTitle(QStringLiteral("Cypherush"));
    mainWindow->setWindowIcon(appIcon);
    mainWindow->setWindowOpacity(0.0);  // hidden until fade-in

    auto* stack = new QStackedWidget(mainWindow);
    mainWindow->setCentralWidget(stack);

    auto* login = new LoginWindow(auth, client);
    auto* chat = new MainChatWindow(msg, auth);
    stack->addWidget(login);
    stack->addWidget(chat);
    stack->setCurrentWidget(login);

    auto* splash = new SplashScreen();  // independent top-level window
    splash->setWindowIcon(appIcon);

    QObject::connect(splash, &SplashScreen::animationFinished,
                     [mainWindow, splash]() {
                         mainWindow->showMaximized();

                         auto* fadeIn = new QPropertyAnimation(
                             mainWindow, "windowOpacity");
                         fadeIn->setDuration(400);
                         fadeIn->setStartValue(0.0);
                         fadeIn->setEndValue(1.0);
                         fadeIn->setEasingCurve(QEasingCurve::OutQuad);
                         fadeIn->start(
                             QAbstractAnimation::DeleteWhenStopped);

                         splash->deleteLater();
                     });
    QObject::connect(
        login, &LoginWindow::loginSucceeded,
        [stack, chat](const std::string&, const std::string& uname) {
            chat->setUsername(uname);
            stack->setCurrentWidget(chat);
        });
    QObject::connect(chat, &MainChatWindow::logoutRequested,
                     [stack, login]() {
                         login->reset();
                         stack->setCurrentWidget(login);
                     });

    splash->show();
    splash->startAnimation();

    return app.exec();
}
