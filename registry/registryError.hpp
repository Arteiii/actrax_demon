#pragma once

#include "pch.h"

namespace Registry {

/**
 * @brief Represents an error that occurs within the Windows Registry
 * operations.
 *
 * This class extends std::runtime_error to provide additional context
 * specifically for Windows Registry errors, including the error code.
 */
class Error final : public std::runtime_error
{
public:
  /**
   * @brief Constructs a Registry::Error object.
   *
   * @param message A descriptive error message.
   * @param errorCode The Windows error code associated with the error.
   */
  Error(const std::string& message, const LSTATUS errorCode)
    : std::runtime_error(message + " Error Code: " + std::to_string(errorCode))
    , errorCode_(errorCode)
  {
  }

  /**
   * @brief Retrieves the Windows error code associated with the error.
   *
   * @return The LSTATUS error code.
   */
  [[nodiscard]] auto GetErrorCode() const -> LSTATUS { return errorCode_; }
  auto

  SetErrorCode(const LSTATUS errorCode) -> void
  {
    errorCode_ = errorCode;
  }

private:
  LSTATUS errorCode_; ///< Stores the original Windows error code.
};

} // namespace Registry
