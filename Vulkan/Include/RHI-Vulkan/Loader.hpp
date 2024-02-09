#pragma once
#include "RHI-Vulkan/Export.hpp"

#include <RHI/Context.hpp>

namespace RHI
{

    struct ApplicationInfo;

    class Context;
    class DebugCallbacks;

    /// @brief Creates a new instance of RHI context, with vulkan backend implementation.
    /// @param appInfo Information regarding the application using this API.
    /// @param debugCallbacks debug callbacks called to log info about the API to the application.
    /// @return return a vulkan implementation of RHI context.
    RHI_Vulkan_EXPORT Ptr<Context> CreateVulkanContext(const ApplicationInfo& appInfo, Ptr<DebugCallbacks> debugCallbacks = nullptr);

} // namespace RHI