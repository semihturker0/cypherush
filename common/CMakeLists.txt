add_library(cypherush_common STATIC
    models/User.cpp
    models/Message.cpp
    models/Contact.cpp
    models/KeyPair.cpp
    crypto/AESEncryptor.cpp
    crypto/RSAEncryptor.cpp
    crypto/HybridEncryptor.cpp
    crypto/KeyManager.cpp
    crypto/PasswordHasher.cpp
    data/EncryptedFileStorage.cpp
    data/UserRepository.cpp
    data/MessageRepository.cpp
    network/NetworkMessage.cpp
    network/MessageProtocol.cpp
)

target_include_directories(cypherush_common
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cypherush_common
    PUBLIC
        Qt6::Core
        Qt6::Network
        ${CRYPTOPP_LIB}
)
