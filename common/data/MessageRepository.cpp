#include "data/MessageRepository.h"

#include <algorithm>
#include <utility>

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>

#include "exceptions/AppExceptions.h"

namespace cypherush {

namespace {

QString bytesToBase64(const std::vector<uint8_t>& bytes) {
    const QByteArray ba(reinterpret_cast<const char*>(bytes.data()),
                        static_cast<int>(bytes.size()));
    return QString::fromLatin1(ba.toBase64());
}

std::vector<uint8_t> base64ToBytes(const QString& text) {
    const QByteArray decoded = QByteArray::fromBase64(text.toLatin1());
    return std::vector<uint8_t>(decoded.begin(), decoded.end());
}

} // namespace

MessageRepository::MessageRepository(EncryptedFileStorage storage)
    : m_storage(std::move(storage)) {
}

MessageRepository::~MessageRepository() = default;

QJsonObject MessageRepository::messageToJson(const Message& message) {
    QJsonObject obj;
    obj["id"] = QString::fromStdString(message.getId());
    obj["senderId"] = QString::fromStdString(message.getSenderId());
    obj["recipientId"] = QString::fromStdString(message.getRecipientId());
    obj["timestamp"] = static_cast<qint64>(message.getTimestamp());
    obj["encryptedAesKey"] = bytesToBase64(message.getEncryptedAesKey());
    obj["encryptedContent"] = bytesToBase64(message.getEncryptedContent());
    obj["iv"] = bytesToBase64(message.getIv());
    return obj;
}

Message MessageRepository::messageFromJson(const QJsonObject& obj) {
    return Message(
        obj["id"].toString().toStdString(),
        obj["senderId"].toString().toStdString(),
        obj["recipientId"].toString().toStdString(),
        static_cast<int64_t>(obj["timestamp"].toInteger()),
        base64ToBytes(obj["encryptedAesKey"].toString()),
        base64ToBytes(obj["encryptedContent"].toString()),
        base64ToBytes(obj["iv"].toString()));
}

void MessageRepository::ensureLoaded() {
    if (m_loaded) {
        return;
    }

    const std::vector<uint8_t> bytes = m_storage.readAll();
    m_cache.clear();

    if (!bytes.empty()) {
        const QByteArray ba(reinterpret_cast<const char*>(bytes.data()),
                            static_cast<int>(bytes.size()));
        const QJsonDocument doc = QJsonDocument::fromJson(ba);
        const QJsonArray messages = doc.object()["messages"].toArray();
        for (const QJsonValue& v : messages) {
            m_cache.push_back(messageFromJson(v.toObject()));
        }
    }

    m_loaded = true;
}

void MessageRepository::persist() {
    QJsonArray messages;
    for (const Message& message : m_cache) {
        messages.append(messageToJson(message));
    }
    QJsonObject root;
    root["messages"] = messages;

    const QByteArray ba =
        QJsonDocument(root).toJson(QJsonDocument::Compact);
    m_storage.writeAll(std::vector<uint8_t>(ba.begin(), ba.end()));
}

void MessageRepository::save(const Message& entity) {
    ensureLoaded();
    auto it = std::find_if(m_cache.begin(), m_cache.end(),
                           [&](const Message& m) {
                               return m.getId() == entity.getId();
                           });
    if (it != m_cache.end()) {
        *it = entity;
    } else {
        m_cache.push_back(entity);
    }
    persist();
}

std::optional<Message> MessageRepository::findById(const std::string& id) {
    ensureLoaded();
    auto it = std::find_if(m_cache.begin(), m_cache.end(),
                           [&](const Message& m) { return m.getId() == id; });
    if (it == m_cache.end()) {
        return std::nullopt;
    }
    return *it;
}

std::vector<Message> MessageRepository::findAll() {
    ensureLoaded();
    return m_cache;
}

void MessageRepository::remove(const std::string& id) {
    ensureLoaded();
    auto it = std::find_if(m_cache.begin(), m_cache.end(),
                           [&](const Message& m) { return m.getId() == id; });
    if (it == m_cache.end()) {
        throw EntityNotFoundException(
            "MessageRepository: no message with id " + id);
    }
    m_cache.erase(it);
    persist();
}

bool MessageRepository::exists(const std::string& id) {
    ensureLoaded();
    return std::any_of(m_cache.begin(), m_cache.end(),
                       [&](const Message& m) { return m.getId() == id; });
}

std::vector<Message> MessageRepository::findBySender(
    const std::string& senderId) {
    ensureLoaded();
    std::vector<Message> result;
    for (const Message& m : m_cache) {
        if (m.getSenderId() == senderId) {
            result.push_back(m);
        }
    }
    return result;
}

std::vector<Message> MessageRepository::findByRecipient(
    const std::string& recipientId) {
    ensureLoaded();
    std::vector<Message> result;
    for (const Message& m : m_cache) {
        if (m.getRecipientId() == recipientId) {
            result.push_back(m);
        }
    }
    return result;
}

std::vector<Message> MessageRepository::findConversation(
    const std::string& userA, const std::string& userB) {
    ensureLoaded();
    std::vector<Message> result;
    for (const Message& m : m_cache) {
        const bool aToB =
            m.getSenderId() == userA && m.getRecipientId() == userB;
        const bool bToA =
            m.getSenderId() == userB && m.getRecipientId() == userA;
        if (aToB || bToA) {
            result.push_back(m);
        }
    }
    std::sort(result.begin(), result.end(),
              [](const Message& lhs, const Message& rhs) {
                  return lhs.getTimestamp() < rhs.getTimestamp();
              });
    return result;
}

} // namespace cypherush
