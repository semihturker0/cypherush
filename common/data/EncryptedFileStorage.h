#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace cypherush {

// Low-level AES-256-GCM encrypted blob file I/O.
class EncryptedFileStorage {
public:
    EncryptedFileStorage(std::string filePath, std::vector<uint8_t> key);
    ~EncryptedFileStorage();

    void writeAll(const std::vector<uint8_t>& plaintext);
    std::vector<uint8_t> readAll();

    bool fileExists() const;
    void deleteFile();

private:
    std::string m_filePath;
    std::vector<uint8_t> m_key;
};

} // namespace cypherush
