#pragma once

#include <RHI/Context.hpp>
#include <RHI/Handle.hpp>

namespace Vulkan
{

class Context final : public RHI::Context
{
public:
    ~Context();

    std::unique_ptr<RHI::ShaderModule> CreateShaderModule(RHI::TL::Span<uint32_t> code) override;

    std::unique_ptr<RHI::ShaderBindGroupAllocator> CreateShaderBindGroupAllocator() override;

    std::unique_ptr<RHI::FrameScheduler> CreateFrameScheduler() override;

    std::unique_ptr<RHI::Swapchain> CreateSwapchain(const RHI::SwapchainCreateInfo& createInfo) override;

    std::unique_ptr<RHI::ResourcePool> CreateResourcePool(const RHI::ResourcePoolCreateInfo& createInfo) override;

    RHI::Handle<RHI::GraphicsPipeline> CreateGraphicsPipeline(const RHI::GraphicsPipelineStateCreateInfo& createInfo) override;

    RHI::Handle<RHI::ComputePipeline> CreateComputePipeline(const RHI::ComputePipelineStateCreateInfo& createInfo) override;

    RHI::Handle<RHI::SamplerState> CreateSampler(const RHI::SamplerStateCreateInfo& createInfo) override;

    void Free(RHI::Handle<RHI::GraphicsPipeline> pso) override;

    void Free(RHI::Handle<RHI::ComputePipeline> pso) override;

    void Free(RHI::Handle<RHI::SamplerState> pso) override;

    std::unique_ptr<RHI::Pass> CreatePass(const RHI::PassCreateInfo& createInfo) override;

public:
    /// Object pools. 
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