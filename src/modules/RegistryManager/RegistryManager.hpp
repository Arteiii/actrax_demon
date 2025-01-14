#ifndef REGISTRY_MANAGER_HPP
#define REGISTRY_MANAGER_HPP

#include "RegistryGuard.hpp"

namespace Reg {

class RegistryManager
{
public:
  RegistryManager();

  explicit
  RegistryManager(const std::vector<Registry::EntryConfig>& customEntries);

  ~
  RegistryManager();

  auto
  Initialize() -> void;
  auto
  Cleanup() -> void;

private:
  Registry::RegistryGuard regGuard_;
  static constexpr int DefaultDelayMs = 1000;

  static auto
  OnRegistryChange(const std::wstring& valueName,
                   const std::wstring& currentValue,
                   const Registry::EntryConfig& entry) -> void;

  auto
  AddRegistryEntry(const Registry::EntryConfig& config) -> void;

  static auto
  CrashDumpEnabled() -> Registry::EntryConfig;

  static auto
  CreateRegistryEntry(HKEY hKey,
                      const std::wstring& subKey,
                      const std::wstring& valueName,
                      DWORD regType,
                      const std::vector<std::wstring>& allowedValues)
    -> Registry::EntryConfig;
};

} // namespace Reg

#endif // REGISTRY_MANAGER_HPP
