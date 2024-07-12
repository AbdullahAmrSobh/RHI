#pragma once

#include "RHI/Export.hpp"
#include "RHI/Common/Ptr.hpp"
#include "RHI/Common/Handle.hpp"
#include "RHI/Common/Result.hpp"
#include "RHI/Common/Debug.hpp"
#include "RHI/Common/Span.hpp"

#include "RHI/Resources.hpp"
#include "RHI/RenderGraph.hpp"
#include "RHI/Swapchain.hpp"
#include "RHI/CommandList.hpp"

#include <functional>

namespace RHI
{
    class ResourceTracker;

    using DeviceMemoryPtr = void*;

    enum class Backend
    {
        Vulkan13,
    };

    enum class DeviceType
    {
        CPU,
        Integerated,
        Dedicated,
        Virtual
    };

    enum class Vendor
    {
        Intel,
        Nvida,
        AMD,
        Other,
    };

    struct Version
    {
        uint16_t major;
        uint16_t minor;
        uint32_t patch;
    };

    struct ApplicationInfo
    {
        const char* applicationName;    // The name of the users application.
        Version     applicationVersion; // The version of the users application.
        const char* engineName;         // The version of the users application.
        Version     engineVersion;      // The version of the users application.
    };

    struct Limits
    {
        size_t stagingMemoryLimit;
    };

    struct StagingBuffer
    {
        DeviceMemoryPtr ptr;
        Handle<Buffer>  buffer;
        size_t          offset;
    };

    class RHI_EXPORT Context
    {
    public:
        virtual ~Context() = default;

        Limits GetLimits() const;

        RHI_NODISCARD Ptr<RenderGraph> CreateRenderGraph();

        void CompileRenderGraph(RenderGraph& renderGraph);

        void ExecuteRenderGraph(RenderGraph& renderGraph, Fence* signalFence = nullptr);

        RHI_NODISCARD Ptr<Swapchain> CreateSwapchain(const SwapchainCreateInfo& createInfo);

        RHI_NODISCARD Ptr<ShaderModule> CreateShaderModule(TL::Span<const uint32_t> shaderBlob);

        RHI_NODISCARD Ptr<Fence> CreateFence();

        RHI_NODISCARD Ptr<CommandPool> CreateCommandPool(CommandPoolFlags flags);

        RHI_NODISCARD Handle<BindGroupLayout> CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo);

        void DestroyBindGroupLayout(Handle<BindGroupLayout> handle);

        RHI_NODISCARD Handle<BindGroup> CreateBindGroup(Handle<BindGroupLayout> handle, uint32_t bindlessElementsCount = UINT32_MAX);

        void DestroyBindGroup(Handle<BindGroup> handle);

        void UpdateBindGroup(Handle<BindGroup> handle, TL::Span<const ResourceBinding> bindings);

