#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
    #define NOMINMAX
#endif

#include <RHI/RHI.h>

#include <D3D12MemAlloc.h>
#include <d3d12.h>
#include <dxgi1_6.h>

namespace RHI::D3D12
{
    inline constexpr uint32_t MaxCommandListsPerPool = 64;
    inline constexpr uint32_t MaxBindingsPerGroup = 64;
} // namespace RHI::D3D12
