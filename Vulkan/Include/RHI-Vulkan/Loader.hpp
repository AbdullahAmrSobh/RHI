#pragma once
#include <RHI-Vulkan/Export.hpp>
#include <RHI/Device.hpp>

#include <TL/UniquePtr.hpp>

namespace RHI
{
    struct ApplicationInfo;

    /// @brief Creates a new instance of RHI device, with vulkan backend implementation.
    /// @param appInfo Information regarding the application using this API.
    /// @return return a vulkan implementation of RHI device.
    RHI_Vulkan_EXPORT TL::Ptr<Device> CreateVulkanDevice(const ApplicationInfo& appInfo);
} // namespace RHI