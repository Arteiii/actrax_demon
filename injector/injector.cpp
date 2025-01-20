
#include "pch.h"

#include "injector.hpp"

namespace Injector {

auto
LoadLibraryRemoteThread::OpenTargetProcess(const DWORD processId) -> HANDLE
{
  const HANDLE processHandle =
    OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE,
                FALSE,
                processId);
  if (processHandle == nullptr) {
    LOG_ERROR("Failed to open target process. Error: " + GetLastError());
    throw std::runtime_error("Failed to open target process.");
  }
  return processHandle;
}

auto
LoadLibraryRemoteThread::AllocateDllPath(const HANDLE processHandle,
                                         const std::string& dllPath) -> LPVOID
{
  const size_t pathSize = (dllPath.size() + 1) * sizeof(char);
  const LPVOID remoteMemory = VirtualAllocEx(
    processHandle, nullptr, pathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

  if (remoteMemory == nullptr) {
    LOG_ERROR("Failed to allocate memory in target process. Error: " +
              GetLastError());
    throw std::runtime_error("Failed to allocate memory in target process.");
  }

  if (WriteProcessMemory(
        processHandle, remoteMemory, dllPath.c_str(), pathSize, nullptr) == 0) {
    LOG_ERROR("Failed to write DLL path to target process memory. Error: " +
              GetLastError());

    if (VirtualFreeEx(processHandle, remoteMemory, 0, MEM_RELEASE) == 0) {
      LOG_ERROR("Failed to free allocated memory in target process. Error: " +
                GetLastError());
    }

    throw std::runtime_error(
      "Failed to write DLL path to target process memory.");
  }

  return remoteMemory;
}

auto
LoadLibraryRemoteThread::InjectDll(const DWORD processId,
                                   const std::string& dllPath) -> bool
{
  try {
    LOG_TRACE("Starting DLL injection process.");

    if (const DWORD fileAttributes = GetFileAttributesA(dllPath.c_str());
        fileAttributes == INVALID_FILE_ATTRIBUTES ||
        (fileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      LOG_ERROR("DLL path does not exist or is inaccessible: " + dllPath);
      return false;
    }
    LOG_TRACE("DLL path is valid and accessible: " + dllPath);

    const HANDLE processHandle = OpenTargetProcess(processId);
    if (processHandle == nullptr) {
      LOG_ERROR("Failed to open target process. Error: " +
                std::to_string(GetLastError()));
      return false;
    }
    LOG_TRACE("Opened target process handle.");

    const LPVOID remoteMemory = AllocateDllPath(processHandle, dllPath);
    if (remoteMemory == nullptr) {
      LOG_ERROR("Failed to allocate memory in the target process.");
      CloseHandle(processHandle);
      return false;
    }

    LOG_TRACE("Allocated memory in the target process for the DLL path.");

    const HMODULE kernel32Handle = GetModuleHandleA("kernel32.dll");
    LOG_TRACE("Obtained handle to kernel32.dll.");

    const FARPROC loadLibraryAddress =
      GetProcAddress(kernel32Handle, "LoadLibraryA");
    if (loadLibraryAddress == nullptr) {
      LOG_ERROR("Failed to get the address of LoadLibraryA. Error: " +
                std::to_string(GetLastError()));
      VirtualFreeEx(processHandle, remoteMemory, 0, MEM_RELEASE);
      CloseHandle(processHandle);
      return false;
    }
    LOG_TRACE("Retrieved address of LoadLibraryA.");

    const HANDLE remoteThread = CreateRemoteThread(
      processHandle,
      nullptr,
      0,
      reinterpret_cast<LPTHREAD_START_ROUTINE>(loadLibraryAddress),
      remoteMemory,
      0,
      nullptr);

    if (remoteThread == nullptr) {
      const DWORD errorCode = GetLastError();
      LOG_ERROR("Failed to create remote thread. Error: " +
                std::to_string(errorCode));
      VirtualFreeEx(processHandle, remoteMemory, 0, MEM_RELEASE);
      CloseHandle(processHandle);
      return false;
    }

    LOG_TRACE("Created remote thread.");

    WaitForSingleObject(remoteThread, INFINITE);
    LOG_TRACE("Remote thread execution completed.");

    if (VirtualFreeEx(processHandle, remoteMemory, 0, MEM_RELEASE) == 0) {
      LOG_ERROR(
        "Failed to free allocated memory in the target process. Error: " +
        std::to_string(GetLastError()));
      CloseHandle(remoteThread);
      CloseHandle(processHandle);
      return false;
    }
    LOG_TRACE("Freed allocated memory in the target process.");

    if (CloseHandle(remoteThread) == 0) {
      LOG_ERROR("Failed to close remote thread handle. Error: " +
                std::to_string(GetLastError()));
      CloseHandle(processHandle);
      return false;
    }
    LOG_TRACE("Closed remote thread handle.");

    if (CloseHandle(processHandle) == 0) {
      LOG_ERROR("Failed to close process handle. Error: " +
                std::to_string(GetLastError()));
      return false;
    }
    LOG_TRACE("Closed process handle.");

    LOG_TRACE("DLL injection process completed successfully.");
    return true;
  } catch (const std::exception& ex) {
    LOG_ERROR("Error during DLL injection: " + std::string(ex.what()));
    return false;
  }
}
