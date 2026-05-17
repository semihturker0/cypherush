#pragma once

#include <optional>
#include <string>
#include <vector>

#include <QJsonObject>

#include "data/EncryptedFileStorage.h"
#include "data/IRepository.h"
#include "models/User.h"

namespace cypherush {

class UserRepository : public IRepository<User> {
public:
    explicit UserRepository(EncryptedFileStorage storage);
    ~UserRepository() override;

    void save(const User& entity) override;
    std::optional<User> findById(const std::string& id) override;
    std::vector<User> findAll() override;
    void remove(const std::string& id) override;
    bool exists(const std::string& id) override;

    std::optional<User> findByUsername(const std::string& username);

private:
    void ensureLoaded();
    void persist();
    static QJsonObject userToJson(const User& user);
    static User userFromJson(const QJsonObject& obj);

    EncryptedFileStorage m_storage;
    std::vector<User> m_cache;
    bool m_loaded = false;
};

} // namespace cypherush
