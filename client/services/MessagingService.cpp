#include "services/MessagingService.h"

#include <algorithm>
#include <utility>

namespace cypherush {

MessagingService::MessagingService(std::shared_ptr<ChatClient> client,
                                   QObject* parent)
    : QObject(parent)
    , m_client(std::move(client)) {
    connect(m_client.get(), &ChatClient::publicKeyReceived, this,
            &MessagingService::onPublicKeyReceived);
    connect(m_client.get(), &ChatClient::messageReceived, this,
            &MessagingService::onClientMessageReceived);
    connect(m_client.get(), &ChatClient::errorOccurred, this,
            &MessagingService::onClientErrorOccurred);
    connect(m_client.get(), &ChatClient::authFailed, this,
            &MessagingService::onClientAuthFailed);
}

MessagingService::~MessagingService() = default;

void MessagingService::sendMessage(const std::string& recipientUsername,
                                   const std::string& plaintext) {
    auto it = m_publicKeyCache.find(recipientUsername);
    if (it != m_publicKeyCache.end()) {
        m_client->sendEncryptedMessage(recipientUsername, it->second,
                                       plaintext);
        return;
    }

    m_pendingSends.push_back({recipientUsername, plaintext});
    m_client->requestPublicKey(recipientUsername);
}

void MessagingService::requestContactKey(const std::string& username) {
    m_pendingContactKeyRequests.push_back(username);
    m_client->requestPublicKey(username);
}

void MessagingService::onPublicKeyReceived(
    const std::string& username, const std::vector<uint8_t>& publicKey) {
    m_publicKeyCache[username] = publicKey;

    auto it = m_pendingSends.begin();
    while (it != m_pendingSends.end()) {
        if (it->recipient == username) {
            m_client->sendEncryptedMessage(it->recipient, publicKey,
                                           it->plaintext);
            it = m_pendingSends.erase(it);
        } else {
            ++it;
        }
    }

    auto req = std::find(m_pendingContactKeyRequests.begin(),
                         m_pendingContactKeyRequests.end(), username);
    if (req != m_pendingContactKeyRequests.end()) {
        m_pendingContactKeyRequests.erase(req);
        emit contactKeyResolved(username, true);
    }
}

void MessagingService::onClientAuthFailed(const std::string&) {
    if (m_pendingContactKeyRequests.empty()) {
        return;  // belongs to the login flow; AuthenticationService owns it
    }
    const std::string username = m_pendingContactKeyRequests.front();
    m_pendingContactKeyRequests.pop_front();
    emit contactKeyResolved(username, false);
}

void MessagingService::onClientMessageReceived(const std::string& sender,
                                               const std::string& content,
                                               qint64 timestamp) {
    emit messageReceived(sender, content, timestamp);
}

void MessagingService::onClientErrorOccurred(const std::string& code,
                                             const std::string& message) {
    if (code == "decrypt_failed") {
        emit messageSendFailed("", message);
    }
}

} // namespace cypherush
