#pragma once

#if defined(_MSC_VER)
    #define RHI_DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) | defined(__clang__)
    #define RHI_DEBUG_BREAK() __builtin_trap()
#else
    #error "Unsupported compiler"
#endif

#define RHI_ASSERT(expression) \
    if (!(expression))         \
    {                          \
        RHI_DEBUG_BREAK();     \
    }

#define RHI_UNREACHABLE() RHI_ASSERT(false)

#ifdef _MSC_VER
    #define RHI_INLINE __forceinline
#else
    #define RHI_INLINE inline __attribute__((always_inline))
#endif

#define RHI_NODISCARD [[nodiscard]]