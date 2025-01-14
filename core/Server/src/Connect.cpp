//
// Created by arteii on 11/28/24.
//

#include "Helper.hpp"

#include <Server.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <winhttp.h>

namespace Server {

auto
Session::Connect::IsSessionTokenValid() const -> bool
{
  if (hConnect_ == nullptr) {
    LOG_ERROR("Connection handle is null.");
    return false;
  }

  if (client_->GetSessionToken() == nullptr) {
    LOG_ERROR("sessionToken handle is null.");
    return false;
  }

  if (client_->GetSessionToken()->IsExpired()) {
    LOG_ERROR("sessionToken is expired.");
    return false;
  }

  return true;
}

auto
Session::Connect::CreateAuthorizationHeader() const -> std::wstring
{
  auto sessionToken = client_->GetSessionToken()->GetToken();
  return L"Authorization: " +
         std::wstring(sessionToken.begin(), sessionToken.end());
}

auto
Session::Connect::CreateApplicationRequestHeaders() -> std::wstring
{
  return L"Content-Type: application/json";
}

auto
Session::Connect::CreateCombinedHeaders() const -> std::wstring
{
  const std::wstring appHeader = CreateApplicationRequestHeaders();
  const std::wstring authHeader = CreateAuthorizationHeader();

  return appHeader + L"\r\n" + authHeader;
}

Session::Connect::Connect(Session* session,
                          const std::wstring& server,
                          const INTERNET_PORT port,
                          Client* client)
  : hConnect_(nullptr)
  , session_(session)
  , client_(std::move(client))
{

  hConnect_ =
    WinHttpConnect(session_->GetSessionHandle(), server.c_str(), port, 0U);

  if (hConnect_ == nullptr) {
    throw std::runtime_error("Failed to connect to server: " +
                             std::to_string(GetLastError()));
  }

  if (client_->GetRefreshToken() == nullptr) {
    LOG_TRACE("Creating New User...");
    CreateClient();
    LOG_TRACE("Created New User Successfully");
  }

  if (client_->GetRefreshToken() != nullptr &&
      client_->GetClientId() != nullptr) {
    LOG_TRACE("Acquire Session Token...");
    GetSessionByRefreshToken();
    LOG_TRACE("Acquire Session Token Successfully");
  }

  LOG_TRACE("Finished Initializer");
}

auto
Session::Connect::GetTasks() const -> std::vector<Task>
{
  if (!IsSessionTokenValid()) {
    throw std::runtime_error("Invalid session token.");
  }

  const HINTERNET hRequest = WinHttpOpenRequest(
    hConnect_, L"GET", L"/tasks", nullptr, nullptr, nullptr, 0U);

  if (hRequest == nullptr) {
    throw std::runtime_error("Failed to open request handle.");
  }

  const BOOL requestSent = WinHttpSendRequest(
    hRequest, CreateCombinedHeaders().c_str(), -1, nullptr, 0, 0, 0U);

  if (requestSent == 0) {
    WinHttpCloseHandle(hRequest);
    throw std::runtime_error("Failed to send request.");
  }

  if (WinHttpReceiveResponse(hRequest, nullptr) == 0) {
    WinHttpCloseHandle(hRequest);
    throw std::runtime_error("Failed to receive response.");
  }

  DWORD statusCode = 0U;
  DWORD statusCodeSize = sizeof(statusCode);
  if (WinHttpQueryHeaders(hRequest,
                          WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                          nullptr,
                          &statusCode,
                          &statusCodeSize,
                          nullptr) == 0) {
    WinHttpCloseHandle(hRequest);
    throw std::runtime_error("Failed to query status code.");
  }

  if (statusCode != 200) {
    WinHttpCloseHandle(hRequest);
    throw std::runtime_error("Failed to retrieve tasks, status code: " +
                             std::to_string(statusCode));
  }

  DWORD dwSize = 0;
  DWORD dwDownloaded = 0;
  DWORD dwBufLen = 0;

  if (WinHttpQueryDataAvailable(hRequest, &dwSize) == 0) {
    WinHttpCloseHandle(hRequest);
    throw std::runtime_error("Failed to query data available.");
  }

  auto* pszOutBuffer = static_cast<LPSTR>(malloc(dwSize + 1));
  if (pszOutBuffer == nullptr) {
    WinHttpCloseHandle(hRequest);
    throw std::runtime_error("Failed to allocate memory for response body.");
  }

  if (WinHttpReadData(hRequest, pszOutBuffer, dwSize, &dwDownloaded) == 0) {
    free(pszOutBuffer);
    WinHttpCloseHandle(hRequest);
    throw std::runtime_error("Failed to read data.");
  }

  pszOutBuffer[dwDownloaded] = '\0';

  std::vector<Task> tasks;
  try {
    std::cout << "Debug: JSON response received:\n"
              << pszOutBuffer << std::endl;

    for (auto jsonResponse = nlohmann::json::parse(pszOutBuffer);
         const auto& taskData : jsonResponse) {
      Task task;
      from_json(taskData, task);
      tasks.push_back(task);
    }
  } catch (const std::exception& e) {
    free(pszOutBuffer);
    WinHttpCloseHandle(hRequest);

    throw std::runtime_error("Failed to parse JSON response: " +
                             std::string(e.what()));
  }

  free(pszOutBuffer);
  WinHttpCloseHandle(hRequest);

  return tasks;
}

auto
Session::Connect::Log(const log_level& logLevel,
                      const std::string& message) const -> void
{
  if (!IsSessionTokenValid()) {
    throw std::runtime_error("Invalid session token.");
  }

  const HINTERNET hRequest =
    WinHttpOpenRequest(hConnect_,
                       L"POST",
                       L"/log",
                       nullptr,
                       nullptr, // WINHTTP_NO_REFERER
                       nullptr, // WINHTTP_DEFAULT_ACCEPT_TYPES
                       0U);

  if (hRequest == nullptr) {
    LOG_ERROR("Failed to open request handle.");
    throw std::runtime_error("Failed to open request handle.");
  }

  // TODO!: fix placeholder with actual current user name
  const std::string jsonPayload =
    R"({"log_level":")" + Logger::GetLogLevelString(logLevel) +
    R"(","message":")" + message + R"(","user_name":"placeholder"})";

  const BOOL requestSent =
    WinHttpSendRequest(hRequest,
                       CreateCombinedHeaders().c_str(),
                       -1,
                       const_cast<char*>(jsonPayload.c_str()),
                       jsonPayload.length(),
                       jsonPayload.length(),
                       0U);

  LOG_TRACE("Request sent: " + std::to_string(requestSent));

  HANDLE_WINHTTP_CALL(requestSent, "Failed to send log request.", hRequest);

  HANDLE_WINHTTP_CALL(WinHttpReceiveResponse(hRequest, nullptr),
                      "Failed to receive response.",
                      hRequest);

  DWORD statusCode = 0U;
  DWORD statusCodeSize = sizeof(statusCode);

  HANDLE_WINHTTP_CALL(
    WinHttpQueryHeaders(hRequest,
                        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        nullptr,
                        &statusCode,
                        &statusCodeSize,
                        nullptr),
    "Failed to query status code.",
    hRequest);

  LOG_TRACE("Received status code: " + std::to_string(statusCode));
  LOG_STATUS(statusCode == HttpOk, "Log sent successfully!");
  LOG_STATUS(statusCode != HttpOk,
             "Server responded with status code: " +
               std::to_string(statusCode));

  HANDLE_ERROR(WinHttpCloseHandle(hRequest) != 0,
               "Failed to close request handle.");
}

