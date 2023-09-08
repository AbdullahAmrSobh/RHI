#pragma once

#include <RHI/Context.hpp>
#include <RHI/Handle.hpp>
#include <vulkan/vulkan.h>

namespace Vulkan
{

class Context final : public RHI::Context
{
public:
    Context() = default;
    ~Context();

    static Context* Create(const RHI::ApplicationInfo&          appInfo,
                           RHI::DeviceSelectionCallback         deviceSelectionCallbacks,
                           std::unique_ptr<RHI::DebugCallbacks> debugCallbacks);

    std::unique_ptr<RHI::ShaderModule> CreateShaderModule(RHI::TL::Span<uint8_t> code) override;

    std::unique_ptr<RHI::Swapchain> CreateSwapchain(const RHI::SwapchainCreateInfo& createInfo) override;

    std::unique_ptr<RHI::ResourcePool> CreateResourcePool(const RHI::ResourcePoolCreateInfo& createInfo) override;

    RHI::Handle<RHI::GraphicsPipeline> CreateGraphicsPipeline(const RHI::GraphicsPipelineCreateInfo& createInfo) override;

    RHI::Handle<RHI::ComputePipeline> CreateComputePipeline(const RHI::ComputePipelineCreateInfo& createInfo) override;

    RHI::Handle<RHI::Sampler> CreateSampler(const RHI::SamplerCreateInfo& createInfo) override;

    std::unique_ptr<RHI::FrameScheduler> CreateFrameScheduler() override;

    std::unique_ptr<RHI::ShaderBindGroupAllocator> CreateShaderBindGroupAllocator() override;

    void Free(RHI::Handle<RHI::GraphicsPipeline> pso) override;

    void Free(RHI::Handle<RHI::ComputePipeline> pso) override;

    void Free(RHI::Handle<RHI::Sampler> pso) override;
};

}  // namespace Vulkan