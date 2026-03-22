#pragma once

#include <RHI-D3D12/Export.hpp>

namespace RHI
{
#define RHI_D3D12_EXPORT
    class Device;

    /// @brief Creates a new instance of RHI device, with vulkan backend implementation.
    /// @param appInfo Information regarding the application using this API.
    /// @return return a vulkan implementation of RHI device.
    RHI_D3D12_EXPORT Device* CreateD3D12Device();

    RHI_D3D12_EXPORT void DestroyD3D12Device(Device* device);
} // namespace RHI