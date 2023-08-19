#pragma once

#include <RHI/Context.hpp>
#include <RHI/Handle.hpp>

namespace Vulkan
{

class Context final : public RHI::Context
{
public:
    ~Context();

    RHI::Handle<RHI::Pass> CreatePass(const RHI::PassCreateInfo& createInfo) override;

    std::unique_ptr<RHI::FrameScheduler> CreateFrameScheduler() override;

    std::unique_ptr<RHI::Swapchain> CreateSwapchain(const RHI::SwapchainCreateInfo& createInfo) override;

    std::unique_ptr<RHI::ResourcePool> CreateResourcePool(const RHI::ResourcePoolCreateInfo& createInfo) override;

    std::unique_ptr<RHI::PipelineStateCache> CreatePipelineStateCache(const RHI::PipelineStateCacheCreateInfo& createInfo) override;

    RHI::Handle<RHI::PipelineState> CreateGraphicsPipeline(const RHI::GraphicsPipelineCreateInfo& createInfo) override;
    RHI::Handle<RHI::PipelineState> CreateComputePipeline(const RHI::ComputePipelineCreateInfo& createInfo) override;
    RHI::Handle<RHI::PipelineState> CreateRayTracingPipeline(const RHI::RayTracingPipelineCreateInfo& createInfo) override;

    RHI::Handle<RHI::Sampler> CreateSampler(const RHI::SamplerCreateInfo& createInfo) override;

    void Free(RHI::Handle<RHI::Pass> pass) override;
    void Free(RHI::Handle<RHI::PipelineState> pso) override;
    void Free(RHI::Handle<RHI::Sampler> sampler) override;

public:
    // /// @brief Pool for all image resources.
    // RHI::HandlePool<struct RHI::Image, struct RHI::ImageDescriptor> m_imagesPool;

    // /// @brief Pool for all buffer resources.
    // RHI::HandlePool<struct RHI::Buffer, struct RHI::BufferDescriptor> m_buffersPool;

    // /// @brief Pool for all pipeline state resources.
    // RHI::HandlePool<struct RHI::PipelineState, struct RHI::PipelineStateDescriptor> m_pipelineStatesPool;

    // /// @brief Pool for all sampelr resources.
    // RHI::HandlePool<struct RHI::Sampler, struct RHI::SamplerDescriptor> m_samplersPool;

    // /// @brief Pool for all image views objects.
    // RHI::HandlePool<struct ImageView, struct ImageViewDescriptor> m_ImageViewsPool;

    // /// @brief Pool for all buffer views objects.
    // RHI::HandlePool<struct BufferView, struct BufferViewDescriptor> m_BufferViewsPool;
};

}  // namespace Vulkan