#pragma once

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#define NOMINMAX            // Exclude min and max macros

#include "Logger.hpp"

#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <vector>
#include <windows.h>
