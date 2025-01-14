//
// Created by arteii on 11/28/24.
//

#ifndef HELPER_HPP
#define HELPER_HPP

namespace Server {

#define LOG_STATUS(isSuccess, logMessage)                                      \
  if (isSuccess) {                                                             \
    LOG_INFO(logMessage);                                                 \
  } else {                                                                     \
    LOG_ERROR(logMessage);                                                \
  }

#define HANDLE_ERROR(isSuccess, message)                                       \
  if (!isSuccess) {                                                            \
    LOG_ERROR(message);                                                   \
    throw std::runtime_error(message);                                         \
  }

#define HANDLE_WINHTTP_CALL(call, errorMessage, hRequest)                      \
  if ((call) == FALSE) {                                                       \
    LOG_ERROR(errorMessage + std::string("(WinErr: ") +                   \
                   std::to_string(GetLastError()) + std::string(")"));         \
    WinHttpCloseHandle(hRequest);                                              \
    throw std::runtime_error(errorMessage);                                    \
  }

} // namespace Server

#endif // HELPER_HPP
