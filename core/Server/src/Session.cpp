//
// Created by arteii on 11/2/24.
//

#include "Server.hpp"

#include <nlohmann/json.hpp>

namespace Server {

using WinHttpOpenProc =
  HINTERNET(WINAPI*)(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD);

using WinHttpCloseHandleProc = BOOL(WINAPI*)(HINTERNET);

class WinHttpLoader
{
public:
  WinHttpLoader()
  {
    hWinHttpDll_ = LoadLibraryW(L"winhttp.dll");
    if (!hWinHttpDll_) {
      throw std::runtime_error("Failed to load winhttp.dll");
    }

    winHttpOpen_ = reinterpret_cast<WinHttpOpenProc>(
      GetProcAddress(hWinHttpDll_, "WinHttpOpen"));
    winHttpCloseHandle_ = reinterpret_cast<WinHttpCloseHandleProc>(
      GetProcAddress(hWinHttpDll_, "WinHttpCloseHandle"));

    if (!winHttpOpen_ || !winHttpCloseHandle_) {
      FreeLibrary(hWinHttpDll_);
      throw std::runtime_error("Failed to load required WinHTTP functions");
    }
  }

  ~WinHttpLoader()
  {
    if (hWinHttpDll_) {
      FreeLibrary(hWinHttpDll_);
    }
  }

  auto
  WinHttpOpen(const wchar_t* userAgent,
              const DWORD accessType,
              const wchar_t* proxyName,
              const wchar_t* proxyBypass,
              const DWORD flags) const -> HINTERNET
  {
    return winHttpOpen_(userAgent, accessType, proxyName, proxyBypass, flags);
  }

  auto
  WinHttpCloseHandle(HINTERNET handle) const -> BOOL
  {
    return winHttpCloseHandle_(handle);
  }

private:
  HMODULE hWinHttpDll_{ nullptr };
  WinHttpOpenProc winHttpOpen_{ nullptr };
  WinHttpCloseHandleProc winHttpCloseHandle_{ nullptr };
};

static auto
GetWinHttpLoader() -> const WinHttpLoader&

{
  static WinHttpLoader loader;
  return loader;
}

Session::Session(const std::wstring& agent)
{
  const auto& loader = GetWinHttpLoader();
  hSession_ = loader.WinHttpOpen(agent.c_str(),
                                 0,       // WINHTTP_ACCESS_TYPE_DEFAULT_PROXY
                                 nullptr, // WINHTTP_NO_PROXY_NAME
                                 nullptr, // WINHTTP_NO_PROXY_BYPASS
                                 0U);
  if (!hSession_) {
    throw std::runtime_error("Failed to create WinHTTP session");
  }
}

Session::~Session()
{
  if (hSession_ != nullptr) {
    const auto& loader = GetWinHttpLoader();
    (void)loader.WinHttpCloseHandle(hSession_);
  }
}

auto
Session::GetSessionHandle() const -> HINTERNET
{
  return hSession_;
}

} // namespace Server