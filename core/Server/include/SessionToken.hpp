//
// Created by arteii on 11/27/24.
//

#ifndef SESSIONTOKEN_HPP
#define SESSIONTOKEN_HPP

#include <StringEncryptionUtils.hpp>

#include <chrono>
#include <stdexcept>
#include <string>

#include <windows.h>

namespace Server {
class SessionToken
{
public:
  class TokenExpiredException final : public std::runtime_error
  {
  public:
    explicit TokenExpiredException(const std::string& message)
      : std::runtime_error(message)
    {
    }
  };

  SessionToken()
  {

    hKernel32_ = LoadLibraryA(SECURE_STR("kernel32.dll").c_str());
    if (hKernel32_ == nullptr) {
      throw std::runtime_error(SECURE_STR("Failed to load kernel32.dll."));
    }

    createMutex_ = reinterpret_cast<create_mutex_a_fn>(
      GetProcAddress(hKernel32_, SECURE_STR("CreateMutexA").c_str()));
    closeHandle_ = reinterpret_cast<close_handle_fn>(
      GetProcAddress(hKernel32_, SECURE_STR("CloseHandle").c_str()));
    waitForSingleObject_ = reinterpret_cast<wait_for_single_object_fn>(
      GetProcAddress(hKernel32_, SECURE_STR("WaitForSingleObject").c_str()));
    releaseMutex_ = reinterpret_cast<release_mutex_fn>(
      GetProcAddress(hKernel32_, SECURE_STR("ReleaseMutex").c_str()));

    if ((createMutex_ == nullptr) || (closeHandle_ == nullptr) ||
        (waitForSingleObject_ == nullptr) || (releaseMutex_ == nullptr)) {
      FreeLibrary(hKernel32_);
      throw std::runtime_error(
        SECURE_STR("Failed to retrieve function addresses."));
    }

    mutexHandle_ = createMutex_(nullptr, FALSE, nullptr);
    if (mutexHandle_ == nullptr) {
      throw std::runtime_error(SECURE_STR("Failed to create mutex."));
    }
  }

  ~SessionToken()
  {
    if (mutexHandle_ != nullptr) {
      closeHandle_(mutexHandle_);
    }
    if (hKernel32_ != nullptr) {
      FreeLibrary(hKernel32_);
    }
  }

  auto
  SetToken(const std::string& token, const int64_t validityInSeconds) -> void
  {
    LockMutex();
    token_ = token;
    expiryTime_ = GetCurrentTimestamp() + validityInSeconds;
    UnlockMutex();
  }

  [[nodiscard]] auto
  GetToken() const -> std::string
  {
    LockMutex();
    if (IsExpired()) {
      UnlockMutex();
      throw TokenExpiredException(SECURE_STR("Session token has expired."));
    }
    std::string currentToken = token_;
    UnlockMutex();
    return currentToken;
  }

  [[nodiscard]] auto
  IsExpired() const -> bool
  {
    return GetCurrentTimestamp() > expiryTime_;
  }

private:
  HMODULE hKernel32_;
  HANDLE mutexHandle_;
  std::string token_;
  int64_t expiryTime_{ 0 };

  using create_mutex_a_fn = HANDLE(WINAPI*)(LPSECURITY_ATTRIBUTES,
                                            BOOL,
                                            LPCSTR);
  using close_handle_fn = BOOL(WINAPI*)(HANDLE);
  using wait_for_single_object_fn = DWORD(WINAPI*)(HANDLE, DWORD);
  using release_mutex_fn = BOOL(WINAPI*)(HANDLE);

  create_mutex_a_fn createMutex_;
  close_handle_fn closeHandle_;
  wait_for_single_object_fn waitForSingleObject_;
  release_mutex_fn releaseMutex_;

  auto
  LockMutex() const -> void
  {
    if (const DWORD waitResult = waitForSingleObject_(mutexHandle_, INFINITE);
        waitResult != WAIT_OBJECT_0) {
      throw std::runtime_error(SECURE_STR("Failed to acquire mutex lock."));
    }
  }

  auto
  UnlockMutex() const -> void
  {
    if (releaseMutex_(mutexHandle_) == 0) {
      throw std::runtime_error(SECURE_STR("Failed to release mutex lock."));
    }
  }

  static auto
  GetCurrentTimestamp() -> int64_t
  {
    return std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
  }
};

} // namespace Server

#endif // SESSIONTOKEN_HPP
