
#ifndef Vulkan_H
#define Vulkan_H

#ifdef VULKAN_STATIC_DEFINE
#  define Vulkan
#  define VULKAN_NO_EXPORT
#else
#  ifndef Vulkan
#    ifdef Vulkan_EXPORTS
        /* We are building this library */
#      define Vulkan __declspec(dllexport)
#    else
        /* We are using this library */
#      define Vulkan __declspec(dllimport)
#    endif
#  endif

#  ifndef VULKAN_NO_EXPORT
#    define VULKAN_NO_EXPORT 
#  endif
#endif

#ifndef VULKAN_DEPRECATED
#  define VULKAN_DEPRECATED __declspec(deprecated)
#endif

#ifndef VULKAN_DEPRECATED_EXPORT
#  define VULKAN_DEPRECATED_EXPORT Vulkan VULKAN_DEPRECATED
#endif

#ifndef VULKAN_DEPRECATED_NO_EXPORT
#  define VULKAN_DEPRECATED_NO_EXPORT VULKAN_NO_EXPORT VULKAN_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef VULKAN_NO_DEPRECATED
#    define VULKAN_NO_DEPRECATED
#  endif
#endif

/* This needs to suppress only for MSVC */
#if defined(_MSC_VER) && !defined(__ICL)
#  define RHI_SUPPRESS_C4251 _Pragma("warning(suppress:4251)")
#else
#  define RHI_SUPPRESS_C4251
#endif

#endif /* Vulkan_H */