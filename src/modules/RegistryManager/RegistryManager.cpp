#include "RegistryManager.hpp"

#include "src/modules/ApplicationController/ApplicationController.hpp"

#include <Logger.hpp>

namespace Reg {

RegistryManager::RegistryManager()
  : regGuard_(DefaultDelayMs, OnRegistryChange)
{
  Initialize();
  AddRegistryEntry(CrashDumpEnabled());
}

RegistryManager::RegistryManager(
  const std::vector<Registry::EntryConfig>& customEntries)
  : regGuard_(DefaultDelayMs, OnRegistryChange)
{
  Initialize();
  for (const auto& entry : customEntries) {
    AddRegistryEntry(entry);
  }
}

RegistryManager::~RegistryManager()
{
  Cleanup();
}

auto
RegistryManager::Initialize() -> void
{
  LOG_INFO("Registry Manager initializing...");
  regGuard_.Start();
}

auto
RegistryManager::Cleanup() -> void
{
  LOG_INFO("Cleaning up Registry Manager...");
  regGuard_.Stop();
}

auto
RegistryManager::OnRegistryChange(const std::wstring& valueName,
                                  const std::wstring& currentValue,
                                  const Registry::EntryConfig& entry) -> void
{
  const std::wstring logMessage = L"Registry value changed: " + valueName +
                                  L" (Current Value: " + currentValue +
                                  L", RegPath: " + entry.subKey +
                                  L", ValueName: " + entry.valueName + L")";
  LOG_INFO(logMessage);
  ApplicationController::ImmediateExit();
}

auto
RegistryManager::AddRegistryEntry(const Registry::EntryConfig& config) -> void
{
  try {
    regGuard_.AddEntry(config);
    LOG_INFO(L"Successfully added entry for: " + config.valueName);
  } catch (const std::runtime_error& e) {
    LOG_ERROR(L"Failed to add entry for '" + config.valueName + L"': " +
              std::wstring(e.what(), e.what() + strlen(e.what())));
  }
}

auto
RegistryManager::CrashDumpEnabled() -> Registry::EntryConfig
{
  const auto crashDumpEnabled = CreateRegistryEntry(
    HKEY_LOCAL_MACHINE,
    LR"#(SYSTEM\CurrentControlSet\Control\CrashControl)#", // SubKey
    L"CrashDumpEnabled",                                   // ValueName
    REG_DWORD,                                             // RegType
    { L"0" }                                               // AllowedValues
  );

  return crashDumpEnabled;
}

auto
RegistryManager::CreateRegistryEntry(
  const HKEY hKey,
  const std::wstring& subKey,
  const std::wstring& valueName,
  const DWORD regType,
  const std::vector<std::wstring>& allowedValues) -> Registry::EntryConfig
{
  Registry::EntryConfig config;
  config.hKey = hKey;
  config.subKey = subKey;
  config.valueName = valueName;
  config.regType = regType;
  config.allowedValues = allowedValues;
  return config;
}

} // namespace Reg
