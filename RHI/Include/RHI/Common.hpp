#pragma once

#include <algorithm>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "RHI/Flags.hpp"
#include "RHI/Format.hpp"
#include "RHI/Result.hpp"

#if defined(_MSC_VER)
#    define RHI_DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) | defined(__clang__)
#    define RHI_DEBUG_BREAK() __builtin_trap()
#else
#    error "Unsupported compiler"
#endif

#define RHI_ASSERT(expression) \
    if (!(expression))         \
    {                          \
        RHI_DEBUG_BREAK();     \
    }

#define RHI_UNREACHABLE() RHI_ASSERT(false)

#ifdef _MSC_VER
#    define RHI_FORCE_INLINE __forceinline
#else
#    define RHI_FORCE_INLINE inline __attribute__((always_inline))
#endif

#define RHI_NODISCARD [[nodiscard]]