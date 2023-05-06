#include "RHI/Context.hpp"

#include "RHI/Backend/Vulkan/Context.hpp"
#include "RHI/Backend/Vulkan/FrameScheduler.hpp"
#include "RHI/Backend/Vulkan/Pipeline.hpp"
#include "RHI/Backend/Vulkan/Resources.hpp"
#include "RHI/Backend/Vulkan/ShaderResourceGroup.hpp"
#include "RHI/Debug.hpp"

namespace RHI
{

void Debug::Init(std::unique_ptr<DebugCallbacks> callbacks)
{
    Debug::s_callbacks = std::move(callbacks);
}

DebugCallbacks& Debug::Get()
{
    return *Debug::s_callbacks;
}

std::unique_ptr<Context> Context::Create(const ApplicationInfo& appInfo, std::unique_ptr<DebugCallbacks> debugCallbacks, Backend backend)
{
    if (debugCallbacks)
        Debug::Init(std::move(debugCallbacks));

    std::unique_ptr<Context> context;
    switch (backend)
    case Backend::Vulkan: context = std::make_unique<Vulkan::Context>(); {
    }

    context->Init(appInfo);

    RHI_ASSERT_MSG(context->Init(appInfo) == ResultCode::Success, "Failed to initalize context");

    return std::move(context);
}

std::unique_ptr<ShaderResourceGroupAllocator> Context::CreateShaderResourceGroupAllocator()
{
    std::unique_ptr<Vulkan::ShaderResourceGroupAllocator> shaderResourceGroupAllocator =
        std::make_unique<Vulkan::ShaderResourceGroupAllocator>(*this);
    RHI_ASSERT_MSG(shaderResourceGroupAllocator->Init() == ResultCode::Success, "Failed to create object");
    return std::move(shaderResourceGroupAllocator);
}

std::unique_ptr<Image> Context::CreateImage(const ResourceAllocationInfo& allocationInfo, const ImageCreateInfo& createInfo)
{
    std::unique_ptr<Vulkan::Image> image = std::make_unique<Vulkan::Image>(*this);
    RHI_ASSERT_MSG(image->Init(allocationInfo, createInfo) == ResultCode::Success, "Failed to create object");
    return std::move(image);
}

std::unique_ptr<Buffer> Context::CreateBuffer(const ResourceAllocationInfo& allocationInfo, const BufferCreateInfo& createInfo)
{
    std::unique_ptr<Vulkan::Buffer> buffer = std::make_unique<Vulkan::Buffer>(*this);
    RHI_ASSERT_MSG(buffer->Init(allocationInfo, createInfo) == ResultCode::Success, "Failed to create object");
    return std::move(buffer);
}

std::unique_ptr<Swapchain> Context::CreateSwapchain(const SwapchainCreateInfo& createInfo)
{
    std::unique_ptr<Vulkan::Swapchain> swapchain = std::make_unique<Vulkan::Swapchain>(*this);
    RHI_ASSERT_MSG(swapchain->Init(createInfo) == ResultCode::Success, "Failed to create object");
    return std::move(swapchain);
}

std::unique_ptr<Fence> Context::CreateFence()
{
    std::unique_ptr<Vulkan::Fence> fence = std::make_unique<Vulkan::Fence>(*this);
    RHI_ASSERT_MSG(fence->Init() == ResultCode::Success, "Failed to create object");
    return std::move(fence);
}

std::unique_ptr<Sampler> Context::CreateSampler(const SamplerCreateInfo& createInfo)
{
    std::unique_ptr<Vulkan::Sampler> sampler = std::make_unique<Vulkan::Sampler>(*this);
    RHI_ASSERT_MSG(sampler->Init(createInfo) == ResultCode::Success, "Failed to create object");
    return std::move(sampler);
}

std::unique_ptr<FrameScheduler> Context::CreateFrameScheduler()
{
    std::unique_ptr<Vulkan::FrameScheduler> frameScheduler = std::make_unique<Vulkan::FrameScheduler>(*this);
    RHI_ASSERT_MSG(frameScheduler->Init() == ResultCode::Success, "Failed to create object");
    return std::move(frameScheduler);
}

std::unique_ptr<PipelineState> Context::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
{
    std::unique_ptr<Vulkan::PipelineState> pipelineState = std::make_unique<Vulkan::PipelineState>(*this);
    RHI_ASSERT_MSG(pipelineState->Init(createInfo) == ResultCode::Success, "Failed to create object");
    return std::move(pipelineState);
}

std::unique_ptr<PipelineState> Context::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
{
    std::unique_ptr<Vulkan::PipelineState> pipelineState = std::make_unique<Vulkan::PipelineState>(*this);
    RHI_ASSERT_MSG(pipelineState->Init(createInfo) == ResultCode::Success, "Failed to create object");
    return std::move(pipelineState);
}

std::unique_ptr<PipelineState> Context::CreateRaytraceingPipeline(const RayTracingPipelineCreateInfo& createInfo)
{
    std::unique_ptr<Vulkan::PipelineState> pipelineState = std::make_unique<Vulkan::PipelineState>(*this);
    RHI_ASSERT_MSG(pipelineState->Init(createInfo) == ResultCode::Success, "Failed to create object");
    return std::move(pipelineState);
}

}  // namespace RHI