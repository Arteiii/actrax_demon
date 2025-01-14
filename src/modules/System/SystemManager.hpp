//
// Created by arteii on 12/2/24.
//

#ifndef SYSTEMMANAGER_HPP
#define SYSTEMMANAGER_HPP

#include "../Utility/Utility.hpp"

#include <optional>

namespace System {

auto
SaveTokensToFile(const std::wstring& refreshToken, const std::wstring& clientId)
  -> void;

auto
LoadTokensFromFile()
  -> std::pair<std::optional<std::string>, std::optional<std::string>>;

} // namespace System

#endif // SYSTEMMANAGER_HPP
