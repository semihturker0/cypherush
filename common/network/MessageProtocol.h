// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
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