        RHI_NODISCARD Handle<PipelineLayout> CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo);

        void DestroyPipelineLayout(Handle<PipelineLayout> handle);

        RHI_NODISCARD Handle<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo);

        void DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle);

        RHI_NODISCARD Handle<ComputePipeline> CreateComputePipeline(const ComputePipelineCreateInfo& createInfo);

        void DestroyComputePipeline(Handle<ComputePipeline> handle);

        RHI_NODISCARD Handle<Sampler> CreateSampler(const SamplerCreateInfo& createInfo);

        void DestroySampler(Handle<Sampler> handle);

        RHI_NODISCARD Result<Handle<Image>> CreateImage(const ImageCreateInfo& createInfo);

        void DestroyImage(Handle<Image> handle);

        RHI_NODISCARD Result<Handle<Buffer>> CreateBuffer(const BufferCreateInfo& createInfo);

        void DestroyBuffer(Handle<Buffer> handle);

        RHI_NODISCARD Handle<ImageView> CreateImageView(const ImageViewCreateInfo& createInfo);

        void DestroyImageView(Handle<ImageView> handle);

        RHI_NODISCARD Handle<BufferView> CreateBufferView(const BufferViewCreateInfo& createInfo);

        void DestroyBufferView(Handle<BufferView> handle);

        RHI_NODISCARD DeviceMemoryPtr MapBuffer(Handle<Buffer> handle);

        void UnmapBuffer(Handle<Buffer> handle);

        void DispatchGraph(RenderGraph& renderGraph);

        RHI_NODISCARD StagingBuffer AllocateTempBuffer(size_t size);

        void StageResourceWrite(Handle<Image> image, ImageSubresourceLayers subresources, Handle<Buffer> buffer, size_t bufferOffset);

        void StageResourceWrite(Handle<Buffer> buffer, size_t offset, size_t size, Handle<Buffer> srcBuffer, size_t srcOffset);

        void StageResourceRead(Handle<Image> image, ImageSubresourceLayers subresources, Handle<Buffer> buffer, size_t bufferOffset, Fence* fence);

        void StageResourceRead(Handle<Buffer> buffer, size_t offset, size_t size, Handle<Buffer> srcBuffer, size_t srcOffset, Fence* fence);

    private:
        bool ValidateCreateInfo(const SwapchainCreateInfo& createInfo) const;
        bool ValidateCreateInfo(const BindGroupLayoutCreateInfo& createInfo) const;
        bool ValidateCreateInfo(const PipelineLayoutCreateInfo& createInfo) const;
        bool ValidateCreateInfo(const GraphicsPipelineCreateInfo& createInfo) const;
        bool ValidateCreateInfo(const ComputePipelineCreateInfo& createInfo) const;
        bool ValidateCreateInfo(const SamplerCreateInfo& createInfo) const;
        bool ValidateCreateInfo(const ImageCreateInfo& createInfo) const;
        bool ValidateCreateInfo(const BufferCreateInfo& createInfo) const;
        bool ValidateCreateInfo(const ImageViewCreateInfo& createInfo) const;
        bool ValidateCreateInfo(const BufferViewCreateInfo& createInfo) const;

    protected:
        Context(Ptr<DebugCallbacks> debugCallbacks);

        void Shutdown();

        void DebugLogError(std::string_view message) const;
        void DebugLogWarn(std::string_view message) const;
        void DebugLogInfo(std::string_view message) const;

        // clang-format off
        virtual Ptr<Swapchain>           Internal_CreateSwapchain(const SwapchainCreateInfo& createInfo) = 0;
        virtual Ptr<ShaderModule>        Internal_CreateShaderModule(TL::Span<const uint32_t> shaderBlob) = 0;
        virtual Ptr<Fence>               Internal_CreateFence() = 0;
        virtual Ptr<CommandPool>         Internal_CreateCommandPool(CommandPoolFlags flags) = 0;
        virtual Handle<BindGroupLayout>  Internal_CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroyBindGroupLayout(Handle<BindGroupLayout> handle) = 0;
        virtual Handle<BindGroup>        Internal_CreateBindGroup(Handle<BindGroupLayout> handle, uint32_t bindlessElementsCount) = 0;
        virtual void                     Internal_DestroyBindGroup(Handle<BindGroup> handle) = 0;
        virtual void                     Internal_UpdateBindGroup(Handle<BindGroup> handle, TL::Span<const ResourceBinding> bindings) = 0;
        virtual Handle<PipelineLayout>   Internal_CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroyPipelineLayout(Handle<PipelineLayout> handle) = 0;
        virtual Handle<GraphicsPipeline> Internal_CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle) = 0;
        virtual Handle<ComputePipeline>  Internal_CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroyComputePipeline(Handle<ComputePipeline> handle) = 0;
        virtual Handle<Sampler>          Internal_CreateSampler(const SamplerCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroySampler(Handle<Sampler> handle) = 0;
        virtual Result<Handle<Image>>    Internal_CreateImage(const ImageCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroyImage(Handle<Image> handle) = 0;
        virtual Result<Handle<Buffer>>   Internal_CreateBuffer(const BufferCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroyBuffer(Handle<Buffer> handle) = 0;
        virtual Handle<ImageView>        Internal_CreateImageView(const ImageViewCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroyImageView(Handle<ImageView> handle) = 0;
        virtual Handle<BufferView>       Internal_CreateBufferView(const BufferViewCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroyBufferView(Handle<BufferView> handle) = 0;
        virtual DeviceMemoryPtr          Internal_MapBuffer(Handle<Buffer> handle) = 0;
        virtual void                     Internal_DispatchGraph(RenderGraph& renderGraph, Fence* signalFence) = 0;
        virtual void                     Internal_UnmapBuffer(Handle<Buffer> handle) = 0;
        virtual void                     Internal_StageResourceWrite(Handle<Image> image, ImageSubresourceLayers subresources, Handle<Buffer> buffer, size_t bufferOffset) = 0;
        virtual void                     Internal_StageResourceWrite(Handle<Buffer> buffer, size_t offset, size_t size, Handle<Buffer> srcBuffer, size_t srcOffset) = 0;
        virtual void                     Internal_StageResourceRead(Handle<Image> image, ImageSubresourceLayers subresources, Handle<Buffer> buffer, size_t bufferOffset, Fence* fence) = 0;
        virtual void                     Internal_StageResourceRead(Handle<Buffer> buffer, size_t offset, size_t size, Handle<Buffer> srcBuffer, size_t srcOffset, Fence* fence) = 0;
        // clang-format on

    protected:
        Ptr<Limits> m_limits;

    private:
        mutable Ptr<DebugCallbacks> m_debugCallbacks;

        TL::Vector<Handle<Buffer>> m_stagingBuffers;
    };

    template<typename T>
    inline static Result<Handle<Image>> CreateImageWithData(Context& context, const ImageCreateInfo& createInfo, TL::Span<const T> content)
    {
        auto [handle, result] = context.CreateImage(createInfo);

        if (result != ResultCode::Success)
            return result;

        auto stagingBuffer = context.AllocateTempBuffer(content.size_bytes());
        memcpy(stagingBuffer.ptr, content.data(), content.size_bytes());

        ImageSubresourceLayers subresources{};
        subresources.imageAspects = ImageAspect::Color; // todo: this should be deduced from the format
        subresources.arrayCount   = createInfo.arrayCount;
        subresources.mipLevel     = createInfo.mipLevels;
        context.StageResourceWrite(handle, subresources, stagingBuffer.buffer, stagingBuffer.offset);

        return handle;
    }

    template<typename T>
    inline static Result<Handle<Buffer>> CreateBufferWithData(Context& context, Flags<BufferUsage> usageFlags, TL::Span<const T> content)
    {
        BufferCreateInfo createInfo{};
        createInfo.byteSize   = content.size_bytes();
        createInfo.usageFlags = usageFlags;

        auto [handle, result] = context.CreateBuffer(createInfo);

        if (result != ResultCode::Success)
            return result;

        if (content.size_bytes() <= context.GetLimits().stagingMemoryLimit)
        {
            auto ptr = context.MapBuffer(handle);
            memcpy(ptr, content.data(), content.size_bytes());
            context.UnmapBuffer(handle);
        }
        else
        {
            auto stagingBuffer = context.AllocateTempBuffer(content.size_bytes());
            memcpy(stagingBuffer.ptr, content.data(), content.size_bytes());
            context.StageResourceWrite(handle, 0, content.size_bytes(), stagingBuffer.buffer, stagingBuffer.offset);
        }

        return handle;
    }
} // namespace RHI