// Cypherush server entry point.
// Run with "--test" to execute the in-process self-test suite instead.

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <QCoreApplication>
#include <QDebug>

#include "ChatServer.h"

#include <cryptopp/osrng.h>
#include <cryptopp/queue.h>
#include <cryptopp/rsa.h>

#include "crypto/AESEncryptor.h"
#include "crypto/HybridEncryptor.h"
#include "crypto/KeyManager.h"
#include "crypto/PasswordHasher.h"
#include "crypto/RSAEncryptor.h"
#include "data/EncryptedFileStorage.h"
#include "data/MessageRepository.h"
#include "data/UserRepository.h"
#include "exceptions/AppExceptions.h"
#include "models/KeyPair.h"
#include "models/Message.h"
#include "models/User.h"
#include "network/MessageProtocol.h"
#include "network/NetworkMessage.h"

namespace {

std::vector<uint8_t> saveKey(const CryptoPP::CryptoMaterial& key) {
    CryptoPP::ByteQueue queue;
    key.Save(queue);
    std::vector<uint8_t> der(static_cast<std::size_t>(queue.MaxRetrievable()));
    queue.Get(der.data(), der.size());
    return der;
}

std::vector<uint8_t> toBytes(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

bool testAes() {
    const std::vector<uint8_t> plaintext = toBytes("Merhaba, Cypherush!");

    cypherush::AESEncryptor aes;
    const std::vector<uint8_t> cipher = aes.encrypt(plaintext);
    const std::vector<uint8_t> recovered = aes.decrypt(cipher);

    return recovered == plaintext;
}

bool testRsa() {
    CryptoPP::AutoSeededRandomPool prng;

    CryptoPP::RSA::PrivateKey privateKey;
    privateKey.GenerateRandomWithKeySize(prng, 2048);
    CryptoPP::RSA::PublicKey publicKey(privateKey);

    cypherush::RSAEncryptor rsa(saveKey(publicKey), saveKey(privateKey));

    std::vector<uint8_t> data(32);
    prng.GenerateBlock(data.data(), data.size());

    const std::vector<uint8_t> cipher = rsa.encrypt(data);
    const std::vector<uint8_t> recovered = rsa.decrypt(cipher);

    return recovered == data;
}

bool testHybridE2E() {
    const cypherush::KeyPair aliceKeys =
        cypherush::KeyManager::generateRsaKeyPair();

    // Bob can only encrypt to Alice (public key only).
    cypherush::HybridEncryptor bob(
        cypherush::RSAEncryptor(aliceKeys.getPublicKey()));
    // Alice holds her private key and can decrypt.
    cypherush::HybridEncryptor alice(cypherush::RSAEncryptor(
        aliceKeys.getPublicKey(), aliceKeys.getPrivateKey()));

    const std::vector<uint8_t> message =
        toBytes("Sifreli olmasi gereken gizli mesaj");

    const std::vector<uint8_t> cipher = bob.encrypt(message);
    const std::vector<uint8_t> recovered = alice.decrypt(cipher);

    return recovered == message;
}

bool testKeyManagerPersistence() {
    const cypherush::KeyPair keys =
        cypherush::KeyManager::generateRsaKeyPair();

    const std::string pubPath = "cypherush_test_pub.der";
    const std::string privPath = "cypherush_test_priv.bin";
    const std::string password = "mySecretPass123";

    bool ok = true;

    cypherush::KeyManager::savePublicKey(keys.getPublicKey(), pubPath);
    const std::vector<uint8_t> loadedPub =
        cypherush::KeyManager::loadPublicKey(pubPath);
    ok = ok && (loadedPub == keys.getPublicKey());

    cypherush::KeyManager::savePrivateKey(keys.getPrivateKey(), privPath,
                                          password);
    const std::vector<uint8_t> loadedPriv =
        cypherush::KeyManager::loadPrivateKey(privPath, password);
    ok = ok && (loadedPriv == keys.getPrivateKey());

    bool wrongPasswordRejected = false;
    try {
        cypherush::KeyManager::loadPrivateKey(privPath, "yanlisparola");
    } catch (const cypherush::InvalidCredentialsException&) {
        wrongPasswordRejected = true;
    }
    ok = ok && wrongPasswordRejected;

    std::remove(pubPath.c_str());
    std::remove(privPath.c_str());

    return ok;
}

bool testPasswordHasher() {
    const std::string hash =
        cypherush::PasswordHasher::hashPassword("MyPa55!");

    const bool correct =
        cypherush::PasswordHasher::verifyPassword("MyPa55!", hash);
    const bool wrong =
        cypherush::PasswordHasher::verifyPassword("wrong", hash);

    return correct && !wrong;
}

bool testRepository() {
    const std::string usersPath = "test_users.dat";
    const std::string messagesPath = "test_messages.dat";

    std::remove(usersPath.c_str());
    std::remove(messagesPath.c_str());

    CryptoPP::AutoSeededRandomPool prng;
    std::vector<uint8_t> key(cypherush::AESEncryptor::KEY_SIZE);
    prng.GenerateBlock(key.data(), key.size());

    const std::vector<uint8_t> dummy{0x01, 0x02, 0x03};

    bool ok = true;

    // --- Users -----------------------------------------------------------
    {
        cypherush::UserRepository users(
            cypherush::EncryptedFileStorage(usersPath, key));

        users.save(cypherush::User("u1", "alice", "h1", dummy));
        users.save(cypherush::User("u2", "bob", "h2", dummy));
        users.save(cypherush::User("u3", "carol", "h3", dummy));

        ok = ok && (users.findAll().size() == 3);

        auto u2 = users.findById("u2");
        ok = ok && u2.has_value() && u2->getUsername() == "bob";

        auto byName = users.findByUsername("carol");
        ok = ok && byName.has_value() && byName->getId() == "u3";

        users.save(cypherush::User("u2", "bob_renamed", "h2", dummy));
        auto u2b = users.findById("u2");
        ok = ok && u2b.has_value() &&
             u2b->getUsername() == "bob_renamed";

        users.remove("u3");
        ok = ok && (users.findAll().size() == 2);
        ok = ok && users.exists("u1") && !users.exists("u3");
    }

    // --- Messages --------------------------------------------------------
    {
        cypherush::MessageRepository messages(
            cypherush::EncryptedFileStorage(messagesPath, key));

        messages.save(
            cypherush::Message("m1", "u1", "u2", 100, dummy, dummy, dummy));
        messages.save(
            cypherush::Message("m2", "u2", "u1", 200, dummy, dummy, dummy));
        messages.save(
            cypherush::Message("m3", "u1", "u3", 150, dummy, dummy, dummy));
        messages.save(
            cypherush::Message("m4", "u3", "u1", 300, dummy, dummy, dummy));
        messages.save(
            cypherush::Message("m5", "u1", "u2", 50, dummy, dummy, dummy));

        ok = ok && (messages.findAll().size() == 5);
        ok = ok && (messages.findBySender("u1").size() == 3);
        ok = ok && (messages.findByRecipient("u1").size() == 2);

        auto conv = messages.findConversation("u1", "u2");
        ok = ok && (conv.size() == 3);
        // Sorted ascending by timestamp: m5(50), m1(100), m2(200).
        ok = ok && conv[0].getId() == "m5" &&
             conv[1].getId() == "m1" && conv[2].getId() == "m2";

        // Direction symmetry.
        ok = ok &&
             (messages.findConversation("u2", "u1").size() == 3);
    }

    // --- Persistence: fresh instances read from disk ---------------------
    {
        cypherush::UserRepository users(
            cypherush::EncryptedFileStorage(usersPath, key));
        ok = ok && (users.findAll().size() == 2);
        auto u2 = users.findById("u2");
        ok = ok && u2.has_value() &&
             u2->getUsername() == "bob_renamed";

        cypherush::MessageRepository messages(
            cypherush::EncryptedFileStorage(messagesPath, key));
        ok = ok && (messages.findAll().size() == 5);
    }

    std::remove(usersPath.c_str());
    std::remove(messagesPath.c_str());

    return ok;
}

bool testProtocolRoundtrip() {
    using cypherush::MessageProtocol;
    using cypherush::NetworkMessage;

    NetworkMessage msg(NetworkMessage::Type::Register,
                       QByteArray("test_payload_data"));
    QByteArray buffer = MessageProtocol::serialize(msg);

    auto parsed = MessageProtocol::tryParse(buffer);
    return parsed.has_value() &&
           parsed->getType() == NetworkMessage::Type::Register &&
           parsed->getPayload() == QByteArray("test_payload_data") &&
           buffer.isEmpty();
}

bool testProtocolPartialBuffer() {
    using cypherush::MessageProtocol;

    QByteArray buffer("\x01\x02", 2);
    auto parsed = MessageProtocol::tryParse(buffer);
    return !parsed.has_value() && buffer.size() == 2;
}

bool testProtocolMultiMessage() {
    using cypherush::MessageProtocol;
    using cypherush::NetworkMessage;

    QByteArray buffer;
    buffer.append(MessageProtocol::serialize(
        NetworkMessage(NetworkMessage::Type::Login, QByteArray("first"))));
    buffer.append(MessageProtocol::serialize(NetworkMessage(
        NetworkMessage::Type::SendMessage, QByteArray("second"))));

    auto first = MessageProtocol::tryParse(buffer);
    const bool firstOk =
        first.has_value() &&
        first->getType() == NetworkMessage::Type::Login &&
        first->getPayload() == QByteArray("first") && !buffer.isEmpty();

    auto second = MessageProtocol::tryParse(buffer);
    const bool secondOk =
        second.has_value() &&
        second->getType() == NetworkMessage::Type::SendMessage &&
        second->getPayload() == QByteArray("second") && buffer.isEmpty();

    return firstOk && secondOk;
}

bool testProtocolInvalidType() {
    using cypherush::MessageProtocol;

    QByteArray buffer;
    buffer.resize(5);
    buffer[0] = '\x00';
    buffer[1] = '\x00';
    buffer[2] = '\x00';
    buffer[3] = '\x01';                       // length = 1
    buffer[4] = static_cast<char>(0xFF);      // invalid type byte

    try {
        MessageProtocol::tryParse(buffer);
        return false;  // should have thrown
    } catch (const cypherush::ProtocolException&) {
        return true;
    }
}

int runCase(const char* name, bool (*fn)()) {
    try {
        if (fn()) {
            std::cout << "[OK] " << name << " passed\n";
            return 0;
        }
        std::cout << "[FAIL] " << name << " mismatch\n";
    } catch (const std::exception& e) {
        std::cout << "[FAIL] " << name << " threw: " << e.what() << "\n";
    }
    return 1;
}

} // namespace

int runAllTests() {
    int failures = 0;

    failures += runCase("AES roundtrip", testAes);
    failures += runCase("RSA roundtrip", testRsa);
    failures += runCase("Hybrid E2E roundtrip", testHybridE2E);
    failures += runCase("KeyManager persistence + wrong-password rejection",
                        testKeyManagerPersistence);
    failures += runCase("PasswordHasher", testPasswordHasher);
    failures += runCase("Repository CRUD + persistence", testRepository);
    failures += runCase("Protocol roundtrip", testProtocolRoundtrip);
    failures += runCase("Protocol partial buffer rejected",
                        testProtocolPartialBuffer);
    failures += runCase("Protocol multi-message stream",
                        testProtocolMultiMessage);
    failures += runCase("Protocol invalid type rejected",
                        testProtocolInvalidType);

    return failures;
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    if (argc > 1 && std::string(argv[1]) == "--test") {
        return runAllTests();
    }

    qInfo() << "Cypherush server starting...";

    // Demo storage key — in production this must be derived from a master
    // password via PBKDF2 rather than hard-coded.
    std::vector<uint8_t> storageKey(32, 0x42);

    auto userRepo = std::make_shared<cypherush::UserRepository>(
        cypherush::EncryptedFileStorage("users.dat", storageKey));
    auto messageRepo = std::make_shared<cypherush::MessageRepository>(
        cypherush::EncryptedFileStorage("messages.dat", storageKey));

    cypherush::ChatServer server(userRepo, messageRepo, 55555);
    if (!server.start()) {
        qCritical() << "Failed to start ChatServer";
        return 1;
    }

    return app.exec();
}
