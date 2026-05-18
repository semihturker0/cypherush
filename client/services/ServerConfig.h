// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#pragma once

#include <QByteArray>
#include <QString>

namespace cypherush {

// Stores the server address as an obfuscated blob under the user's
// app-data directory, hidden from the end user.
class ServerConfig {
public:
    static QString configFilePath();
    static QString loadServerAddress();
    static bool saveServerAddress(const QString& ip);

private:
    static QByteArray obfuscate(const QByteArray& data);
    static QByteArray deobfuscate(const QByteArray& data);

    static constexpr const char* OBFUSCATION_KEY =
        "CYPHERUSH_2025_OBFS_KEY_V1";
};

} // namespace cypherush
