#include "services/ServerConfig.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>

namespace cypherush {

namespace {

const QString kDefaultAddress = QStringLiteral("127.0.0.1");

bool isValidIpv4(const QString& ip) {
    static const QRegularExpression pattern(
        QStringLiteral(R"(^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$)"));
    return pattern.match(ip).hasMatch();
}

} // namespace

QString ServerConfig::configFilePath() {
    return QStandardPaths::writableLocation(
               QStandardPaths::AppDataLocation) +
           QStringLiteral("/config.dat");
}

QByteArray ServerConfig::obfuscate(const QByteArray& data) {
    const QByteArray key(OBFUSCATION_KEY);
    QByteArray out(data.size(), Qt::Uninitialized);
    for (int i = 0; i < data.size(); ++i) {
        out[i] = static_cast<char>(data[i] ^ key[i % key.size()]);
    }
    return out;
}

QByteArray ServerConfig::deobfuscate(const QByteArray& data) {
    return obfuscate(data);  // XOR is symmetric
}

QString ServerConfig::loadServerAddress() {
    const QString path = configFilePath();
    QFile file(path);
    if (!file.exists()) {
        return kDefaultAddress;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "ServerConfig: cannot open config for reading:"
                   << path;
        return kDefaultAddress;
    }

    const QByteArray plain = deobfuscate(file.readAll());
    file.close();

    const QString ip = QString::fromUtf8(plain).trimmed();
    if (!isValidIpv4(ip)) {
        qWarning() << "ServerConfig: config invalid/corrupt, using default";
        return kDefaultAddress;
    }
    return ip;
}

bool ServerConfig::saveServerAddress(const QString& ip) {
    const QString path = configFilePath();
    const QDir dir = QFileInfo(path).absoluteDir();
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        qCritical() << "ServerConfig: cannot create config directory:"
                    << dir.absolutePath();
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical() << "ServerConfig: cannot open config for writing:"
                    << path;
        return false;
    }

    const QByteArray blob = obfuscate(ip.trimmed().toUtf8());
    const qint64 written = file.write(blob);
    file.close();

    if (written != blob.size()) {
        qCritical() << "ServerConfig: incomplete write to" << path;
        return false;
    }
    return true;
}

} // namespace cypherush
