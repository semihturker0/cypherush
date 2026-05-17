#pragma once

#include <cstdint>
#include <vector>

namespace cypherush {

class IEncryptor {
public:
    virtual ~IEncryptor() = default;
    virtual std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data) = 0;
    virtual std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) = 0;
};

} // namespace cypherush
