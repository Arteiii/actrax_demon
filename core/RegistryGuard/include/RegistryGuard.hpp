#ifndef REGISTRYGUARD_HPP
#define REGISTRYGUARD_HPP

#include <RegistryError.hpp>
#include <atomic>
#include <functional>
#include <string>
#include <vector>
#include <windows.h>

namespace Registry {

/**
 * @brief Structure to configure registry entry details for monitoring.
 *
 * Defines the registry key, subkey, value name, data type, and the allowed
 * values for validation.
 */
struct EntryConfig
{
  HKEY hKey;              ///< Handle to the registry key.
  std::wstring subKey;    ///< Name of the subkey in the registry.
  std::wstring valueName; ///< Name of the registry value to monitor.
  DWORD regType;          ///< Registry data type (e.g., REG_SZ, REG_DWORD).
  std::vector<std::wstring>
    allowedValues; ///< List of allowed values for validation.
};

/**
 * @brief Class that monitors registry entries and triggers a callback on value
 * change.
 *
 * RegistryGuard can be used to watch specific registry entries at a specified
 * interval. If a value changes from the allowed set, a callback function is
 * triggered with details.
 */
class RegistryGuard
{
public:
  /// Callback type for registry value changes.
  using callback = std::function<
    void(const std::wstring&, const std::wstring&, const EntryConfig&)>;

  /**
   * @brief Creates an EntryConfig structure to define a registry entry
   * configuration.
   *
   * @param hKey         Handle to the registry key.
   * @param subKey       Subkey path under the provided registry key.
   * @param valueName    Name of the value to monitor.
   * @param regType      Type of the registry value (e.g., REG_SZ, REG_DWORD).
   * @param allowedValues Allowed values for the specified registry value.
   * @return EntryConfig Structure containing registry entry configuration.
   */
  static auto
  CreateRegistryEntry(HKEY hKey,
                      const std::wstring& subKey,
                      const std::wstring& valueName,
                      DWORD regType,
                      const std::vector<std::wstring>& allowedValues)
    -> EntryConfig;

  /**
   * @brief Constructs a RegistryGuard instance.
   *
   * @param checkIntervalMs Interval in milliseconds for checking registry
   * values.
   * @param clb            Callback function to invoke on registry value change.
   */
  RegistryGuard(int checkIntervalMs, callback clb);

  /// Destructor to ensure monitoring stops gracefully.
  ~
  RegistryGuard();

  /**
   * @brief Adds an entry to the registry monitor list.
   *
   * @param config Configuration of the registry entry to add.
   */
  auto
  AddEntry(const EntryConfig& config) -> void;

  /// Starts the monitoring thread for registry entries.
  auto
  Start() -> void;

  /// Stops the monitoring thread for registry entries.
  auto
  Stop() -> void;

private:
  /// Thread function to monitor registry entries continuously.
  static auto WINAPI
  Monitor(LPVOID lpParam) -> DWORD;

  /// Internal function to check the status of each monitored registry entry.
  auto
  CheckEntries() -> void;

  static constexpr size_t BufferSize =
    256U;                     ///< Buffer size for registry value data.
  int checkIntervalMs_;       ///< Interval in milliseconds for checks.
  callback callback_;         ///< Callback function for registry value changes.
  std::atomic<bool> running_; ///< Flag to indicate if monitoring is active.
  HANDLE monitorThread_{ nullptr };       ///< Handle to the monitoring thread.
  std::vector<EntryConfig> entryConfigs_; ///< List of entries to monitor.
};

} // namespace Registry

#endif // REGISTRYGUARD_HPP
