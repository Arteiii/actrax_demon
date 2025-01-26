#pragma once

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "../integrity/stringEncryptionUtils.hpp"
#include "../logger/Logger.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <windows.h>
#include <winhttp.h>
