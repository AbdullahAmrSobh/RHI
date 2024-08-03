#pragma once
#include <RHI-Vulkan/Export.hpp>
#include <RHI/Context.hpp>

namespace RHI
{
    struct ApplicationInfo;
    class Context;

    /// @brief Creates a new instance of RHI context, with vulkan backend implementation.
    /// @param appInfo Information regarding the application using this API.
    /// @return return a vulkan implementation of RHI context.
    RHI_Vulkan_EXPORT Ptr<Context> CreateVulkanContext(const ApplicationInfo& appInfo);
} // namespace RHI