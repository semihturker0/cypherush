#pragma once

#include <optional>

#include <QByteArray>

#include "network/NetworkMessage.h"

namespace cypherush {

// Static utility for length-prefixed wire (de)serialization.
// Wire format: [4B big-endian uint32 length][1B type][N B payload]
// where length == 1 (type) + payload size.
class MessageProtocol {
public:
    MessageProtocol() = delete;

    static QByteArray serialize(const NetworkMessage& msg);
    static std::optional<NetworkMessage> tryParse(QByteArray& buffer);
};

} // namespace cypherush
