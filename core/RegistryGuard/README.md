# RegistryGuard

`RegistryGuard` is a C++ utility for monitoring Windows registry entries. It periodically checks specified registry
values and triggers a callback if a monitored value deviates from its allowed values. This tool can be helpful for
maintaining configurations or tracking unauthorized changes in registry settings.

## Features

- **Configurable Monitoring**: Define registry entries and allowed values to monitor.
- **Callback Functionality**: Customizable callback when a value changes from its allowed set.
- **Thread-based Monitoring**: Runs in a background thread with adjustable intervals.

## Installation

Include `RegistryGuard.hpp` and `RegistryGuard.cpp` in your project, ensuring access to Windows API and `Logger.hpp` for
logging.

## Usage Example

```cpp
#include "RegistryGuard.hpp"
#include <iostream>

// Custom callback function for registry changes
void OnRegistryChange(const std::wstring& valueName,
                      const std::wstring& currentValue,
                      const Registry::EntryConfig& config) {
    std::wcout << L"Registry value changed for " << config.valueName 
               << L". New value: " << currentValue << std::endl;
}

int main() {
    // Define a registry entry to monitor
    Registry::EntryConfig entry = Registry::RegistryGuard::CreateRegistryEntry(
        HKEY_CURRENT_USER,
        L"Software\\MyApp",
        L"MyValue",
        REG_SZ,
        {L"AllowedValue1", L"AllowedValue2"}
    );

    // Create a RegistryGuard instance
    Registry::RegistryGuard regGuard(5000, OnRegistryChange);
    
    // Add the entry to the guard
    regGuard.AddEntry(entry);

    // Start monitoring
    regGuard.Start();
    
    // Monitor for a period, then stop (for demonstration purposes)
    Sleep(15000); // Sleep for 15 seconds
    regGuard.Stop();

    return 0;
}
```

### Explanation

1. **Define Entry**: Use `CreateRegistryEntry` to configure an entry with a key, subkey, value name, data type, and
   allowed values.
2. **Set Up Callback**: The `OnRegistryChange` function will be called if the registry value differs from the allowed
   list.
3. **Monitor Entries**: Call `Start()` to begin monitoring and `Stop()` to end it.

### License

This project is licensed under the MIT License. See the [LICENSE](LICENSE-MIT) file for details.