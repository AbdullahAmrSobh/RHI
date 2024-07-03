#include "RHI/Context.hpp"
#include "RHI/CommandList.hpp"
#include "RHI/RenderGraph.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Swapchain.hpp"

#include "RHI/Common/Assert.hpp"
#include "RHI/Common/Callstack.hpp"

#include <tracy/Tracy.hpp>

namespace RHI
{
    Context::Context(Ptr<DebugCallbacks> debugCallbacks)
        : m_limits(CreatePtr<Limits>())
        , m_debugCallbacks(std::move(debugCallbacks))
    {
    }

    Context::~Context() = default;

    void Context::Shutdown()
    {
    }

    Limits Context::GetLimits() const
    {
        return *m_limits;
    }

    Ptr<RenderGraph> Context::CreateRenderGraph()
    {
        ZoneScoped;

        return CreatePtr<RenderGraph>(this);
    }

    void Context::CompileRenderGraph(RenderGraph& renderGraph)
    {
        ZoneScoped;

        (void)renderGraph;

        RHI_ASSERT(false); // TODO: Implement this function.
    }

    void Context::ExecuteRenderGraph(RenderGraph& renderGraph, Fence* signalFence)
    {
        ZoneScoped;

        Internal_DispatchGraph(renderGraph, signalFence);
    }

    Ptr<Swapchain> Context::CreateSwapchain(const SwapchainCreateInfo& createInfo)
    {
        ZoneScoped;

        RHI_ASSERT(createInfo.imageSize.width > 0 && createInfo.imageSize.height > 0);
        RHI_ASSERT(createInfo.imageCount >= Swapchain::MinImageCount);
        RHI_ASSERT(createInfo.imageCount <= Swapchain::MaxImageCount);
        RHI_ASSERT(createInfo.imageFormat != Format::Unknown);
        RHI_ASSERT(createInfo.imageUsage != ImageUsage::None);

        return Internal_CreateSwapchain(createInfo);
    }

    Ptr<ShaderModule> Context::CreateShaderModule(TL::Span<const uint32_t> shaderBlob)
    {
        ZoneScoped;

        return Internal_CreateShaderModule(shaderBlob);
    }

    Ptr<Fence> Context::CreateFence()
    {
        ZoneScoped;

        return Internal_CreateFence();
    }

    Ptr<CommandEncoder> Context::CreateCommandEncoder()
    {
        ZoneScoped;

        return Internal_CreateCommandEncoder();
    }

    Handle<BindGroupLayout> Context::CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreateBindGroupLayout(createInfo);
    }

    void Context::DestroyBindGroupLayout(Handle<BindGroupLayout> handle)
    {
        ZoneScoped;

        Internal_DestroyBindGroupLayout(handle);
    }

    Handle<BindGroup> Context::CreateBindGroup(Handle<BindGroupLayout> bindGroupLayoutHandle, uint32_t bindlessElementsCount)
    {
        ZoneScoped;

        return Internal_CreateBindGroup(bindGroupLayoutHandle, bindlessElementsCount);
    }

    void Context::DestroyBindGroup(Handle<BindGroup> handle)
    {
        ZoneScoped;

        Internal_DestroyBindGroup(handle);
    }

    void Context::UpdateBindGroup(Handle<BindGroup> handle, TL::Span<const ResourceBinding> bindings)
    {
        ZoneScoped;

        Internal_UpdateBindGroup(handle, bindings);
    }

    Handle<PipelineLayout> Context::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreatePipelineLayout(createInfo);
    }

    void Context::DestroyPipelineLayout(Handle<PipelineLayout> handle)
    {
        ZoneScoped;

        Internal_DestroyPipelineLayout(handle);
    }

    Handle<GraphicsPipeline> Context::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreateGraphicsPipeline(createInfo);
    }

    void Context::DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)
    {
        ZoneScoped;

        Internal_DestroyGraphicsPipeline(handle);
    }

    Handle<ComputePipeline> Context::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreateComputePipeline(createInfo);
    }

    void Context::DestroyComputePipeline(Handle<ComputePipeline> handle)
    {
        ZoneScoped;

        Internal_DestroyComputePipeline(handle);
    }

    Handle<Sampler> Context::CreateSampler(const SamplerCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreateSampler(createInfo);
    }

    void Context::DestroySampler(Handle<Sampler> handle)
    {
        ZoneScoped;

        Internal_DestroySampler(handle);
    }

    Result<Handle<Image>> Context::CreateImage(const ImageCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreateImage(createInfo);
    }

    void Context::DestroyImage(Handle<Image> handle)
    {
        ZoneScoped;

        Internal_DestroyImage(handle);
    }

    Result<Handle<Buffer>> Context::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreateBuffer(createInfo);
    }

    void Context::DestroyBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;

        Internal_DestroyBuffer(handle);
    }

    Handle<ImageView> Context::CreateImageView(const ImageViewCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreateImageView(createInfo);
    }

    void Context::DestroyImageView(Handle<ImageView> handle)
    {
        ZoneScoped;

        Internal_DestroyImageView(handle);
    }

    Handle<BufferView> Context::CreateBufferView(const BufferViewCreateInfo& createInfo)
    {
        ZoneScoped;

        return Internal_CreateBufferView(createInfo);
    }

    void Context::DestroyBufferView(Handle<BufferView> handle)
    {
        ZoneScoped;

        Internal_DestroyBufferView(handle);
    }

    DeviceMemoryPtr Context::MapBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;

        return Internal_MapBuffer(handle);
    }

    void Context::UnmapBuffer(Handle<Buffer> handle)
    {
        ZoneScoped;

        Internal_UnmapBuffer(handle);
    }

    StagingBuffer Context::AllocateTempBuffer(size_t size)
    {
        ZoneScoped;

        BufferCreateInfo createInfo{};
        createInfo.byteSize = size;
        createInfo.usageFlags = BufferUsage::CopySrc | BufferUsage::CopyDst;
        auto buffer = CreateBuffer(createInfo).GetValue();
        auto ptr = MapBuffer(buffer);
        m_stagingBuffers.push_back(buffer);
        return StagingBuffer{ ptr, buffer, 0 };
    }

    void Context::DebugLogError(std::string_view message)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (m_debugCallbacks)
            m_debugCallbacks->LogError(message);
#else
        (void)message;
#endif
    }

    void Context::DebugLogWarn(std::string_view message)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (m_debugCallbacks)
            m_debugCallbacks->LogWarnning(message);
#else
        (void)message;
#endif
    }

    void Context::DebugLogInfo(std::string_view message)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (m_debugCallbacks)
            m_debugCallbacks->LogInfo(message);
#else
        (void)message;
#endif
    }
} // namespace RHI