#include "Context.hpp"

#include "RHI-Vulkan/Loader.hpp"

#include "FrameScheduler.hpp"
#include "PipelineState.hpp"
#include "Resources.hpp"
#include "Swapchain.hpp"

std::unique_ptr<RHI::Context> RHI::CreateVulkanRHI(const RHI::ApplicationInfo&          appInfo,
                                                      RHI::DeviceSelectionCallback         deviceSelectionCallbacks,
                                                      std::unique_ptr<RHI::DebugCallbacks> debugCallbacks)
{
    return nullptr;
}

namespace Vulkan
{

}  // namespace Vulkan
