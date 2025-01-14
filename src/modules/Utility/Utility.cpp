//
// Created by arteii on 12/2/24.
//

#include "Utility.hpp"

namespace Utility {

auto
ConvertToWString(const std::string& str) -> std::wstring
{
  LOG_TRACE("Converting string to wstring: " + str);

  if (str.empty()) {
    return L"";
  }

  const size_t sizeRequired = std::mbstowcs(nullptr, str.c_str(), 0);
  if (sizeRequired == static_cast<size_t>(-1)) {
    throw std::runtime_error("Failed to convert string to wstring");
  }

  std::wstring wstr(sizeRequired, L'\0');
  std::mbstowcs(wstr.data(), str.c_str(), sizeRequired);

  LOG_TRACE(L"covnerted wstring:" + wstr);

  return wstr;
}

auto
SendHeartbeat(const Server::Session::Connect* connection) -> int
{
  LOG_TRACE("Heartbeat sent to server.");
  connection->Log(log_level::Info, "Heartbeat sent successfully.");

  return 1;
}

} // namespace Utility
