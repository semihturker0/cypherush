#pragma once

#include <optional>
#include <string>
#include <vector>

namespace cypherush {

template <typename T>
class IRepository {
public:
    virtual ~IRepository() = default;

    virtual void save(const T& entity) = 0;
    virtual std::optional<T> findById(const std::string& id) = 0;
    virtual std::vector<T> findAll() = 0;
    virtual void remove(const std::string& id) = 0;
    virtual bool exists(const std::string& id) = 0;
};

} // namespace cypherush
