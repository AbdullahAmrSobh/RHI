#pragma once

#include <RHI/RHI.hpp>

#include <TL/Assert.hpp>

#include <webgpu/webgpu_cpp.h>

namespace RHI::WebGPU
{
    inline static WGPUStringView ConvertToStringView(const char* str)
    {
        return {str, strlen(str)};
    }

    inline static WGPUStringView ConvertToStringViewFmt(const char* str, ...)
    {
        return {str, strlen(str)};
    }

} // namespace RHI::WebGPU