//
// Created by arteii on 10/23/24.
//

#ifndef REGISTRYHELPER_HPP
#define REGISTRYHELPER_HPP

#include <string>
#include <vector>

#define UMDF_USING_NTSTATUS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Registry {

class Helper
{
public:
  Helper() = default;

  /**
   * Reads a DWORD value from the registry.
   *
   * @param hKey Handle to an open registry key.
   * @param subKey Subkey name under the open registry key.
   * @param value Value name of the DWORD to read.
   * @return The DWORD value read from the registry.
   * @throws Error if the value cannot be read.
   */
  static auto
  RegGetDword(HKEY hKey, const std::wstring& subKey, const std::wstring& value)
    -> DWORD;

  /**
   * Reads a string value from the registry.
   *
   * @param hKey Handle to an open registry key.
   * @param subKey Subkey name under the open registry key.
   * @param value Value name of the string to read.
   * @return The string value read from the registry.
   * @throws Error if the value cannot be read.
   */
  static auto
  RegGetString(HKEY hKey, const std::wstring& subKey, const std::wstring& value)
    -> std::wstring;

  /**
   * Reads a multi-string value from the registry.
   *
   * @param hKey Handle to an open registry key.
   * @param subKey Subkey name under the open registry key.
   * @param value Value name of the multi-string to read.
   * @return A vector of strings read from the registry.
   * @throws Error if the value cannot be read.
   */
  static auto
  RegGetMultiString(HKEY hKey,
                    const std::wstring& subKey,
                    const std::wstring& value) -> std::vector<std::wstring>;

  /**
   * Enumerates subkeys under the specified registry key.
   *
   * @param hKey Handle to an open registry key.
   * @param subKey Subkey name under the open registry key.
   * @return A vector of pairs containing subkey names and their types.
   * @throws Error if the subkeys cannot be enumerated.
   */
  static auto
  RegEnumSubKeys(HKEY hKey, const std::wstring& subKey)
    -> std::vector<std::pair<std::wstring, DWORD>>;

  /**
   * Enumerates values under the specified registry key.
   *
   * @param hKey Handle to an open registry key.
   * @param subKey Subkey name under the open registry key.
   * @return A vector of pairs containing value names and their types.
   * @throws Error if the values cannot be enumerated.
   */
  static auto
  RegEnumValues(HKEY hKey, const std::wstring& subKey)
    -> std::vector<std::pair<std::wstring, DWORD>>;

  /**
   * Sets a DWORD value in the registry.
   *
   * @param hKey Handle to an open registry key.
   * @param subKey Subkey name under the open registry key.
   * @param value Value name to set the DWORD.
   * @param data The DWORD data to write.
   * @throws Error if the value cannot be set.
   */
  static auto
  RegSetDword(HKEY hKey,
              const std::wstring& subKey,
              const std::wstring& value,
              DWORD data) -> void;

  /**
   * Sets a string value in the registry.
   *
   * @param hKey Handle to an open registry key.
   * @param subKey Subkey name under the open registry key.
   * @param value Value name to set the string.
   * @param data The string data to write.
   * @throws Error if the value cannot be set.
   */
  static auto
  RegSetString(HKEY hKey,
               const std::wstring& subKey,
               const std::wstring& value,
               const std::wstring& data) -> void;

  /**
   * Sets a multi-string value in the registry.
   *
   * @param hKey Handle to an open registry key.
   * @param subKey Subkey name under the open registry key.
   * @param value Value name to set the multi-string.
   * @param data A vector of strings to write.
   * @throws Error if the value cannot be set.
   */
  static auto
  RegSetMultiString(HKEY hKey,
                    const std::wstring& subKey,
                    const std::wstring& value,
                    const std::vector<std::wstring>& data) -> void;
};
} // namespace Registry

#endif // REGISTRYHELPER_HPP
