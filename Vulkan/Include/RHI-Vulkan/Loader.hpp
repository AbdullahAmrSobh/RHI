#pragma once
#include <RHI/Device.hpp>

#include <TL/Ptr.hpp>

#include <RHI-Vulkan/Export.hpp>

namespace RHI
{

    struct Version
    {
        uint16_t major = 0;
        uint16_t minor = 0;
        uint32_t patch = 0;
    };

    struct ApplicationInfo
    {
        const char* applicationName    = nullptr; // The name of the users application.
        Version     applicationVersion = {};      // The version of the users application.
        const char* engineName         = nullptr; // The version of the users application.
        Version     engineVersion      = {};      // The version of the users application.
    };

    /// @brief Creates a new instance of RHI device, with vulkan backend implementation.
    /// @param appInfo Information regarding the application using this API.
    /// @return return a vulkan implementation of RHI device.
    RHI_Vulkan_EXPORT Device* CreateVulkanDevice(const ApplicationInfo& appInfo);

    RHI_Vulkan_EXPORT void DestroyVulkanDevice(Device* device);
} // namespace RHI