#include "network/NetworkMessage.h"

#include <utility>

namespace cypherush {

NetworkMessage::NetworkMessage(Type type, QByteArray payload)
    : m_type(type)
    , m_payload(std::move(payload)) {
}

NetworkMessage::Type NetworkMessage::getType() const {
    return m_type;
}

const QByteArray& NetworkMessage::getPayload() const {
    return m_payload;
}

QString NetworkMessage::typeToString(Type t) {
    switch (t) {
        case Type::Register:
            return QStringLiteral("Register");
        case Type::Login:
            return QStringLiteral("Login");
        case Type::AuthSuccess:
            return QStringLiteral("AuthSuccess");
        case Type::AuthFailure:
            return QStringLiteral("AuthFailure");
        case Type::GetPublicKey:
            return QStringLiteral("GetPublicKey");
        case Type::PublicKeyResponse:
            return QStringLiteral("PublicKeyResponse");
        case Type::SendMessage:
            return QStringLiteral("SendMessage");
        case Type::IncomingMessage:
            return QStringLiteral("IncomingMessage");
        case Type::Error:
            return QStringLiteral("Error");
        case Type::Ping:
            return QStringLiteral("Ping");
        case Type::Pong:
            return QStringLiteral("Pong");
    }
    return QStringLiteral("Unknown");
}

} // namespace cypherush
