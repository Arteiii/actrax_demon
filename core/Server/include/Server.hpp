//
// Created by arteii on 11/2/24.
//

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Tasks.hpp"

#include <Logger.hpp>

#include <memory>
#include <string>

namespace Server {
// Define HTTP status codes
constexpr unsigned int HttpOk = 200U;
constexpr unsigned int HttpCreated = 201U;
constexpr unsigned int HttpAccepted = 202U;
constexpr unsigned int HttpNoContent = 204U;
constexpr unsigned int HttpBadRequest = 400U;
constexpr unsigned int HttpUnauthorized = 401U;
constexpr unsigned int HttpForbidden = 403U;
constexpr unsigned int HttpNotFound = 404U;
constexpr unsigned int HttpMethodNotAllowed = 405U;
constexpr unsigned int HttpInternalServerError = 500U;
constexpr unsigned int HttpNotImplemented = 501U;
constexpr unsigned int HttpBadGateway = 502U;
constexpr unsigned int HttpServiceUnavailable = 503U;

using HINTERNET = void*;
using INTERNET_PORT = unsigned short;

class Session
{
public:
  explicit Session(const std::wstring& agent);

  ~Session();

  [[nodiscard]] auto
  GetSessionHandle() const -> HINTERNET;

  class Connect
  {
  public:
    // placed here for now have to rework later idk
    // TODO: rework into own module
    Connect(Session* session,
            const std::wstring& server,
            INTERNET_PORT port,
            Client* client);

    ~Connect();

    auto
    Log(const log_level& logLevel, const std::string& message) const -> void;

    [[nodiscard]] auto
    GetTasks() const -> std::vector<Task>;

    [[nodiscard]] auto
    GetClient() const -> const std::shared_ptr<Client>&;

  private:
    HINTERNET hConnect_;
    Session* session_;
    std::shared_ptr<Client> client_;

  protected:
    [[nodiscard]] auto
    IsSessionTokenValid() const -> bool;

    [[nodiscard]] auto
    CreateAuthorizationHeader() const -> std::wstring;

    static auto
    CreateApplicationRequestHeaders() -> std::wstring;

    [[nodiscard]] auto
    CreateCombinedHeaders() const -> std::wstring;

    auto
    CreateClient() const -> void;

    auto
    GetSessionByRefreshToken() const -> void;
  };

private:
  HINTERNET hSession_;
};
} // namespace Server

#endif // SERVER_HPP
