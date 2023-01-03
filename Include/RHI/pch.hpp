#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "RHI/Core/Expected.hpp"

#ifdef _WIN64
#    define RHI_WINDOWS
#elif __APPLE__
#    error "Apple Platforms are not supported yet"
#elif __ANDROID__
#    define RHI_ANDROID
#elif __linux__
#    define RHI_LINUX
#else
#    error "Unknown compiler"
#endif
