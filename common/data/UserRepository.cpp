// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#include "data/UserRepository.h"

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
    const QByteArray decoded =
        QByteArray::fromBase64(text.toLatin1());
    return std::vector<uint8_t>(decoded.begin(), decoded.end());
}

} // namespace

UserRepository::UserRepository(EncryptedFileStorage storage)
    : m_storage(std::move(storage)) {
}

UserRepository::~UserRepository() = default;

QJsonObject UserRepository::userToJson(const User& user) {
    QJsonObject obj;
    obj["id"] = QString::fromStdString(user.getId());
    obj["username"] = QString::fromStdString(user.getUsername());
    obj["hashedPassword"] =
        QString::fromStdString(user.getHashedPassword());
    obj["publicKey"] = bytesToBase64(user.getPublicKey());
    return obj;
}

User UserRepository::userFromJson(const QJsonObject& obj) {
    return User(obj["id"].toString().toStdString(),
                obj["username"].toString().toStdString(),
                obj["hashedPassword"].toString().toStdString(),
                base64ToBytes(obj["publicKey"].toString()));
}

void UserRepository::ensureLoaded() {
    if (m_loaded) {
        return;
    }

    const std::vector<uint8_t> bytes = m_storage.readAll();
    m_cache.clear();

    if (!bytes.empty()) {
        const QByteArray ba(reinterpret_cast<const char*>(bytes.data()),
                            static_cast<int>(bytes.size()));
        const QJsonDocument doc = QJsonDocument::fromJson(ba);
        const QJsonArray users = doc.object()["users"].toArray();
        for (const QJsonValue& v : users) {
            m_cache.push_back(userFromJson(v.toObject()));
        }
    }

    m_loaded = true;
}

void UserRepository::persist() {
    QJsonArray users;
    for (const User& user : m_cache) {
        users.append(userToJson(user));
    }
    QJsonObject root;
    root["users"] = users;

    const QByteArray ba =
        QJsonDocument(root).toJson(QJsonDocument::Compact);
    m_storage.writeAll(std::vector<uint8_t>(ba.begin(), ba.end()));
}

void UserRepository::save(const User& entity) {
    ensureLoaded();
    auto it = std::find_if(m_cache.begin(), m_cache.end(),
                           [&](const User& u) {
                               return u.getId() == entity.getId();
                           });
    if (it != m_cache.end()) {
        *it = entity;
    } else {
        m_cache.push_back(entity);
    }
    persist();
}

std::optional<User> UserRepository::findById(const std::string& id) {
    ensureLoaded();
    auto it = std::find_if(m_cache.begin(), m_cache.end(),
                           [&](const User& u) { return u.getId() == id; });
    if (it == m_cache.end()) {
        return std::nullopt;
    }
    return *it;
}

std::vector<User> UserRepository::findAll() {
    ensureLoaded();
    return m_cache;
}

void UserRepository::remove(const std::string& id) {
    ensureLoaded();
    auto it = std::find_if(m_cache.begin(), m_cache.end(),
                           [&](const User& u) { return u.getId() == id; });
    if (it == m_cache.end()) {
        throw EntityNotFoundException("UserRepository: no user with id " + id);
    }
    m_cache.erase(it);
    persist();
}

bool UserRepository::exists(const std::string& id) {
    ensureLoaded();
    return std::any_of(m_cache.begin(), m_cache.end(),
                       [&](const User& u) { return u.getId() == id; });
}

std::optional<User> UserRepository::findByUsername(
    const std::string& username) {
    ensureLoaded();
    auto it = std::find_if(m_cache.begin(), m_cache.end(),
                           [&](const User& u) {
                               return u.getUsername() == username;
                           });
    if (it == m_cache.end()) {
        return std::nullopt;
    }
    return *it;
}

} // namespace cypherush
