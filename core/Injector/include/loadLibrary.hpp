//
// Created by arteii on 11/26/24.
//

#ifndef LOADLIBRARY_HPP
#define LOADLIBRARY_HPP

#include <string>
#include <windows.h>

namespace Injector {

/**
 * A utility class for injecting a DLL into a target process using LoadLibrary.
 */
class LoadLibraryRemoteThread
{
public:
  LoadLibraryRemoteThread() = default;
  ~LoadLibraryRemoteThread() = default;

  /**
   * Injects a DLL into the specified process using the LoadLibrary function.
   * This method performs the following steps:
   * 1. Opens the target process with the necessary access rights.
   * 2. Allocates memory in the target process and writes the DLL path into it.
   * 3. Retrieves the address of LoadLibraryA from kernel32.dll.
   * 4. Creates a remote thread in the target process to call LoadLibraryA.
   * 5. Cleans up all allocated resources and handles after execution.
   *
   * @param processId The ID of the target process.
   * @param dllPath The full path to the DLL to inject.
   * @return True if the injection is successful, false otherwise.
   *
   * @throws std::runtime_error If critical errors occur during the injection
   * process.
   */
  [[nodiscard]] static auto
  InjectDll(DWORD processId, const std::string& dllPath) -> bool;

private:
  /**
   * Opens the target process with the required access rights.
   * This function ensures the necessary permissions are granted to perform
   * remote thread creation and memory operations in the target process.
   *
   * @param processId The ID of the process to open.
   * @return A valid handle to the process.
   *
   * @throws std::runtime_error If the process cannot be opened.
   */
  static auto
  OpenTargetProcess(DWORD processId) -> HANDLE;

  /**
   * Allocates memory in the target process and writes the DLL path into it.
   * This function allocates memory in the target process using `VirtualAllocEx`
   * and writes the DLL path string into the allocated memory using
   * `WriteProcessMemory`.
   *
   * @param processHandle A valid handle to the target process.
   * @param dllPath The full path to the DLL.
   * @return The address of the allocated memory in the target process.
   *
   * @throws std::runtime_error If memory allocation or writing fails.
   */
  static auto
  AllocateDllPath(HANDLE processHandle, const std::string& dllPath) -> LPVOID;
};

} // namespace Injector

#endif // LOADLIBRARY_HPP
