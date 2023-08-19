
#ifndef RHI_VULKAN_EXPORT_H
#define RHI_VULKAN_EXPORT_H

#ifdef RHI_VULKAN_STATIC_DEFINE
#  define RHI_VULKAN_EXPORT
#  define RHI_VULKAN_NO_EXPORT
#else
#  ifndef RHI_VULKAN_EXPORT
#    ifdef RHI_Vulkan_EXPORTS
        /* We are building this library */
#      define RHI_VULKAN_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define RHI_VULKAN_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef RHI_VULKAN_NO_EXPORT
#    define RHI_VULKAN_NO_EXPORT 
#  endif
#endif

#ifndef RHI_VULKAN_DEPRECATED
#  define RHI_VULKAN_DEPRECATED __declspec(deprecated)
#endif

#ifndef RHI_VULKAN_DEPRECATED_EXPORT
#  define RHI_VULKAN_DEPRECATED_EXPORT RHI_VULKAN_EXPORT RHI_VULKAN_DEPRECATED
#endif

#ifndef RHI_VULKAN_DEPRECATED_NO_EXPORT
#  define RHI_VULKAN_DEPRECATED_NO_EXPORT RHI_VULKAN_NO_EXPORT RHI_VULKAN_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef RHI_VULKAN_NO_DEPRECATED
#    define RHI_VULKAN_NO_DEPRECATED
#  endif
#endif

/* This needs to suppress only for MSVC */
#if defined(_MSC_VER) && !defined(__ICL)
#  define RHI_SUPPRESS_C4251 _Pragma("warning(suppress:4251)")
#else
#  define RHI_SUPPRESS_C4251
#endif

#endif /* RHI_VULKAN_EXPORT_H */
