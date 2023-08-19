#pragma once

#ifdef _MSC_VER
    #define RHI_FORCE_INLINE __forceinline
#else
    #define RHI_FORCE_INLINE inline __attribute__((always_inline))
#endif
