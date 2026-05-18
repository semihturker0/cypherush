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