Session::Connect::~Connect()
{
  if (hConnect_ != nullptr) {
    if (WinHttpCloseHandle(hConnect_) == 0) {
      const DWORD error = GetLastError();
      LOG_WARN("Failed to close connection: " + error);
    }
  }
}

auto
Session::Connect::CreateClient() const -> void
{
  if (client_->GetClientUid() == nullptr) {
    LOG_ERROR("Missing ClientUid for Create User.");
    throw std::runtime_error("Missing ClientUid for Create User.");
  }

  if (client_->GetClientName() == nullptr) {
    LOG_ERROR("Missing clientName for Create User.");
    throw std::runtime_error("Missing clientName for Create User.");
  }

  const HINTERNET hRequest =
    WinHttpOpenRequest(hConnect_,
                       L"POST",
                       L"/client",
                       nullptr,
                       nullptr, // WINHTTP_NO_REFERER
                       nullptr, // WINHTTP_DEFAULT_ACCEPT_TYPES
                       0U);

  HANDLE_WINHTTP_CALL(
    hRequest != nullptr, "Failed to open request handle.", hRequest);

  const std::string jsonPayload = R"({"name":")" + *client_->GetClientName() +
                                  R"(","uid":")" + *client_->GetClientUid() +
                                  R"("})";

  LOG_TRACE("Json Payload Created...");
  const BOOL requestSent =
    WinHttpSendRequest(hRequest,
                       CreateApplicationRequestHeaders().c_str(),
                       -1,
                       const_cast<char*>(jsonPayload.c_str()),
                       jsonPayload.length(),
                       jsonPayload.length(),
                       0U);

  HANDLE_WINHTTP_CALL(
    requestSent != FALSE, "Failed to send /token request.", hRequest);

  HANDLE_WINHTTP_CALL(WinHttpReceiveResponse(hRequest, nullptr) != FALSE,
                      "Failed to receive response for /token.",
                      hRequest);

  DWORD statusCode = 0U;
  DWORD statusCodeSize = sizeof(statusCode);

  HANDLE_WINHTTP_CALL(
    WinHttpQueryHeaders(hRequest,
                        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        nullptr,
                        &statusCode,
                        &statusCodeSize,
                        nullptr) != 0,
    "Failed to query status code for /token.",
    hRequest);

  LOG_TRACE("Server responded with status code: " + std::to_string(statusCode));

  if (statusCode != HttpOk) {
    LOG_ERROR("Server responded with error code: " +
              std::to_string(statusCode));
    HANDLE_WINHTTP_CALL(false, "Server responded with error.", hRequest);
  }

  if (statusCode == HttpOk) {
    LOG_INFO("Register Client sent successfully!");

    DWORD availableSize = 0;
    std::string responseBody;

    do {
      HANDLE_WINHTTP_CALL(WinHttpQueryDataAvailable(hRequest, &availableSize) !=
                            0,
                          "Failed to query data availability.",
                          hRequest);

      if (availableSize > 0) {
        std::vector<char> buffer(availableSize + 1);
        DWORD bytesRead = 0;

        if (WinHttpReadData(
              hRequest, buffer.data(), availableSize, &bytesRead) == 0) {
          LOG_ERROR("Failed to read response data.");
          WinHttpCloseHandle(hRequest);
          throw std::runtime_error("Failed to read response data.");
        }

        buffer[bytesRead] = '\0'; // for safety
        responseBody.append(buffer.data(), bytesRead);
      }
    } while (availableSize > 0);

    LOG_TRACE("Response body: " + responseBody);

    try {
      if (nlohmann::json jsonResponse = nlohmann::json::parse(responseBody);
          jsonResponse.contains("refresh_token")) {

        client_->SetRefreshToken(std::make_unique<std::string>(
          jsonResponse["refresh_token"].get<std::string>()));

        client_->SetClientId(std::make_unique<std::string>(
          jsonResponse["client_id"].get<std::string>()));

      } else {
        LOG_ERROR("Response JSON does not contain 'refresh_token'.");
        throw std::runtime_error("Missing 'refresh_token' in server response.");
      }
    } catch (const std::exception& ex) {
      LOG_ERROR("Failed to parse response JSON: " + std::string(ex.what()));
      throw;
    }
  } else {
    LOG_ERROR("Server responded with status code: " +
              std::to_string(statusCode));
    HANDLE_WINHTTP_CALL(false, "Server responded with error.", hRequest);
  }

  HANDLE_WINHTTP_CALL(WinHttpCloseHandle(hRequest) != 0,
                      "Failed to close request handle.",
                      hRequest);
}

