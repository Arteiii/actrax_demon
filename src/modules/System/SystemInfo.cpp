//
// Created by arteii on 11/2/24.
//

#include "SystemInfo.hpp"
#include "Logger.hpp"
#include "RegistryHelper.hpp"
#include "src/modules/ApplicationController/ApplicationController.hpp"

#include <iphlpapi.h>

#include <windows.h>

#include <array>
#include <iostream>

namespace WinInfo {

constexpr uintptr_t KUserSharedDataAddress =
  0x7FFE0000U; /// KUSER_SHARED_DATA -
               /// https://www.geoffchappell.com/studies/windows/km/ntoskrnl/inc/api/ntexapi_x/kuser_shared_data/index.htm
constexpr std::size_t MaxUsernameLen = 256;
constexpr std::size_t Sha256HashSize = 32;

auto
Version() -> std::tuple<const ULONG, const ULONG, const ULONG>
{
  constexpr ULONG majorVersionOffset = 0x26CU; // major version
  constexpr ULONG minorVersionOffset = 0x270U; // minor version
  constexpr ULONG buildNumberOffset = 0x260U;  // build number

  const ULONG majorVersion = *reinterpret_cast<const ULONG*>(
    KUserSharedDataAddress + majorVersionOffset);
  const ULONG minorVersion = *reinterpret_cast<const ULONG*>(
    KUserSharedDataAddress + minorVersionOffset);
  const ULONG buildNumber =
    *reinterpret_cast<const ULONG*>(KUserSharedDataAddress + buildNumberOffset);

  LOG_TRACE("Full Version: " + std::to_string(majorVersion) + "." +
            std::to_string(minorVersion) + "." + std::to_string(buildNumber));

  return std::make_tuple(majorVersion, minorVersion, buildNumber);
}

/// Function to get the MAC address
auto
GetMacAddress() -> std::string
{
  ULONG macAddrLen = 0;

  if (GetAdaptersInfo(nullptr, &macAddrLen) == ERROR_BUFFER_OVERFLOW) {
    std::vector<char> adapterInfo(macAddrLen);

    if (GetAdaptersInfo(reinterpret_cast<IP_ADAPTER_INFO*>(adapterInfo.data()),
                        &macAddrLen) == ERROR_SUCCESS) {
      const auto* pAdapterInfo =
        reinterpret_cast<PIP_ADAPTER_INFO>(adapterInfo.data());

      while (pAdapterInfo != nullptr) {
        if (pAdapterInfo->AddressLength == 6U) {
          std::ostringstream oss;
          for (int i = 0; i < 6U; ++i) {
            oss << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(pAdapterInfo->Address[i]);
            if (i != 5U) {
              oss << ':';
            }
          }
          LOG_TRACE("MAC Address: " + oss.str());
          return oss.str();
        }
        pAdapterInfo = pAdapterInfo->Next;
      }
    } else {
      LOG_ERROR("GetAdaptersInfo failed with error: " + GetLastError());
    }
  } else {
    LOG_ERROR("Initial call to GetAdaptersInfo failed with error: " +
              GetLastError());
  }

  return {};
}

// Function to get the Windows product key
auto
GetWindowsProductKey() -> std::string
{
  try {
    auto productKeyWide = Registry::Helper::RegGetString(
      HKEY_LOCAL_MACHINE,
      LR"(SOFTWARE\Microsoft\Windows NT\CurrentVersion\SoftwareProtectionPlatform)",
      L"BackupProductKeyDefault");

    if (productKeyWide.empty()) {
      LOG_TRACE("Warning: Product key is empty or not found in the registry.");
      return "";
    }

    std::string productKey(productKeyWide.begin(), productKeyWide.end());

    LOG_TRACE("Windows Product Key: " + productKey);
    return productKey;
  } catch (const std::exception& e) {
    LOG_ERROR("Error retrieving Windows Product Key: " + std::string(e.what()));
    return "";
  } catch (...) {
    LOG_ERROR(
      "Error retrieving Windows Product Key: Unknown exception occurred.");
    return "";
  }
}

// Function to get the Microsoft username
auto
GetMicrosoftUsername() -> std::string
{
  std::array<char, MaxUsernameLen> username;
  auto usernameLen = static_cast<DWORD>(username.size());

  // https://learn.microsoft.com/windows/win32/api/winbase/nf-winbase-getusernamea
  if (GetUserNameA(username.data(), &usernameLen) != ERROR_SUCCESS) {
    LOG_TRACE("Microsoft Username: " +
              std::string(username.data(), usernameLen - 1));
    return { username.data(), usernameLen - 1 };
  }

  const DWORD error = GetLastError();
  char* errorMsg = nullptr;
  FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                   FORMAT_MESSAGE_IGNORE_INSERTS,
                 nullptr,
                 error,
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 reinterpret_cast<LPSTR>(&errorMsg),
                 0,
                 nullptr);

