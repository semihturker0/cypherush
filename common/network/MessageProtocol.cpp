// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#include "network/MessageProtocol.h"

#include <QDataStream>
#include <QIODevice>

#include "exceptions/AppExceptions.h"

namespace cypherush {

namespace {

bool isValidType(quint8 raw) {
    return raw <= static_cast<quint8>(NetworkMessage::Type::Pong);
}

} // namespace

QByteArray MessageProtocol::serialize(const NetworkMessage& msg) {
    const QByteArray& payload = msg.getPayload();

    QByteArray out;
    QDataStream stream(&out, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setVersion(QDataStream::Qt_6_0);

    const quint32 length = static_cast<quint32>(payload.size()) + 1;
    stream << length;
    stream << static_cast<quint8>(msg.getType());
    stream.writeRawData(payload.constData(),
                        static_cast<int>(payload.size()));

    return out;
}

std::optional<NetworkMessage> MessageProtocol::tryParse(QByteArray& buffer) {
    if (buffer.size() < 4) {
        return std::nullopt;  // header not yet received
    }

    QDataStream in(buffer);
    in.setByteOrder(QDataStream::BigEndian);
    in.setVersion(QDataStream::Qt_6_0);

    quint32 length = 0;
    in >> length;

    if (length < 1) {
        throw ProtocolException(
            "MessageProtocol: declared packet length is zero");
    }

    if (static_cast<quint64>(buffer.size()) <
        static_cast<quint64>(4) + length) {
        return std::nullopt;  // packet not fully arrived yet
    }

    const quint8 typeRaw =
        static_cast<quint8>(static_cast<unsigned char>(buffer.at(4)));
    if (!isValidType(typeRaw)) {
        throw ProtocolException(
            "MessageProtocol: unknown message type byte " +
            std::to_string(static_cast<int>(typeRaw)));
    }

    const QByteArray payload =
        buffer.mid(5, static_cast<int>(length) - 1);

    NetworkMessage msg(static_cast<NetworkMessage::Type>(typeRaw), payload);

    buffer.remove(0, static_cast<int>(4 + length));

    return msg;
}

} // namespace cypherush
