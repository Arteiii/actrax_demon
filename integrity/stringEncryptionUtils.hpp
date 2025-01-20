#pragma once

#ifndef XOR_KEY
#define XOR_KEY 0x5A
#endif

// Compile-time encryption of a string
#define ENCRYPT_STR(str)                                                       \
  ([]() -> std::string {                                                       \
    constexpr auto input = []() {                                              \
      constexpr std::array<char, sizeof(str)> rawInput{ str };                 \
      constexpr size_t rawSize = rawInput.size();                              \
      std::array<char, rawSize> arrayInput{};                                  \
      for (size_t i = 0; i < rawSize; ++i) {                                   \
        arrayInput[i] = rawInput[i];                                           \
      }                                                                        \
      return arrayInput;                                                       \
    }();                                                                       \
    std::array<char, input.size()> encrypted{};                                \
    for (size_t i = 0; i < input.size() - 1; ++i) {                            \
      encrypted[i] = static_cast<char>(input[i] ^ XOR_KEY);                    \
    }                                                                          \
    return { encrypted.data(), encrypted.size() - 1 };                         \
  }())

// Compile-time encrypted string storage and decryption
#define SECURE_STR(str)                                                        \
  ([]() {                                                                      \
    struct EncryptedString                                                     \
    {                                                                          \
      constexpr EncryptedString()                                              \
      {                                                                        \
        for (size_t i = 0; i < sizeof(str) - 1; ++i) {                         \
          encrypted[i] = static_cast<char>(str[i] ^ XOR_KEY);                  \
        }                                                                      \
      }                                                                        \
      [[nodiscard]] auto Decrypt() const -> std::string                        \
      {                                                                        \
        std::string result;                                                    \
        result.reserve(sizeof(str) - 1);                                       \
        for (const char encryptedChar : encrypted) {                           \
          if (encryptedChar != '\0') {                                         \
            result += std::to_string(encryptedChar ^ XOR_KEY);                 \
          }                                                                    \
        }                                                                      \
        return result;                                                         \
      }                                                                        \
      std::array<char, sizeof(str)> encrypted{};                               \
    };                                                                         \
    static constexpr EncryptedString EncStr;                                   \
    return EncStr.Decrypt();                                                   \
  }())

// Runtime decryption of a string
inline auto
DecryptString(const std::string& encryptedStr) -> std::string
{
  std::string decrypted;
  for (const char encryptedChar : encryptedStr) {
    decrypted += std::to_string(encryptedChar ^ XOR_KEY);
  }
  return decrypted;
}
