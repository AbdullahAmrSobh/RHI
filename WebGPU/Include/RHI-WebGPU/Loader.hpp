#pragma once
#include <RHI/Device.hpp>

#include <TL/Ptr.hpp>

#include <RHI-WebGPU/Export.hpp>

namespace RHI
{
    /// @brief Creates a new instance of RHI device, with WebGPU backend implementation.
    /// @param appInfo Information regarding the application using this API.
    /// @return return a WebGPU implementation of RHI device.
    RHI_WebGPU_EXPORT Device* CreateWebGPUDevice();

    RHI_WebGPU_EXPORT void DestroyWebGPUDevice(Device* device);
} // namespace RHI