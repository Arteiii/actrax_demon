#include "pch.h"

#include "registryError.hpp"
#include "registryHelper.hpp"

namespace Registry {
// Read a DWORD value from the registry
auto
// ReSharper disable once CppParameterMayBeConst
Helper::RegGetDword(HKEY hKey,
                    const std::wstring& subKey,
                    const std::wstring& value) -> DWORD
{
  DWORD data{};
  auto dataSize = static_cast<DWORD>(sizeof(data));
  LSTATUS errorCode{ ERROR_SUCCESS };

  // Assign the return value to an LSTATUS type (signed 32-bit)
  errorCode = ::RegGetValueW(hKey,
                             reinterpret_cast<LPCWSTR>(subKey.c_str()),
                             reinterpret_cast<LPCWSTR>(value.c_str()),
                             static_cast<DWORD>(RRF_RT_REG_DWORD),
                             nullptr,
                             &data,
                             &dataSize);

  if (errorCode != ERROR_SUCCESS) {
    // Throw a registry Error exception if an error occurs
    throw Error("Cannot read DWORD from registry.", errorCode);
  }

  return data;
}

// Read a string value from the registry
auto
// ReSharper disable once CppParameterMayBeConst
Helper::RegGetString(HKEY hKey,
                     const std::wstring& subKey,
                     const std::wstring& value) -> std::wstring
{
  LSTATUS errorCode{ ERROR_SUCCESS };

  // Retrieve the size of the string value data
  DWORD dataSize{};
  errorCode = ::RegGetValueW(hKey,
                             reinterpret_cast<LPCWSTR>(subKey.c_str()),
                             reinterpret_cast<LPCWSTR>(value.c_str()),
                             static_cast<DWORD>(RRF_RT_REG_SZ),
                             nullptr,
                             nullptr,
                             &dataSize);

  // Check if the value information retrieval was successful
  if (errorCode != ERROR_SUCCESS) {
    throw Error("Cannot read string from registry", errorCode);
  }

  // Use smart pointers for automatic memory management
  std::vector<wchar_t> data(dataSize / sizeof(wchar_t));

  // Retrieve the actual string value data
  errorCode = ::RegGetValueW(hKey,
                             reinterpret_cast<LPCWSTR>(subKey.c_str()),
                             reinterpret_cast<LPCWSTR>(value.c_str()),
                             static_cast<DWORD>(RRF_RT_REG_SZ),
                             nullptr,
                             data.data(),
                             &dataSize);

  // Check if the value data retrieval was successful
  if (errorCode != ERROR_SUCCESS) {
    throw Error("Cannot read string from registry", errorCode);
  }

  // Resize the vector based on the actual size of the string
  data.resize((dataSize / sizeof(wchar_t)) - 1U);

  // convert the wchar_t vector to a wstring
  return { data.begin(), data.end() };
}

// Read a multi-string value from the registry
auto
// ReSharper disable once CppParameterMayBeConst
Helper::RegGetMultiString(HKEY hKey,
                          const std::wstring& subKey,
                          const std::wstring& value)
  -> std::vector<std::wstring>
{
  LSTATUS errorCode{ ERROR_SUCCESS };

  // Retrieve the size of the multi-string value data
  DWORD dataSize{};
  errorCode = ::RegGetValueW(hKey,
                             reinterpret_cast<LPCWSTR>(subKey.c_str()),
                             reinterpret_cast<LPCWSTR>(value.c_str()),
                             RRF_RT_REG_MULTI_SZ,
                             nullptr,
                             nullptr,
                             &dataSize);

  // Check if the value information retrieval was successful
  if (errorCode != ERROR_SUCCESS) {
    throw Error("Cannot read multi-string from registry", errorCode);
  }

  std::vector<wchar_t> data(dataSize / sizeof(wchar_t));

  // Retrieve the actual multi-string value data
  errorCode = ::RegGetValueW(hKey,
                             reinterpret_cast<LPCWSTR>(subKey.c_str()),
                             reinterpret_cast<LPCWSTR>(value.c_str()),
                             RRF_RT_REG_MULTI_SZ,
                             nullptr,
                             data.data(),
                             &dataSize);

  // Check if the value data retrieval was successful
  if (errorCode != ERROR_SUCCESS) {
    throw Error("Cannot read multi-string from registry", errorCode);
  }

  // Resize the vector based on the actual size of the multi-string
  data.resize(dataSize / sizeof(wchar_t));

  // Parse the double-NUL-terminated string into a vector of wstrings
  std::vector<std::wstring> result;
  const wchar_t* currStringPtr = data.data();

  while (*currStringPtr != L'\0') {
    const size_t currStringLength = wcslen(currStringPtr);

    // Emplace back the current string pointer and its length
    (void)result.emplace_back(currStringPtr, currStringLength);

    // Increment the pointer correctly without changing signedness
    currStringPtr += currStringLength + 1U; // Move to the next string
  }

  // Return the vector containing individual strings from the multi-string
  return result;
}

auto
// ReSharper disable once CppParameterMayBeConst
Helper::RegEnumSubKeys(HKEY hKey, const std::wstring& subKey)
  -> std::vector<std::pair<std::wstring, DWORD>>
{
  HKEY keyHandle;
  const LSTATUS openResult =
    RegOpenKeyExW(hKey,
                  reinterpret_cast<LPCWSTR>(subKey.c_str()),
                  0U,
                  // ReSharper disable once CppRedundantCastExpression
                  static_cast<DWORD>(KEY_READ),
                  &keyHandle);

  // Initialize variables
  std::vector<std::pair<std::wstring, DWORD>>
    subKeys;                    // Vector to store results
  long retCode = ERROR_SUCCESS; // Initialize return code
  DWORD subKeyCount{};          // Number of subkeys under the key
  DWORD maxSubKeyNameLen{};     // Maximum length of a subkey name

  // Check if the key was opened successfully
  if (openResult != ERROR_SUCCESS) {
    throw Error("Failed to open registry key.",
                openResult); // Throw error if unable to open key
  }

  // Retrieve information about the specified registry key
  retCode = ::RegQueryInfoKeyW(keyHandle,
                               nullptr, // No user-defined class
                               nullptr, // No user-defined class size
                               nullptr, // Reserved
                               &subKeyCount,
                               &maxSubKeyNameLen,
                               nullptr, // No subkey class length
                               nullptr, // No value count
                               nullptr, // No value max length
                               nullptr, // No security descriptor
                               nullptr, // No last write time
                               nullptr);

  // If the key information retrieval fails, set the return code
  if (retCode != ERROR_SUCCESS) {
    (void)RegCloseKey(keyHandle); // Clean up before exiting
    throw Error{
      "RegQueryInfoKey failed while preparing for value enumeration.", retCode
    };
  }

  // Allocate a buffer for storing subkey names
  maxSubKeyNameLen++; // Increase length for null terminator

  // Using unique_ptr fordynamic array allocation
  // NOLINTNEXTLINE(clang-tidy, *-avoid-c-arrays):
  const auto nameBuffer = std::make_unique<wchar_t[]>(maxSubKeyNameLen);

  // Enumerate subkeys under the registry key
  for (auto index = 0U; index < subKeyCount; index++) {
    DWORD subKeyNameLen = maxSubKeyNameLen;

    // Retrieve information about the specified subkey
    retCode = ::RegEnumKeyExW(keyHandle,
                              index,
                              reinterpret_cast<LPWSTR>(nameBuffer.get()),
                              &subKeyNameLen,
                              nullptr, // Reserved
                              nullptr, // No class information
                              nullptr, // No class size
                              nullptr  // No last write time
    );

    // If the subkey information retrieval fails, clean up and throw
    if (retCode != ERROR_SUCCESS) {
      (void)RegCloseKey(keyHandle); // Clean up before throwing
      throw Error{ "Cannot get subkey info from the registry", retCode };
    }

    // Add the subkey name and type to the vector
    (void)subKeys.emplace_back(std::wstring{ nameBuffer.get(), subKeyNameLen },
                               0);
  }

  // Close the key handle before the single exit point
  (void)RegCloseKey(keyHandle);

  // Return the vector containing subkey names and types
  return subKeys;
}

auto
// ReSharper disable once CppParameterMayBeConst
Helper::RegEnumValues(HKEY hKey, const std::wstring& subKey)
  -> std::vector<std::pair<std::wstring, DWORD>>
{
  HKEY keyHandle;

  // Open the specified key (root key or subkey)
  const LSTATUS openResult =
    RegOpenKeyExW(hKey,
                  reinterpret_cast<LPCWSTR>(subKey.c_str()),
                  0U,
                  // ReSharper disable once CppRedundantCastExpression
                  static_cast<DWORD>(KEY_READ),
                  &keyHandle);

  if (openResult != ERROR_SUCCESS) {
    // Handle error by throwing an exception
    throw Error("Failed to open registry key.", openResult);
  }

  DWORD valueCount{};
  DWORD maxValueNameLen{};
  long retCode = ::RegQueryInfoKeyW(keyHandle,
                                    nullptr, // no user-defined class
                                    nullptr, // no user-defined class size
                                    nullptr, // reserved
                                    nullptr, // no subkey count
                                    nullptr, // no subkey max length
                                    nullptr, // no subkey class length
                                    &valueCount,
                                    &maxValueNameLen,
                                    nullptr, // no max value length
                                    nullptr, // no security descriptor
                                    nullptr  // no last write time
  );

  if (retCode != ERROR_SUCCESS) {
    // Handle error, close the key handle and throw an exception
    (void)RegCloseKey(keyHandle);
    throw Error{
      "RegQueryInfoKey failed while preparing for value enumeration.", retCode
    };
  }

  maxValueNameLen++;

  // NOLINTNEXTLINE(clang-tidy, *-avoid-c-arrays):
  const auto nameBuffer = std::make_unique<wchar_t[]>(maxValueNameLen);

  std::vector<std::pair<std::wstring, DWORD>> valueInfo;

  // Enumerate the values
  for (auto index = 0U; index < valueCount; index++) {
    // Get the name and type
    DWORD valueNameLen = maxValueNameLen;
    DWORD valueType{};
    retCode = ::RegEnumValueW(keyHandle,
                              index,
                              reinterpret_cast<LPWSTR>(nameBuffer.get()),
                              &valueNameLen,
                              nullptr, // reserved
                              &valueType,
                              nullptr, // no data
                              nullptr  // no data size
    );

    if (retCode != ERROR_SUCCESS) {
      (void)RegCloseKey(keyHandle);
      throw Error{ "Cannot enumerate values: RegEnumValue failed.", retCode };
    }

    (void)valueInfo.emplace_back(std::wstring{ nameBuffer.get(), valueNameLen },
                                 valueType);
  }

  (void)RegCloseKey(keyHandle);

  return valueInfo;
}

auto
// ReSharper disable once CppParameterMayBeConst
Helper::RegSetDword(HKEY hKey,
                    const std::wstring& subKey,
                    const std::wstring& value,
                    const DWORD data) -> void
{
  LSTATUS errorCode{ ERROR_SUCCESS };

  errorCode =
    ::RegSetKeyValueW(hKey,
                      reinterpret_cast<LPCWSTR>(subKey.c_str()),
                      reinterpret_cast<LPCWSTR>(value.c_str()),
                      // ReSharper disable once CppRedundantCastExpression
                      static_cast<DWORD>(REG_DWORD),
                      &data,
                      static_cast<DWORD>(sizeof(data)));
  if (errorCode != ERROR_SUCCESS) {
    throw Error("Cannot set DWORD value in registry.", errorCode);
  }
}

auto
// ReSharper disable once CppParameterMayBeConst
Helper::RegSetString(HKEY hKey,
                     const std::wstring& subKey,
                     const std::wstring& value,
                     const std::wstring& data) -> void
{
  LSTATUS errorCode{ ERROR_SUCCESS };

  // Calculate the size with size_t to avoid narrowing
  const size_t lengthWithNullTerminator =
    data.length() + 1U; // +1 for the null terminator
  const size_t dataSize =
    lengthWithNullTerminator * sizeof(wchar_t); // Calculate total size

  // Check if dataSize exceeds the maximum value for DWORD
  if (dataSize > static_cast<size_t>(MAXDWORD)) {
    throw std::overflow_error("Calculated data size exceeds DWORD limits.");
  }

  // Cast to DWORD safely
  const auto finalDataSize = static_cast<DWORD>(dataSize);

  // Set the string value in the registry
  errorCode =
    ::RegSetKeyValueW(hKey,
                      reinterpret_cast<LPCWSTR>(subKey.c_str()),
                      reinterpret_cast<LPCWSTR>(value.c_str()),
                      // ReSharper disable once CppRedundantCastExpression
                      static_cast<DWORD>(REG_SZ),
                      data.c_str(),
                      finalDataSize);

  if (errorCode != ERROR_SUCCESS) {
    throw Error("Cannot set string value in registry.", errorCode);
  }
}

auto
// ReSharper disable once CppParameterMayBeConst
Helper::RegSetMultiString(HKEY hKey,
                          const std::wstring& subKey,
                          const std::wstring& value,
                          const std::vector<std::wstring>& data) -> void
{
  LSTATUS errorCode{ ERROR_SUCCESS };

  // Concatenate the strings and add an extra null character at the end
  std::wstring multiString;
  for (const auto& str : data) {
    multiString += str;
    multiString.push_back(L'\0');
  }
  multiString.push_back(L'\0'); // Extra null character at the end

  errorCode = ::RegSetKeyValueW(

    hKey,
    reinterpret_cast<LPCWSTR>(subKey.c_str()),
    reinterpret_cast<LPCWSTR>(value.c_str()),
    REG_MULTI_SZ,
    multiString.c_str(),
    static_cast<DWORD>(multiString.length() * sizeof(wchar_t)));
  if (errorCode != ERROR_SUCCESS) {
    throw Error("Cannot set multi-string value in registry.", errorCode);
  }
}
} // namespace Registry
