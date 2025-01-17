# RegistryHelper

Original Repository:
https://github.com/Arteiii/RegistryHelper

This project provides a `RegistryHelper` class for accessing the Windows Registry using Modern C++

## Project Structure

### RegistryHelper

RegistryHelper is a convenient utility encapsulated in a single header file named ``RegistryHelper.hpp``. This header
file provides common operations for interacting with the Windows registry.

### RegistryInterface

RegistryInterface is a work-in-progress module that already contains predefined operations such as:

- GetAutoRebootSetting
- EnumerateServices
  ...

The RegistryInterface is continuously evolving, and additional functionalities will be added over time.
The purpose of this module is to encapsulate and provide an organized set of registry-related operations for ease of
use.

## Usage

### Using as a Library

Include the "RegistryHelper.hpp" header file in your project.  
Link your project with the compiled "RegistryHelper.lib" library.

[.lib files as linker input](https://learn.microsoft.com/en-us/cpp/build/reference/dot-lib-files-as-linker-input?view=msvc-170)

[Walkthrough: Create and use a static library](https://learn.microsoft.com/en-us/cpp/build/walkthrough-creating-and-using-a-static-library-cpp?view=msvc-170)

## Using Directly

Include the "RegistryHelper.h" header file in your source files.

### Example Usage

```cpp
#include "RegistryHelper.h"

// Create an instance of RegistryHelper
RegistryHelper registryHelper;

// Example 1: Reading DWORD value (REG_DWORD)
std::wstring valueNameDword = L"BootDriverFlags";
DWORD dwordValue = registryHelper.RegGetDword(hKey, subKey, valueNameDword);

// Example 2: Reading String value (REG_SZ)
std::wstring valueNameString = L"CurrentUser";
std::wstring stringValue =
  registryHelper.RegGetString(hKey, subKey, valueNameString);

// Example 3: Reading Multi-String value (REG_MULTI_SZ)
std::wstring valueNameMultiString = L"PreshutdownOrder";
std::vector<std::wstring> multiStringValue =
  registryHelper.RegGetMultiString(hKey, subKey, valueNameMultiString);

// Example 4: Enumerate sub-keys
std::vector<std::pair<std::wstring, DWORD>> subKeys =
  registryHelper.RegEnumSubKeys(hKey, subKey);

// Example 5: Enumerate values
std::vector<std::pair<std::wstring, DWORD>> values =
  registryHelper.RegEnumValues(hKey, subKey);


// Set DWORD value under a non-existing key (will create the key first)
registryHelper.RegSetDword(HKEY_LOCAL_MACHINE,
                           L"SOFTWARE\\NewApp",
                           L"NewDWORD",
                           123);
```

Adjust the registry keys, subkeys, and value names based on your specific use case.

## Credits

This project is inspired by the Microsoft Dev Blog post "Use Modern C++ to Access the Windows Registry" by Giovanni
Dicanio. The original blog post can be
found [here](https://learn.microsoft.com/en-us/archive/msdn-magazine/2017/may/c-use-modern-c-to-access-the-windows-registry).

## License

This project is provided under the MIT License. See the [LICENSE](LICENSE-MIT) file for details.