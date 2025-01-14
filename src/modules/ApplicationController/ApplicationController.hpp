//
// Created by arteii on 11/2/24.
//

#ifndef APPLICATIONCONTROLLER_HPP
#define APPLICATIONCONTROLLER_HPP
#include "../Utility/Utility.hpp"
#include <Logger.hpp>

namespace ApplicationController {
/**
 * # Immediately exits the application based on the current build mode.
 *
 * - In **release mode** (`NDEBUG` defined), the application will exit with
 *   `EXIT_SUCCESS`.
 * - In **debug mode** (no `NDEBUG` defined), the application will log an
 *   informational message indicating the exit but will not terminate the
 *   process.
 */
inline auto
ImmediateExit() -> void
{
  LOG_INFO("Exiting application...");
#ifdef NDEBUG // check if release mode
  std::exit(EXIT_SUCCESS);
#else
  LOG_INFO("Exiting in debug mode; application will not terminate.");
#endif
}

/**
 * @brief Sets the dynamic code policy for the current process.
 *        Prohibits dynamic code generation, disallows thread opt-out,
 *        and disallows remote downgrade.
 * @throws std::runtime_error if the policy cannot be set.
 */
auto
SetDynamicCodePolicy() -> void;

/**
 * @brief Sets the image load policy for the current process.
 *        Blocks loading DLLs from remote locations, blocks DLLs with low
 *        integrity levels, and prefers loading DLLs from the System32
 *        directory.
 * @throws std::runtime_error if the policy cannot be set.
 */
auto
SetImageLoadPolicy() -> void;

/**
 * @brief Sets the signature policy for the current process.
 *        Allows only Windows-signed binaries to load.
 * @throws std::runtime_error if the policy cannot be set.
 */
auto
SetSignaturePolicy() -> void;

/**
 * @brief Sets the extension point disable policy for the current process.
 *        Disables the use of extension points for the process.
 * @throws std::runtime_error if the policy cannot be set.
 */
auto
SetExtensionPointPolicy() -> void;

/**
 * @brief Sets the child process creation policy for the current process.
 *        Blocks the creation of child processes by the process.
 * @throws std::runtime_error if the policy cannot be set.
 */
auto
SetChildProcessPolicy() -> void;

/**
 * @brief Sets the handle check policy for the current process.
 *        Raises an exception if an invalid handle reference is encountered,
 *        and permanently enables handle exceptions.
 * @throws std::runtime_error if the policy cannot be set.
 */
auto
SetHandleCheckPolicy() -> void;

/**
 * @brief Checks if Control Flow Guard (CFG) is enabled for the current process.
 *
 * This function queries the current process to determine if Control Flow Guard
 * (CFG) mitigation policy is enabled. If CFG is not enabled, it throws a
 * runtime error indicating the absence of this protection. If the query fails,
 * it throws a runtime error with details of the failure.
 *
 * @throws std::runtime_error if CFG is not enabled or if querying the policy
 * fails.
 */
auto
CheckCfg() -> void;

/**
 * @brief Checks whether the current process is running with administrative
 * privileges.
 *
 * This function checks if the current process has been granted membership in
 * the Administrators group, which is required for certain system-logLevel
 * operations. It uses the Windows API to check if the current token is
 * associated with the Administrators group.
 *
 * @return `true` if the process is running with administrative privileges,
 *         otherwise `false`.
 */
auto
IsRunningAsAdmin() -> bool;

/// @brief Initializes the environment by applying various security and
/// execution policies.
///
/// This function attempts to configure the environment by applying multiple
/// policies, such as image load restrictions, dynamic code restrictions,
/// signature enforcement, and extension point disablement. It also verifies if
/// the current process is running with administrative privileges. Each policy
/// application is wrapped in a try-catch block to log errors without
/// interrupting the initialization process.
///
/// @details
/// The following policies are applied in order:
/// - **Image Load Policy**: Restricts loading of unauthorized or unsafe images.
/// - **Dynamic Code Policy**: Prevents dynamic code execution to enhance
/// security.
/// - **Signature Policy**: Enforces valid digital signatures for binaries and
/// scripts.
/// - **Extension Point Policy**: Disables certain extension points to minimize
/// attack surface.
/// - **Child Process Policy**: Limits or restricts the creation of child
/// processes.
/// - **Handle Check Policy**: Implements strict checks on handles to improve
/// process integrity.
///
/// If any of these policies fail to apply, an error is logged using
/// `LOG_ERROR`. The function also checks and logs whether the process is
/// running as an administrator.
///
/// @throws None. Any exceptions encountered during policy application are
/// caught and logged.
///
/// Example log outputs:
/// - "Failed to apply Dynamic Code Policy: [error message]"
/// - "Running as Admin: true"
///
/// @note This function is intended to enhance the security and reliability of
/// the application.
inline auto
InitEnv() -> void
{
  try {
    SetImageLoadPolicy();
  } catch (const std::runtime_error& e) {
    LOG_ERROR("Failed to apply Image Load Policy: " + std::string(e.what()));
  }

  try {
    SetDynamicCodePolicy();
  } catch (const std::runtime_error& e) {
    LOG_ERROR("Failed to apply Dynamic Code Policy: " + std::string(e.what()));
  }

  try {
    SetSignaturePolicy();
  } catch (const std::runtime_error& e) {
    LOG_ERROR("Failed to apply Signature Policy: " + std::string(e.what()));
  }

  try {
    SetExtensionPointPolicy();
  } catch (const std::runtime_error& e) {
    LOG_ERROR("Failed to apply Extension Point Policy: " +
              std::string(e.what()));
  }

  try {
    SetChildProcessPolicy();
  } catch (const std::runtime_error& e) {
    LOG_ERROR("Failed to apply Child Process Policy: " + std::string(e.what()));
  }

  try {
    SetHandleCheckPolicy();
  } catch (const std::runtime_error& e) {
    LOG_ERROR("Failed to apply Handle Check Policy: " + std::string(e.what()));
  }

  try {
    const bool isAdmin = IsRunningAsAdmin();
    LOG_INFO("Running as Admin: " +
             std::string(Utility::BoolToString(isAdmin)));
  } catch (const std::exception& e) {
    LOG_ERROR("Error checking admin status: " + std::string(e.what()));
  }
}

} // namespace ApplicationController

#endif // APPLICATIONCONTROLLER_HPP
