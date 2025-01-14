//
// Created by arteii on 11/28/24.
//

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Logger.hpp"
#include "SessionToken.hpp"

#include <functional>
#include <memory>
#include <string>

namespace Server {

struct ClientConfig
{
  std::shared_ptr<SessionToken> sessionToken;
  std::string* clientId;
  std::string* clientUid;
  std::string* refreshToken;
  std::string* clientName;
};

class Client
{
public:
  explicit Client(const ClientConfig& config)
    : sessionToken_(config.sessionToken)
    , clientId_((config.clientId != nullptr)
                  ? std::make_shared<std::string>(*config.clientId)
                  : nullptr)
    , clientUid_(std::make_shared<std::string>(*config.clientUid))
    , refreshToken_((config.refreshToken != nullptr)
                      ? std::make_shared<std::string>(*config.refreshToken)
                      : nullptr)
    , clientName_(std::make_shared<std::string>(*config.clientName))
  {
  }

  [[nodiscard]] auto
  GetSessionToken() const -> std::shared_ptr<SessionToken>
  {
    return sessionToken_;
  }

  auto
  SetSessionToken(const std::shared_ptr<SessionToken>& token) -> void
  {
    sessionToken_ = token;
  }

  auto
  SetClientId(const std::shared_ptr<std::string>& newClientId) -> void
  {
    clientId_ = newClientId;
  }

  [[nodiscard]] auto
  GetClientId() const -> std::shared_ptr<std::string>
  {
    return clientId_;
  }

  [[nodiscard]] auto
  GetClientUid() const -> std::shared_ptr<std::string>
  {
    return clientUid_;
  }

  [[nodiscard]] auto
  GetClientName() const -> std::shared_ptr<std::string>
  {
    return clientName_;
  }

  [[nodiscard]] auto
  GetRefreshToken() const -> std::shared_ptr<std::string>
  {
    LOG_TRACE("Client::getRefreshToken()");
    return refreshToken_;
  }

  auto
  SetRefreshToken(const std::shared_ptr<std::string>& token) -> void
  {
    LOG_TRACE("Client::setRefreshToken(" + *token + ")");

    if (!refreshToken_) {
      LOG_TRACE("Refresh Token is a nullptr");
      refreshToken_ = token;
    } else {
      LOG_TRACE("Refresh Token is already set");
      *refreshToken_ = *token;
    }

    LOG_TRACE("Assigned new token value: " + *token);

    if (refreshTokenCallback_) {
      LOG_TRACE("Invoking refresh token callback...");

      if (!refreshToken_ || !clientId_) {
        LOG_WARN("Refresh token or client ID is nullptr. Skipping callback.");
        return;
      }

      refreshTokenCallback_(refreshToken_, clientId_);
    } else {
      LOG_TRACE("No callback set. Continuing...");
    }
  }

  auto
  SetRefreshTokenUpdateCallback(
    const std::function<void(std::shared_ptr<std::string>,
                             std::shared_ptr<std::string>)>& callback) -> void
  {
    refreshTokenCallback_ = callback;
  }

private:
  std::shared_ptr<SessionToken> sessionToken_;
  std::shared_ptr<std::string> clientId_;
  std::shared_ptr<std::string> clientUid_;
  std::shared_ptr<std::string> refreshToken_;
  std::shared_ptr<std::string> clientName_;

  std::function<void(std::shared_ptr<std::string>,
                     std::shared_ptr<std::string>)>
    refreshTokenCallback_;
};

} // namespace Server

#endif // CLIENT_HPP
