#include "pch.h"

#include "Logger.hpp"
#include "registryGuard.hpp"

namespace Registry {
RegistryGuard::RegistryGuard(const int checkIntervalMs, callback clb)
  : checkIntervalMs_(checkIntervalMs)
  , callback_(std::move(clb))
  , running_(false)
{
}

RegistryGuard::~RegistryGuard()
{
  Stop();
}

// Add a registry entry to monitor
auto
RegistryGuard::AddEntry(const EntryConfig& config) -> void
{
  HKEY hKey;
  const LONG result =
    RegOpenKeyExW(config.hKey, config.subKey.c_str(), 0U, KEY_READ, &hKey);

  if (result != ERROR_SUCCESS) {
    std::wstring errorMsg = L"[RegGuard] Unable to open registry key: " +
                            config.subKey + L" (Error code: " +
                            std::to_wstring(result) + L")";
    LOG_ERROR(errorMsg);

    const auto errorMsgBegin = errorMsg.begin();
    const auto errorMsgEnd = errorMsg.end();

    throw std::runtime_error(std::string(errorMsgBegin, errorMsgEnd));
  }

  (void)RegCloseKey(hKey);

  LOG_TRACE("[RegGuard] Successfully Added Entry!");

  entryConfigs_.push_back(config);
}

// Start monitoring
auto
RegistryGuard::Start() -> void
{
  running_ = true;
  monitorThread_ =
    CreateThread(nullptr, 0U, &RegistryGuard::Monitor, this, 0U, nullptr);
}

// Stop monitoring
auto
RegistryGuard::Stop() -> void
{
  running_ = false;
  if (monitorThread_ != nullptr) {
    WaitForSingleObject(monitorThread_, INFINITE);
    CloseHandle(monitorThread_);
    monitorThread_ = nullptr;
  }
}

// Monitor entries
auto WINAPI
RegistryGuard::Monitor(const LPVOID lpParam) -> DWORD
{
  auto* instance = static_cast<RegistryGuard*>(lpParam);
  while (instance->running_) {
    instance->CheckEntries();
    Sleep(instance->checkIntervalMs_);
  }
  return 0U;
}

auto
RegistryGuard::CreateRegistryEntry(
  HKEY hKey,
  const std::wstring& subKey,
  const std::wstring& valueName,
  DWORD regType,
  const std::vector<std::wstring>& allowedValues) -> EntryConfig
{
  return EntryConfig{ hKey, subKey, valueName, regType, allowedValues };
}

auto
RegistryGuard::CheckEntries() -> void
{
  LOG_DEBUG("[RegGuard] Checking registry entries...");

  for (const auto& config : entryConfigs_) {
    try {
      HKEY hOpenedKey;
      if constexpr (IsLogLevelEnabled(log_level::Trace)) {
        Logger::Log(log_level::Trace,
                    L"[RegGuard] Attempting to open registry key: " +
                      config.subKey,
                    "RegistryGuard.cpp",
                    9);
      };

      if (RegOpenKeyExW(
            config.hKey, config.subKey.c_str(), 0U, KEY_READ, &hOpenedKey) !=
          ERROR_SUCCESS) {
        LOG_ERROR(L"[RegGuard] Failed to open registry key: " + config.subKey);
        continue;
      }
      LOG_TRACE(L"[RegGuard] Successfully opened registry key: " +
                config.subKey);

      DWORD dataType;
      std::array<BYTE, BufferSize> data;
      auto dataSize = static_cast<DWORD>(data.size());
      std::wstring currentValue;

      LOG_TRACE(L"[RegGuard] Querying registry value: " + config.valueName);
      if (RegQueryValueExW(hOpenedKey,
                           config.valueName.c_str(),
                           nullptr,
                           &dataType,
                           data.data(),
                           &dataSize) == ERROR_SUCCESS) {
        LOG_TRACE(L"[RegGuard] Successfully queried registry value: " +
                  config.valueName);

        if (dataType == REG_DWORD && dataSize == sizeof(DWORD)) {
          const DWORD dwordValue = *reinterpret_cast<DWORD*>(data.data());
          currentValue = std::to_wstring(dwordValue);
          LOG_TRACE(L"[RegGuard] Read DWORD value: " + currentValue);
        } else if (dataType == REG_SZ || dataType == REG_EXPAND_SZ) {
          currentValue = reinterpret_cast<wchar_t*>(data.data());
          LOG_TRACE(L"[RegGuard] Read String value: " + currentValue);
        } else {
          LOG_WARN(L"[RegGuard] Unsupported registry data type for value: " +
                   config.valueName + L" (DataType: " +
                   std::to_wstring(dataType) + L")");
          (void)RegCloseKey(hOpenedKey);
          continue;
        }
      } else {
        LOG_ERROR(L"[RegGuard] Failed to query registry value: " +
                  config.valueName);
        (void)RegCloseKey(hOpenedKey);
        continue;
      }

      LOG_TRACE(L"[RegGuard] Checking if current value '" + currentValue +
                L"' is among allowed values...");
      if (std::find(config.allowedValues.begin(),
                    config.allowedValues.end(),
                    currentValue) == config.allowedValues.end()) {
        LOG_WARN(L"[RegGuard] Registry value changed for " + config.valueName +
                 L" (Current Value: " + currentValue + L")");
        callback_(config.valueName,
                  currentValue,
                  config); // Updated callback invocation
      } else {
        LOG_TRACE(L"[RegGuard] Current value is among allowed values for " +
                  config.valueName);
      }

      (void)RegCloseKey(hOpenedKey);
      LOG_TRACE(L"[RegGuard] Closed registry key: " + config.subKey);
    } catch (const std::exception& e) {
      LOG_ERROR(L"[RegGuard] Error reading registry entry " + config.valueName +
                L": " + std::wstring(e.what(), e.what() + strlen(e.what())));
    }
  }
}

} // namespace Registry
