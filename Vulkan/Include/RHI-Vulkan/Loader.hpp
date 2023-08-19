#pragma once
#include <memory>

#include "RHI-Vulkan/Export.hpp"

#include <RHI/Context.hpp>

namespace Vulkan
{
class Context;
}

namespace RHI
{

/// @brief Creates a new instance of RHI context, with vulkan backend implementation.
/// @param appInfo Information regarding the application using this API.
/// @param deviceSelectionCallbacks Callback to a function which will return the id of a device to create the context with.
/// @param debugCallbacks debug callbacks called to log info about the API to the application.
/// @return return a vulkan implementation of RHI context.
std::unique_ptr<Vulkan::Context> RHI_Vulkan_EXPORT CreateVulkanRHI(const ApplicationInfo&  appInfo,
                                                                   DeviceSelectionCallback deviceSelectionCallbacks,
                                                                   DebugCallbacks*         debugCallbacks = nullptr);

}  // namespace RHI