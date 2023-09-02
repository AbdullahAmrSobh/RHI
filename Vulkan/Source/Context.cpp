
#include "Context.hpp"

#include "RHI-Vulkan/Loader.hpp"

#include <RHI/Pipeline.hpp>
#include <RHI/ShaderBindGroup.hpp>

#include "FrameScheduler.hpp"
#include "Pipeline.hpp"
#include "ResourcePool.hpp"
#include "Swapchain.hpp"

namespace Vulkan
{

Context::~Context()
{
}

Context* Context::Create(const RHI::ApplicationInfo& appInfo, RHI::DeviceSelectionCallback deviceSelectionCallbacks, std::unique_ptr<RHI::DebugCallbacks> debugCallbacks)
{
    auto context = new Context();
    return context;
}

std::unique_ptr<RHI::ShaderModule> Context::CreateShaderModule(RHI::TL::Span<uint8_t> code)
{
    return {};
}

std::unique_ptr<RHI::ShaderBindGroup> Context::CreateShaderBindGroup(const RHI::ShaderBindGroupLayout& layout)
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

RHI::Handle<RHI::GraphicsPipeline> Context::CreateGraphicsPipeline(const RHI::GraphicsPipelineCreateInfo& createInfo)
{
    return {};
}

RHI::Handle<RHI::ComputePipeline> Context::CreateComputePipeline(const RHI::ComputePipelineCreateInfo& createInfo)
{
    return {};
}

RHI::Handle<RHI::Sampler> Context::CreateSampler(const RHI::SamplerCreateInfo& createInfo)
{
    return {};
}

std::unique_ptr<RHI::FrameScheduler> Context::CreateFrameScheduler()
{
    return {};
}

void Context::Free(RHI::Handle<RHI::GraphicsPipeline> pso)
{
}

void Context::Free(RHI::Handle<RHI::ComputePipeline> pso)
{
}

void Context::Free(RHI::Handle<RHI::Sampler> pso)
{
}

}  // namespace Vulkan