auto
Session::Connect::GetSessionByRefreshToken() const -> void
{

  const HINTERNET hRequest =
    WinHttpOpenRequest(hConnect_,
                       L"GET",
                       L"/token",
                       nullptr,
                       nullptr, // WINHTTP_NO_REFERER
                       nullptr, // WINHTTP_DEFAULT_ACCEPT_TYPES
                       0U);

  HANDLE_WINHTTP_CALL(
    hRequest != nullptr, "Failed to open request handle for /token.", hRequest);

  LOG_TRACE("Creating Json Payload...");
  const std::string jsonPayload =
    R"({"refresh_token":")" + *client_->GetRefreshToken() +
    R"(","client_id":")" + *client_->GetClientId() + R"("})";

  LOG_TRACE("Sending Request...");
  const BOOL requestSent =
    WinHttpSendRequest(hRequest,
                       CreateApplicationRequestHeaders().c_str(),
                       -1,
                       const_cast<char*>(jsonPayload.c_str()),
                       jsonPayload.length(),
                       jsonPayload.length(),
                       0U);

  LOG_TRACE("Succesfully Send Request");

  HANDLE_WINHTTP_CALL(
    requestSent != FALSE, "Failed to send /token request.", hRequest);

  HANDLE_WINHTTP_CALL(WinHttpReceiveResponse(hRequest, nullptr) != FALSE,
                      "Failed to receive response for /token.",
                      hRequest);

  DWORD statusCode = 0;
  DWORD statusCodeSize = sizeof(statusCode);

  HANDLE_WINHTTP_CALL(
    WinHttpQueryHeaders(hRequest,
                        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        nullptr,
                        &statusCode,
                        &statusCodeSize,
                        nullptr) != 0,
    "Failed to query status code for /token.",
    hRequest);

  if (statusCode != HttpOk) {
    LOG_ERROR("Server responded with status code: " +
              std::to_string(statusCode));
    HANDLE_WINHTTP_CALL(false, "Server responded with error.", hRequest);
  }

  DWORD availableSize = 0;
  std::string responseBody;

  do {
    HANDLE_WINHTTP_CALL(WinHttpQueryDataAvailable(hRequest, &availableSize) !=
                          0,
                        "Failed to query data availability.",
                        hRequest);

    if (availableSize > 0) {
      std::vector<char> buffer(availableSize + 1);
      DWORD bytesRead = 0;

      if (WinHttpReadData(hRequest, buffer.data(), availableSize, &bytesRead) ==
          0) {
        LOG_ERROR("Failed to read response data.");
        WinHttpCloseHandle(hRequest);
        throw std::runtime_error("Failed to read response data.");
      }

      buffer[bytesRead] = '\0';
      responseBody.append(buffer.data(), bytesRead);
    }
  } while (availableSize > 0);

  LOG_TRACE("Response body: " + responseBody);

  try {
    if (nlohmann::json jsonResponse = nlohmann::json::parse(responseBody);
        jsonResponse.contains("session_token") &&
        jsonResponse.contains("refresh_token")) {
      const std::string& sessionToken =
        jsonResponse["session_token"].get<std::string>();
      constexpr auto expiresIn =
        static_cast<const int64_t>(20 * 60);

      LOG_TRACE("Creating New SessionToken...");
      const auto newSessionToken = std::make_shared<SessionToken>();
      newSessionToken->SetToken(sessionToken, expiresIn);

      client_->SetSessionToken(newSessionToken);
      LOG_INFO("Session token set successfully");

      const std::string newRefreshToken =
        jsonResponse["refresh_token"].get<std::string>();
      client_->SetRefreshToken(std::make_unique<std::string>(newRefreshToken));
      LOG_INFO("Refresh token set successfully.");
    } else {
      LOG_ERROR("Response JSON missing required fields.");
      throw std::runtime_error("Invalid response: Missing required fields.");
    }
  } catch (const std::exception& ex) {
    LOG_ERROR("Error while processing /token response: " +
              std::string(ex.what()));
    throw;
  }

  HANDLE_WINHTTP_CALL(WinHttpCloseHandle(hRequest) != 0,
                      "Failed to close request handle for /token.",
                      hRequest);
}

} // namespace Server
