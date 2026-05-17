#pragma once

#include <QByteArray>
#include <QString>

namespace cypherush {

class NetworkMessage {
public:
    enum class Type : quint8 {
        Register = 0,
        Login = 1,
        AuthSuccess = 2,
        AuthFailure = 3,
        GetPublicKey = 4,
        PublicKeyResponse = 5,
        SendMessage = 6,
        IncomingMessage = 7,
        Error = 8,
        Ping = 9,
        Pong = 10,
    };

    NetworkMessage() = default;
    NetworkMessage(Type type, QByteArray payload);

    Type getType() const;
    const QByteArray& getPayload() const;

    static QString typeToString(Type t);

private:
    Type m_type = Type::Error;
    QByteArray m_payload;
};

} // namespace cypherush
