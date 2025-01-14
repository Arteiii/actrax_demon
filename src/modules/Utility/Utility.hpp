//
// Created by arteii on 12/2/24.
//

#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <Logger.hpp>
#include <Server.hpp>
#include <string>

namespace Utility {
inline auto BoolToString = [](const bool value) {
  return value ? "True" : "False";
};

auto
ConvertToWString(const std::string& str) -> std::wstring;

auto
SendHeartbeat(const Server::Session::Connect* connection) -> int;

} // namespace Utility

#endif // UTILITY_HPP
