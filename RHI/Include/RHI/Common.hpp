#pragma once

#ifdef _MSC_VER
    #define RHI_FORCE_INLINE __forceinline
#else
    #define RHI_FORCE_INLINE inline __attribute__((always_inline))
#endif

// Detect the current platfrom
#if defined(_WIN32) || defined(_WIN64)
    #define RHI_PLATFORM_WINDOWS
#elif defined(__ANDROID__)
    #define PLATFORM_ANDROID
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_MAC == 1
    #define RHI_PLATFORM_MACOS
#elif TARGET_OS_IPHONE == 1
    #define RHI_PLATFORM_IOS
#endif
#elif defined(__linux__)
    #define RHI_PLATFORM_LINUX
#elif defined(__FreeBSD__)
    #define RHI_PLATFORM_FREEBSD
#endif

// Detect the system architecture
#if defined(__x86_64__) || defined(_M_X64) || defined(__amd64__)
    #define RHI_SYSTEM_ARCHITECTURE_X86_64
#elif defined(__i386__) || defined(_M_IX86)
    #define RHI_SYSTEM_ARCHITECTURE_X86_32
#elif defined(__arm__) || defined(__aarch64__)
    #define RHI_SYSTEM_ARCHITECTURE_ARM
#else
    #define RHI_SYSTEM_ARCHITECTURE_UNKNOWN
#endif
