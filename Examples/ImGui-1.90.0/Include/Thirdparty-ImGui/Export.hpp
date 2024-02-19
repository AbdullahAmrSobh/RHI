
#ifndef Thirdparty_ImGui_EXPORT_H
#define Thirdparty_ImGui_EXPORT_H

#ifdef IMGUI_STATIC_DEFINE
#  define Thirdparty_ImGui_EXPORT
#  define IMGUI_NO_EXPORT
#else
#  ifndef Thirdparty_ImGui_EXPORT
#    ifdef ImGui_EXPORTS
        /* We are building this library */
#      define Thirdparty_ImGui_EXPORT 
#    else
        /* We are using this library */
#      define Thirdparty_ImGui_EXPORT 
#    endif
#  endif

#  ifndef IMGUI_NO_EXPORT
#    define IMGUI_NO_EXPORT 
#  endif
#endif

#ifndef IMGUI_DEPRECATED
#  define IMGUI_DEPRECATED 
#endif

#ifndef IMGUI_DEPRECATED_EXPORT
#  define IMGUI_DEPRECATED_EXPORT Thirdparty_ImGui_EXPORT IMGUI_DEPRECATED
#endif

#ifndef IMGUI_DEPRECATED_NO_EXPORT
#  define IMGUI_DEPRECATED_NO_EXPORT IMGUI_NO_EXPORT IMGUI_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef IMGUI_NO_DEPRECATED
#    define IMGUI_NO_DEPRECATED
#  endif
#endif

/* This needs to suppress only for MSVC */
#if defined(_MSC_VER) && !defined(__ICL)
#  define RHI_SUPPRESS_C4251 _Pragma("warning(suppress:4251)")
#else
#  define RHI_SUPPRESS_C4251
#endif

#endif /* Thirdparty_ImGui_EXPORT_H */
