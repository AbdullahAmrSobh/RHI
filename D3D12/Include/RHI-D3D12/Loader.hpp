#pragma once

#include <RHI-D3D12/Export.hpp>

namespace RHI
{
    class Device;

    RHI_D3D12_EXPORT Device* CreateD3D12Device();

    RHI_D3D12_EXPORT void DestroyD3D12Device(Device* device);
} // namespace RHI