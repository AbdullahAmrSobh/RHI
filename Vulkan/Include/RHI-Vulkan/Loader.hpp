#pragma once
#include <memory>

#include "RHI-Vulkan/Export.hpp"

#include <RHI/Context.hpp>

namespace RHI
{

class Context;

/// @brief Creates a new instance of RHI context, with vulkan backend implementation.
/// @param appInfo Information regarding the application using this API.
/// @param deviceSelectionCallbacks Callback to a function which will return the id of a device to create the context with.
/// @param debugCallbacks debug callbacks called to log info about the API to the application.
/// @return return a vulkan implementation of RHI context.
std::unique_ptr<Context> RHI_Vulkan_EXPORT CreateVulkanRHI(const ApplicationInfo&          appInfo,
                                                           DeviceSelectionCallback         deviceSelectionCallbacks,
                                                           std::unique_ptr<DebugCallbacks> debugCallbacks = nullptr);

}  // namespace RHI