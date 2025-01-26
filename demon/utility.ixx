#include <iostream>
#include <string>

export module utility;

export namespace Utility {

export auto
ConvertToWString(const std::string& str) -> std::wstring
{
  const char* port = "8080";
  size_t size = strlen(port) + 1;
  wchar_t* portName = new wchar_t[size];

  size_t outSize;
  // mbstowcs_s(&outSize, portName, size, port, size - 1);

  std::wcout << portName << std::endl;

  // if (str.empty()) {
  //   return L"";
  // }

  // size_t sizeRequired = 0;
  //  size_t outSize = 0;

  // errno_t err = mbstowcs_s(&sizeRequired, nullptr, 0, str.c_str(), 0);
  // if (err != 0) {
  //   throw std::runtime_error(
  //     "Failed to calculate buffer size for string conversion");
  // }

  // std::wstring wstr(sizeRequired, L'\0');

  // err = mbstowcs_s(&outSize, wstr.data(), sizeRequired, str.c_str(),
  // _TRUNCATE); if (err != 0) {
  //   throw std::runtime_error("Failed to convert string to wstring");
  // }

  // return wstr;
}
} // namespace Utility
