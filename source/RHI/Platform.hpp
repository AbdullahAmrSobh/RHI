#pragma once

#ifdef _WIN64
#    define RHI_WINDOWS
#elif __APPLE__
#    error "Apple Platforms are not supported yet"
#elif __ANDROID__
#    define RHI_ANDROID
#elif __linux__
#    define RHI_LINUX
#else
#    error "Unknown Platform"
#endif
