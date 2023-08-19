#include "Context.hpp"
#include "RHI-Vulkan/Loader.hpp"

#include "PipelineState.hpp"
#include "Resources.hpp"
#include "Swapchain.hpp"
#include "FrameScheduler.hpp"
#include "PipelineState.hpp"

std::unique_ptr<Vulkan::Context> RHI::CreateVulkanRHI(const RHI::ApplicationInfo& appInfo, RHI::DeviceSelectionCallback deviceSelectionCallbacks, RHI::DebugCallbacks* debugCallbacks)
{
    return nullptr;
}

namespace Vulkan
{

RHI::Handle<RHI::Pass> Context::CreatePass(const RHI::PassCreateInfo& createInfo)
{
    return {};
}

std::unique_ptr<RHI::FrameScheduler> Context::CreateFrameScheduler()
{
    return {};
}

std::unique_ptr<RHI::Swapchain> Context::CreateSwapchain(const RHI::SwapchainCreateInfo& createInfo)
{
    return {};
}

std::unique_ptr<RHI::ResourcePool> Context::CreateResourcePool(const RHI::ResourcePoolCreateInfo& createInfo)
{
    return {};
}

std::unique_ptr<RHI::PipelineStateCache> Context::CreatePipelineStateCache(const RHI::PipelineStateCacheCreateInfo& createInfo)
{
    return {};
}

RHI::Handle<RHI::PipelineState> Context::CreateGraphicsPipeline(const RHI::GraphicsPipelineCreateInfo& createInfo)
{
    return {};
}

RHI::Handle<RHI::PipelineState> Context::CreateComputePipeline(const RHI::ComputePipelineCreateInfo& createInfo)
{
    return {};
}

RHI::Handle<RHI::PipelineState> Context::CreateRayTracingPipeline(const RHI::RayTracingPipelineCreateInfo& createInfo)
{
    return {};
}

RHI::Handle<RHI::Sampler> Context::CreateSampler(const RHI::SamplerCreateInfo& createInfo)
{
    return {};
}

void Context::Free(RHI::Handle<RHI::Pass> pass)
{
}

void Context::Free(RHI::Handle<RHI::PipelineState> pso)
{
}

void Context::Free(RHI::Handle<RHI::Sampler> sampler)
{
}

}  // namespace Vulkan
