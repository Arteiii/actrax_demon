#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>

#ifndef COMPILE_TIME_MIN_LOG_LEVEL
#define COMPILE_TIME_MIN_LOG_LEVEL log_level::Trace
#endif

#ifndef COMPILE_TIME_MINIMAL_OUTPUT
#define COMPILE_TIME_MINIMAL_OUTPUT 0
#endif

/**
 * @brief Enum for log levels, allowing categorization of messages by severity.
 */
enum class log_level
{
  None = -1, ///< No logging, used to disable logging entirely.
  Trace = 0, ///< Trace messages for detailed logging, used for debugging.
  Debug,     ///< Debug messages for troubleshooting and development.
  Info,      ///< Informational messages for general updates.
  Warn,      ///< Warning messages indicating potential issues or concerns.
  Error,     ///< Error messages indicating failures or serious issues.
};

/**
 * @brief Compile-time log level threshold.
 * Defines the minimum log level that will be processed at compile time.
 * Logs with levels lower than this will be ignored.
 *
 * To customize the compile-time log level threshold, define this before
 * including the logger:
 * ```cpp
 * #define COMPILE_TIME_MIN_LOG_LEVEL log_level::Info
 * ```
 */
#ifndef COMPILE_TIME_MIN_LOG_LEVEL
#define COMPILE_TIME_MIN_LOG_LEVEL log_level::Trace
#endif

/**
 * @brief Helper function to determine if a log level is enabled at compile
 * time.
 *
 * @param msgLevel The log level to check.
 * @return true if the log level is enabled based on the compile-time threshold.
 * @return false if the log level is below the compile-time threshold.
 */
constexpr auto
IsLogLevelEnabled(log_level msgLevel) -> bool
{
  return static_cast<int>(msgLevel) >=
         static_cast<int>(COMPILE_TIME_MIN_LOG_LEVEL);
}

/**
 * @brief Logger class that provides logging functionality with different log
 * levels.
 *
 * This class is a singleton and ensures that logging happens in a thread-safe
 * manner. It supports different log levels (None, Trace, Debug, Info, Warn,
 * Error) and includes optional minimal or detailed output formats. Logs are
 * output to the standard console (stdout).
 */
class Logger
{
public:
  /**
   * @brief Returns the singleton instance of the Logger.
   *
   * The Logger instance is guaranteed to be created lazily when needed, and it
   * is destroyed when the program terminates.
   *
   * @return Logger& A reference to the Logger instance.
   */
  static auto Instance() -> Logger&
  {
    static Logger instance; // Guaranteed to be destroyed when the program exits
    return instance;
  }

  /**
   * @brief Logs a message with a specified log level.
   *
   * This function logs a message to the console, including the log level and,
   * depending on the configuration, additional details such as timestamp, file,
   * and line number.
   *
   * @param msgLevel The log level for the message (e.g., Trace, Debug, Info,
   * etc.).
   * @param message The message to log.
   * @param file The file where the log was called.
   * @param line The line number where the log was called.
   */
  static auto Log(const log_level msgLevel,
                  const std::string& message,
                  const std::string& file,
                  const int line) -> void
  {
    if (IsLogLevelEnabled(msgLevel)) {
      if constexpr (COMPILE_TIME_MINIMAL_OUTPUT) {
        std::cout << "[" << GetLogLevelString(msgLevel) << "] " << message
                  << std::endl;
      } else {
        std::cout << "[" << GetCurrentTimestamp() << "] "
                  << "[" << GetLogLevelString(msgLevel) << "] "
                  << "[" << file << ":" << line << "] " << message << std::endl;
      }
    }
  }

  // Overloaded Log function for wide string
  static auto Log(const log_level msgLevel,
                  const std::wstring& wmessage,
                  const std::string& file,
                  const int line) -> void
  {
    if (IsLogLevelEnabled(msgLevel)) {
      if constexpr (COMPILE_TIME_MINIMAL_OUTPUT) {
        std::wcout << L"[" << GetLogLevelString(msgLevel).c_str() << L"] "
                   << wmessage << std::endl;
      } else {
        std::wcout << L"[" << GetCurrentTimestamp().c_str() << L"] " << "["
                   << GetLogLevelString(msgLevel).c_str() << L"] " << L"["
                   << file.c_str() << L":" << line << L"] " << wmessage
                   << std::endl;
      }
    }
  }

  /**
   * @brief Converts a log level enum to its string representation.
   *
   * This function converts the log level enum to a human-readable string,
   * such as "INFO" for level::Info.
   *
   * @param logLevel The log level to convert.
   * @return std::string The string representation of the log level.
   */
  static auto GetLogLevelString(const log_level logLevel) -> std::string
  {
    switch (logLevel) {
      case log_level::None:
        return "NONE";
      case log_level::Trace:
        return "TRACE";
      case log_level::Debug:
        return "DEBUG";
      case log_level::Info:
        return "INFO";
      case log_level::Warn:
        return "WARN";
      case log_level::Error:
        return "ERROR";
      default:
        return "UNKNOWN";
    }
  }

  /**
   * @brief Returns the current timestamp in the format "YYYY-MM-DD HH:MM:SS".
   *
   * This function is used for detailed logging to include the timestamp with
   * the log message.
   *
   * @return std::string The current timestamp as a string.
   */
  static auto GetCurrentTimestamp() -> std::string
  {
    const auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm xxx;

#ifdef _WIN32
    localtime_s(&xxx, &time);
#else
    localtime_r(&time, &tm); // POSIX systems
#endif

    std::ostringstream oss;
    oss << std::put_time(&xxx, "%Y-%m-%d %H:%M:%S");
    return oss.str();
  }

private:
  Logger() = default;
};

/**
 * @brief Macro to log a message with a specific log level.
 *
 * This macro calls the Logger::Log function with the given level, message,
 * and the current file and line number.
 *
 * @param level The log level for the message.
 * @param msg The message to log.
 */
#define LOG(level, msg)                                                        \
  if constexpr (IsLogLevelEnabled(level)) {                                    \
    Logger::Log(level, msg, __FILE__, __LINE__);                               \
  }

/**
 * @brief Macro to log a trace message.
 */
#define LOG_TRACE(msg) LOG(log_level::Trace, msg)

/**
 * @brief Macro to log a debug message.
 */
#define LOG_DEBUG(msg) LOG(log_level::Debug, msg)

/**
 * @brief Macro to log an informational message.
 */
#define LOG_INFO(msg) LOG(log_level::Info, msg)

/**
 * @brief Macro to log a warning message.
 */
#define LOG_WARN(msg) LOG(log_level::Warn, msg)

/**
 * @brief Macro to log an error message.
 */
#define LOG_ERROR(msg) LOG(log_level::Error, msg)
