#pragma once

#include <RHI/RHI.hpp>

#include <TL/Assert.hpp>

#include <webgpu/webgpu_cpp.h>

namespace RHI::WebGPU
{
    inline static WGPUStringView ConvertToStringView(const char* str)
    {
        return {.data = str, .length = WGPU_STRLEN};
    }

    inline static WGPUStringView ConvertToStringViewFmt(const char* str, ...)
    {
        return {.data = str, .length = WGPU_STRLEN};
    }

    inline static WGPUColor ConvertToColor(ClearValue clearValue)
    {
        return {
            clearValue.f32.r,
            clearValue.f32.g,
            clearValue.f32.b,
            clearValue.f32.a,
        };
    }

    inline static WGPUStorageTextureAccess ConvertStorageTextureAccess(RHI::Access access)
    {
        switch (access)
        {
        case Access::None:      return WGPUStorageTextureAccess_Undefined;
        case Access::Read:      return WGPUStorageTextureAccess_ReadOnly;
        case Access::Write:     return WGPUStorageTextureAccess_WriteOnly;
        case Access::ReadWrite: return WGPUStorageTextureAccess_ReadWrite;
        }
        return WGPUStorageTextureAccess_Force32;
    }

} // namespace RHI::WebGPU