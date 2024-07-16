
#ifndef RHI_EXPORT_H
#define RHI_EXPORT_H

#ifdef RHI_STATIC_DEFINE
    #define RHI_EXPORT
    #define RHI_NO_EXPORT
#else
    #ifndef RHI_EXPORT
        #ifdef RHI_EXPORTS
            /* We are building this library */
            #define RHI_EXPORT
        #else
            /* We are using this library */
            #define RHI_EXPORT
        #endif
    #endif

    #ifndef RHI_NO_EXPORT
        #define RHI_NO_EXPORT
    #endif
#endif

#ifndef RHI_DEPRECATED
    #define RHI_DEPRECATED
#endif

#ifndef RHI_DEPRECATED_EXPORT
    #define RHI_DEPRECATED_EXPORT RHI_EXPORT RHI_DEPRECATED
#endif

#ifndef RHI_DEPRECATED_NO_EXPORT
    #define RHI_DEPRECATED_NO_EXPORT RHI_NO_EXPORT RHI_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
    #ifndef RHI_NO_DEPRECATED
        #define RHI_NO_DEPRECATED
    #endif
#endif

/* This needs to suppress only for MSVC */
#if defined(_MSC_VER) && !defined(__ICL)
    #define RHI_SUPPRESS_C4251 _Pragma("warning(suppress:4251)")
#else
    #define RHI_SUPPRESS_C4251
#endif

#endif /* RHI_EXPORT_H */