  std::ostringstream oss;
  oss << "Failed to retrieve username (Error " << error << ": "
      << ((errorMsg != nullptr) ? errorMsg : "Unknown error") << ")";

  if (errorMsg != nullptr) {
    LocalFree(errorMsg);
  }

  LOG_ERROR(oss.str());
  throw std::runtime_error(oss.str());
}

auto
GenerateSecureIdentifier() -> std::string
{

  const std::string macAddress = GetMacAddress();
  const std::string productKey = GetWindowsProductKey();
  const std::string username = GetMicrosoftUsername();

  const std::string identifierInput = macAddress + productKey + username;

  BCRYPT_ALG_HANDLE hAlg = nullptr;
  BCRYPT_HASH_HANDLE hHash = nullptr;
  std::array<BYTE, Sha256HashSize> hash;

  NTSTATUS status =
    BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);

  constexpr LONG statusSuccess = 0;
  if (status != statusSuccess) {
    LOG_ERROR("Failed to open algorithm provider for SHA-256. NTSTATUS: " +
              status);
    return "";
  }

  status = BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0);
  if (status != statusSuccess) {
    if (const NTSTATUS closeStatus = BCryptCloseAlgorithmProvider(hAlg, 0);
        closeStatus != statusSuccess) {
      LOG_ERROR("Failed to close algorithm provider. NTSTATUS: " + closeStatus);
    }
    LOG_ERROR("Failed to create hash object. NTSTATUS: " + status);
    return "";
  }

  status = BCryptHashData(
    hHash,
    const_cast<PUCHAR>(reinterpret_cast<const BYTE*>(identifierInput.c_str())),
    static_cast<ULONG>(identifierInput.size()),
    0);
  if (status != statusSuccess) {
    if (const NTSTATUS closeStatus = BCryptDestroyHash(hHash);
        closeStatus != statusSuccess) {
      LOG_ERROR("Failed to destroy hash object. NTSTATUS: " + closeStatus);
    }

    if (const NTSTATUS closeStatus = BCryptCloseAlgorithmProvider(hAlg, 0);
        closeStatus != statusSuccess) {
      LOG_ERROR("Failed to close algorithm provider. NTSTATUS: " + closeStatus);
    }
    LOG_ERROR("Failed to hash data. NTSTATUS: " + status);
    return "";
  }

  status = BCryptFinishHash(hHash, hash.data(), hash.size(), 0);
  if (status != statusSuccess) {
    if (const NTSTATUS closeStatus = BCryptDestroyHash(hHash);
        closeStatus != statusSuccess) {
      LOG_ERROR("Failed to destroy hash object. NTSTATUS: " + closeStatus);
    }

    if (const NTSTATUS closeStatus = BCryptCloseAlgorithmProvider(hAlg, 0);
        closeStatus != statusSuccess) {
      LOG_ERROR("Failed to close algorithm provider. NTSTATUS: " + closeStatus);
    }
    LOG_ERROR("Failed to finish hash. NTSTATUS: " + status);
    return "";
  }

  status = BCryptDestroyHash(hHash);
  if (status != statusSuccess) {
    if (const NTSTATUS closeStatus = BCryptCloseAlgorithmProvider(hAlg, 0);
        closeStatus != statusSuccess) {
      LOG_ERROR("Failed to close algorithm provider. NTSTATUS: " + closeStatus);
    }
    LOG_ERROR("Failed to destroy hash object. NTSTATUS: " + status);
    return "";
  }

  status = BCryptCloseAlgorithmProvider(hAlg, 0);
  if (status != statusSuccess) {
    LOG_ERROR("Failed to close algorithm provider. NTSTATUS: " + status);
    return "";
  }

  std::ostringstream hexStream;
  for (const BYTE byte : hash) {
    hexStream << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(byte);
  }

  LOG_TRACE("Generated Secure Identifier: " + hexStream.str());
  return hexStream.str();
}

auto
GetDesktopPathFromRegistry() -> std::wstring
{
  try {
    const auto subKeyA =
      SECURE_STR(
        "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders")
        ;

    return Registry::Helper::RegGetString(
      HKEY_CURRENT_USER,
      std::wstring(subKeyA.begin(), subKeyA.end()),
      L"Desktop");

  } catch (const std::exception& e) {
    LOG_ERROR("Error: " + std::string(e.what()));
    __throw_exception_again;
  }
}

} // namespace WinInfo
