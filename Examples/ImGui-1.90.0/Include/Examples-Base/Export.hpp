
#ifndef Examples_Base_EXPORT_H
#define Examples_Base_EXPORT_H

#ifdef BASE_STATIC_DEFINE
#  define Examples_Base_EXPORT
#  define BASE_NO_EXPORT
#else
#  ifndef Examples_Base_EXPORT
#    ifdef Base_EXPORTS
        /* We are building this library */
#      define Examples_Base_EXPORT 
#    else
        /* We are using this library */
#      define Examples_Base_EXPORT 
#    endif
#  endif

#  ifndef BASE_NO_EXPORT
#    define BASE_NO_EXPORT 
#  endif
#endif

#ifndef BASE_DEPRECATED
#  define BASE_DEPRECATED __declspec(deprecated)
#endif

#ifndef BASE_DEPRECATED_EXPORT
#  define BASE_DEPRECATED_EXPORT Examples_Base_EXPORT BASE_DEPRECATED
#endif

#ifndef BASE_DEPRECATED_NO_EXPORT
#  define BASE_DEPRECATED_NO_EXPORT BASE_NO_EXPORT BASE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef BASE_NO_DEPRECATED
#    define BASE_NO_DEPRECATED
#  endif
#endif

/* This needs to suppress only for MSVC */
#if defined(_MSC_VER) && !defined(__ICL)
#  define RHI_SUPPRESS_C4251 _Pragma("warning(suppress:4251)")
#else
#  define RHI_SUPPRESS_C4251
#endif

#endif /* Examples_Base_EXPORT_H */
