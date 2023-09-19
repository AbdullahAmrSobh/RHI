#include "pch.hpp"

#include "RHI-Vulkan/Loader.hpp"

#include "Context.hpp"

namespace RHI
{

std::unique_ptr<Context> CreateVulkanRHI(const ApplicationInfo&          appInfo,
                                         DeviceSelectionCallback         deviceSelectionCallbacks,
                                         std::unique_ptr<DebugCallbacks> debugCallbacks)
{
    return std::unique_ptr<Context>(Vulkan::Context::Create(appInfo, deviceSelectionCallbacks, std::move(debugCallbacks)));
}

}  // namespace RHI