//
// Created by arteii on 11/2/24.
//

#include "ApplicationController.hpp"
#include <Logger.hpp>

#include <processthreadsapi.h>
#include <windows.h>

#include <stdexcept>
#include <versionhelpers.h>

namespace ApplicationController {

auto
SetDynamicCodePolicy() -> void
{
  PROCESS_MITIGATION_DYNAMIC_CODE_POLICY dynamicCodePolicy = {};
  dynamicCodePolicy.ProhibitDynamicCode = 1; // prohibit dynamic code generation
  dynamicCodePolicy.AllowThreadOptOut = 0;   // disallow threads to opt out
  dynamicCodePolicy.AllowRemoteDowngrade = 0; // disallow remote downgrade

  if (IsWindowsVersionOrGreater(
        10, 0, 16299)) { // win 10 version 1709 or later

    if (SetProcessMitigationPolicy(ProcessDynamicCodePolicy,
                                   &dynamicCodePolicy,
                                   sizeof(dynamicCodePolicy)) == 0) {

      switch (const DWORD errorCode = GetLastError()) {
        case ERROR_ACCESS_DENIED:
          throw std::runtime_error("Failed to set dynamic code policy: Access "
                                   "denied. Ensure sufficient privileges.");
        case ERROR_INVALID_PARAMETER:
          throw std::runtime_error("Failed to set dynamic code policy: Invalid "
                                   "parameter. Verify the policy settings.");
        case ERROR_NOT_SUPPORTED:
          throw std::runtime_error("Failed to set dynamic code policy: "
                                   "Operation not supported on this system.");
        default:
          throw std::runtime_error(
            "Failed to set dynamic code policy. Unknown error code: " +
            std::to_string(errorCode));
      }
    }
  } else {
    throw std::runtime_error(
      "Dynamic code policy not supported on this version of Windows.");
  }
}

auto
SetImageLoadPolicy() -> void
{
  PROCESS_MITIGATION_IMAGE_LOAD_POLICY imageLoadPolicy = {};
  imageLoadPolicy.NoRemoteImages =
    1; // block loading DLLs from remote locations
  imageLoadPolicy.NoLowMandatoryLabelImages =
    1; // block DLLs with low integrity levels
  imageLoadPolicy.PreferSystem32Images = 1; // prefer loading DLLs from System32

  if (SetProcessMitigationPolicy(ProcessImageLoadPolicy,
                                 &imageLoadPolicy,
                                 sizeof(imageLoadPolicy)) == 0) {
    throw std::runtime_error("Failed to set image load policy. Error: " +
                             std::to_string(GetLastError()));
  }
}

auto
SetSignaturePolicy() -> void
{
  PROCESS_MITIGATION_BINARY_SIGNATURE_POLICY signaturePolicy = {};
  signaturePolicy.MicrosoftSignedOnly = 1; // allow only ms signed binaries

  if (SetProcessMitigationPolicy(ProcessSignaturePolicy,
                                 &signaturePolicy,
                                 sizeof(signaturePolicy)) == 0) {
    throw std::runtime_error("Failed to set signature policy. Error: " +
                             std::to_string(GetLastError()));
  }
}

auto
SetExtensionPointPolicy() -> void
{
  PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY extensionPolicy = {};
  extensionPolicy.DisableExtensionPoints = 1; // disable extension points

  if (SetProcessMitigationPolicy(ProcessExtensionPointDisablePolicy,
                                 &extensionPolicy,
                                 sizeof(extensionPolicy)) == 0) {
    throw std::runtime_error("Failed to disable extension points. Error: " +
                             std::to_string(GetLastError()));
  }
}

auto
SetChildProcessPolicy() -> void
{
  PROCESS_MITIGATION_CHILD_PROCESS_POLICY childPolicy = {};
  childPolicy.NoChildProcessCreation = 1; // block child process creation

  if (SetProcessMitigationPolicy(
        ProcessChildProcessPolicy, &childPolicy, sizeof(childPolicy)) == 0) {
    throw std::runtime_error("Failed to set child process policy. Error: " +
                             std::to_string(GetLastError()));
  }
}

auto
SetHandleCheckPolicy() -> void
{
  PROCESS_MITIGATION_STRICT_HANDLE_CHECK_POLICY handlePolicy = {};
  handlePolicy.RaiseExceptionOnInvalidHandleReference =
    1; // raise exception on invalid handles
  handlePolicy.HandleExceptionsPermanentlyEnabled =
    1; // permanently enable handle exceptions

  if (SetProcessMitigationPolicy(ProcessStrictHandleCheckPolicy,
                                 &handlePolicy,
                                 sizeof(handlePolicy)) == 0) {
    throw std::runtime_error("Failed to set handle check policy. Error: " +
                             std::to_string(GetLastError()));
  }
}

auto
CheckCfg() -> void
{
  PROCESS_MITIGATION_CONTROL_FLOW_GUARD_POLICY cfgPolicy = {};

  if (GetProcessMitigationPolicy(GetCurrentProcess(),
                                 ProcessControlFlowGuardPolicy,
                                 &cfgPolicy,
                                 sizeof(cfgPolicy)) == 0) {
    throw std::runtime_error("Failed to retrieve CFG policy. Error: " +
                             std::to_string(GetLastError()));
  }

  if (!cfgPolicy.EnableControlFlowGuard) {
    throw std::runtime_error(
      "Control Flow Guard is not enabled for this process.");
  }
}

auto
IsRunningAsAdmin() -> bool
{
  bool isAdmin = false;
  PSID adminGroup = nullptr;
  SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

  if (AllocateAndInitializeSid(&ntAuthority,
                               2,
                               SECURITY_BUILTIN_DOMAIN_RID,
                               DOMAIN_ALIAS_RID_ADMINS,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               &adminGroup) == 0) {
    const DWORD error = GetLastError();
    throw std::runtime_error(
      "AllocateAndInitializeSid failed with error code: " +
      std::to_string(error));
  }

  if (CheckTokenMembership(
        nullptr, adminGroup, reinterpret_cast<PBOOL>(&isAdmin)) == 0) {
    const DWORD error = GetLastError();
    FreeSid(adminGroup);
    throw std::runtime_error("CheckTokenMembership failed with error code: " +
                             std::to_string(error));
  }

  FreeSid(adminGroup);

  return isAdmin;
}
} // namespace ApplicationController
