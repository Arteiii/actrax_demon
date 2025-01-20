export module systemManager;

#include <logger.hpp>

#include "stringEncryptionUtils.hpp"

#include <optional>
#include <string>

export namespace System {

export auto
SaveTokensToFile(const std::wstring& refreshToken, const std::wstring& clientId)
  -> void
{

  // TODO!: fix dup code
  const auto desktopPathA = SECURE_STR("C:\\mfg\\Actrax.txt");

  const auto desktopPathW =
    std::wstring(desktopPathA.begin(), desktopPathA.end());

  LOG_TRACE(L"File path: " + desktopPathW);

  if (std::wofstream file((desktopPathW.c_str()),
                          std::ios::out | std::ios::trunc);
      file.is_open()) {
    file << refreshToken << L"\n" << clientId << L"\n";
    LOG_TRACE(L"Tokens written successfully to file");
    file.close();
  } else {
    LOG_ERROR(L"Failed to open Actrax.txt for writing.");
  }
}

export auto
LoadTokensFromFile()
  -> std::pair<std::optional<std::string>, std::optional<std::string>>
{
  LOG_TRACE(SECURE_STR("Loading tokens from file..."));

  const auto desktopPathA = SECURE_STR("C:\\mfg\\Actrax.txt");

  const auto desktopPathW =
    std::wstring(desktopPathA.begin(), desktopPathA.end());

  if (!std::filesystem::exists(desktopPathW)) {
    LOG_ERROR(L"File not found: " + desktopPathW);
    return { std::nullopt, std::nullopt };
  }

  LOG_TRACE(SECURE_STR("File Found at: ") + desktopPathA);

  if (std::wifstream file((desktopPathW.c_str()), std::ios::in);
      file.is_open()) {
    std::wstring refreshTokenW;
    std::wstring clientIdW;

    std::getline(file, refreshTokenW);
    std::getline(file, clientIdW);

    file.close();

    if (!refreshTokenW.empty()) {
      LOG_TRACE(L"RefreshToken: " + refreshTokenW);
    } else {
      LOG_TRACE(L"RefreshToken: Not Found");
    }

    if (!clientIdW.empty()) {
      LOG_TRACE(L"ClientId: " + clientIdW);
    } else {
      LOG_TRACE(L"ClientId: Not Found");
    }

    return { refreshTokenW.empty()
               ? std::nullopt
               : std::make_optional<std::string>(refreshTokenW.begin(),
                                                 refreshTokenW.end()),
             clientIdW.empty() ? std::nullopt
                               : std::make_optional<std::string>(
                                   clientIdW.begin(), clientIdW.end()) };
  }

  LOG_ERROR(SECURE_STR("Failed to open Actrax.txt for reading."));
  return { std::nullopt, std::nullopt };
}

} // namespace System
