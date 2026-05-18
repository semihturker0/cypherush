// ============================================================
// Cypherush - End-to-end encrypted messaging
// Copyright (c) 2026 semihturker0. All Rights Reserved.
//
// This file is proprietary. Unauthorized copying, modification,
// or distribution is prohibited. See LICENSE in the project root
// for full terms.
// ============================================================
#pragma once

#include <stdexcept>
#include <string>

namespace cypherush {

class AppException : public std::runtime_error {
public:
    explicit AppException(const std::string& message)
        : std::runtime_error(message) {}
};

// --- Crypto ---------------------------------------------------------------

class CryptoException : public AppException {
public:
    explicit CryptoException(const std::string& message)
        : AppException(message) {}
};

class DecryptionException : public CryptoException {
public:
    explicit DecryptionException(const std::string& message)
        : CryptoException(message) {}
};

class EncryptionException : public CryptoException {
public:
    explicit EncryptionException(const std::string& message)
        : CryptoException(message) {}
};

class KeyNotFoundException : public CryptoException {
public:
    explicit KeyNotFoundException(const std::string& message)
        : CryptoException(message) {}
};

class InvalidKeyException : public CryptoException {
public:
    explicit InvalidKeyException(const std::string& message)
        : CryptoException(message) {}
};

// --- Network --------------------------------------------------------------

class NetworkException : public AppException {
public:
    explicit NetworkException(const std::string& message)
        : AppException(message) {}
};

class ConnectionException : public NetworkException {
public:
    explicit ConnectionException(const std::string& message)
        : NetworkException(message) {}
};

class ProtocolException : public NetworkException {
public:
    explicit ProtocolException(const std::string& message)
        : NetworkException(message) {}
};

// --- Authentication -------------------------------------------------------

class AuthenticationException : public AppException {
public:
    explicit AuthenticationException(const std::string& message)
        : AppException(message) {}
};

class InvalidCredentialsException : public AuthenticationException {
public:
    explicit InvalidCredentialsException(const std::string& message)
        : AuthenticationException(message) {}
};

class UserNotFoundException : public AuthenticationException {
public:
    explicit UserNotFoundException(const std::string& message)
        : AuthenticationException(message) {}
};

class UserAlreadyExistsException : public AuthenticationException {
public:
    explicit UserAlreadyExistsException(const std::string& message)
        : AuthenticationException(message) {}
};

// --- Repository -----------------------------------------------------------

class RepositoryException : public AppException {
public:
    explicit RepositoryException(const std::string& message)
        : AppException(message) {}
};

class FileStorageException : public RepositoryException {
public:
    explicit FileStorageException(const std::string& message)
        : RepositoryException(message) {}
};

class DataCorruptionException : public RepositoryException {
public:
    explicit DataCorruptionException(const std::string& message)
        : RepositoryException(message) {}
};

class EntityNotFoundException : public RepositoryException {
public:
    explicit EntityNotFoundException(const std::string& message)
        : RepositoryException(message) {}
};

} // namespace cypherush
