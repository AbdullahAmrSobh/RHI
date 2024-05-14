#pragma once

#include "RHI/Export.hpp"
#include "RHI/Common/Ptr.hpp"
#include "RHI/Common/Handle.hpp"
#include "RHI/Common/Result.hpp"
#include "RHI/Common/Debug.hpp"
#include "RHI/Common/Span.hpp"

#include <functional>

namespace RHI
{
    enum class CommandPoolFlags;

    struct ImageSubresourceLayers;

    struct BindGroupData;
    struct SwapchainCreateInfo;
    struct ResourcePoolCreateInfo;
    struct ImageCreateInfo;
    struct BufferCreateInfo;
    struct ImageViewCreateInfo;
    struct BufferViewCreateInfo;
    struct BindGroupLayoutCreateInfo;
    struct PipelineLayoutCreateInfo;
    struct GraphicsPipelineCreateInfo;
    struct ComputePipelineCreateInfo;
    struct SamplerCreateInfo;

    struct Image;
    struct Buffer;
    struct ImageView;
    struct BufferView;
    struct BindGroupLayout;
    struct BindGroup;
    struct PipelineLayout;
    struct GraphicsPipeline;
    struct ComputePipeline;
    struct Sampler;

    class Swapchain;
    class ShaderModule;
    class Fence;
    class CommandPool;
    class CommandList;
    class ResourcePool;
    class RenderGraph;

    class ResourceTracker;

    /// @brief Represents a pointer to GPU device memory
    using DeviceMemoryPtr = void*;

    /// @brief Type of backend Graphics API
    enum class Backend
    {
        Validate,
        Vulkan13,
        DirectX12,
    };

    /// @brief The type of the Physical GPU
    enum class DeviceType
    {
        CPU,
        Integerated,
        Dedicated,
        Virtual
    };

    // Identify the manufactuerer for the reported device
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

    /// @brief Describes information needed to initalize the RHI context
    struct ApplicationInfo
    {
        const char* applicationName;    // The name of the users application.
        Version     applicationVersion; // The version of the users application.
        const char* engineName;         // The version of the users application.
        Version     engineVersion;      // The version of the users application.
    };

    /// @brief Properties about a Physical GPU
    struct DeviceProperties
    {
        uint32_t    id;
        const char* name;
        DeviceType  type;
        Vendor      vendor;
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

        // clang-format off
        Limits                                GetLimits() const;

        RHI_NODISCARD Ptr<RenderGraph>        CreateRenderGraph();

        void                                  CompileRenderGraph(RenderGraph& renderGraph);

        void                                  ExecuteRenderGraph(RenderGraph& renderGraph, Fence* signalFence = nullptr);

        RHI_NODISCARD Ptr<Swapchain>          CreateSwapchain(const SwapchainCreateInfo& createInfo);

        RHI_NODISCARD Ptr<ShaderModule>       CreateShaderModule(TL::Span<const uint8_t> shaderBlob);

        RHI_NODISCARD Ptr<Fence>              CreateFence();

        RHI_NODISCARD Ptr<CommandPool>        CreateCommandPool(CommandPoolFlags flags);

        RHI_NODISCARD Ptr<ResourcePool>       CreateResourcePool(const ResourcePoolCreateInfo& createInfo);

        RHI_NODISCARD Handle<BindGroupLayout> CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo);

        void                                  DestroyBindGroupLayout(Handle<BindGroupLayout> handle);

        RHI_NODISCARD Handle<BindGroup>       CreateBindGroup(Handle<BindGroupLayout> handle);

        void                                  DestroyBindGroup(Handle<BindGroup> handle);

        void                                  UpdateBindGroup(Handle<BindGroup> handle, const BindGroupData& data);

        RHI_NODISCARD Handle<PipelineLayout>  CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo);

        void                                  DestroyPipelineLayout(Handle<PipelineLayout> handle);

        RHI_NODISCARD Handle<GraphicsPipeline>CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo);

        void                                  DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle);

        RHI_NODISCARD Handle<ComputePipeline> CreateComputePipeline(const ComputePipelineCreateInfo& createInfo);

        void                                  DestroyComputePipeline(Handle<ComputePipeline> handle);

        RHI_NODISCARD Handle<Sampler>         CreateSampler(const SamplerCreateInfo& createInfo);

        void                                  DestroySampler(Handle<Sampler> handle);

        RHI_NODISCARD Result<Handle<Image>>   CreateImage(const ImageCreateInfo& createInfo);

        void                                  DestroyImage(Handle<Image> handle);

        RHI_NODISCARD Result<Handle<Buffer>>  CreateBuffer(const BufferCreateInfo& createInfo);

        void                                  DestroyBuffer(Handle<Buffer> handle);

        RHI_NODISCARD Handle<ImageView>       CreateImageView(const ImageViewCreateInfo& createInfo);

        void                                  DestroyImageView(Handle<ImageView> handle);

        RHI_NODISCARD Handle<BufferView>      CreateBufferView(const BufferViewCreateInfo& createInfo);

        void                                  DestroyBufferView(Handle<BufferView> handle);

        RHI_NODISCARD DeviceMemoryPtr         MapBuffer(Handle<Buffer> handle);

        void                                  UnmapBuffer(Handle<Buffer> handle);

        void                                  DispatchGraph(RenderGraph& renderGraph);

        RHI_NODISCARD StagingBuffer           AllocateTempBuffer(size_t size);

        void                                  StageResourceWrite(Handle<Image> image, ImageSubresourceLayers subresources, Handle<Buffer> buffer, size_t bufferOffset);

        void                                  StageResourceWrite(Handle<Buffer> buffer, size_t offset, size_t size, Handle<Buffer> srcBuffer, size_t srcOffset);

        void                                  StageResourceRead(Handle<Image> image, ImageSubresourceLayers subresources, Handle<Buffer> buffer, size_t bufferOffset, Fence* fence);

        void                                  StageResourceRead(Handle<Buffer> buffer, size_t offset, size_t size, Handle<Buffer> srcBuffer, size_t srcOffset, Fence* fence);

        // clang-format on
    private:
        void Flush();

    protected:
        Context(Ptr<DebugCallbacks> debugCallbacks);

        void Shutdown();

        void DebugLogError(std::string_view message);
        void DebugLogWarn(std::string_view message);
        void DebugLogInfo(std::string_view message);

        void PushDeferCommand(std::function<void()> command);

        // clang-format off
        virtual Ptr<Swapchain>           Internal_CreateSwapchain(const SwapchainCreateInfo& createInfo) = 0;
        virtual Ptr<ShaderModule>        Internal_CreateShaderModule(TL::Span<const uint8_t> shaderBlob) = 0;
        virtual Ptr<Fence>               Internal_CreateFence() = 0;
        virtual Ptr<CommandPool>         Internal_CreateCommandPool(CommandPoolFlags flags) = 0;
        virtual Ptr<ResourcePool>        Internal_CreateResourcePool(const ResourcePoolCreateInfo& createInfo) = 0;
        virtual Handle<BindGroupLayout>  Internal_CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) = 0;
        virtual void                     Internal_DestroyBindGroupLayout(Handle<BindGroupLayout> handle) = 0;
        virtual Handle<BindGroup>        Internal_CreateBindGroup(Handle<BindGroupLayout> handle) = 0;
        virtual void                     Internal_DestroyBindGroup(Handle<BindGroup> handle) = 0;
        virtual void                     Internal_UpdateBindGroup(Handle<BindGroup> handle, const BindGroupData& data) = 0;
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
        Ptr<DebugCallbacks>        m_debugCallbacks;
        ResourceTracker*           m_resourceTracker;
        uint64_t                   m_frameIndex;

        TL::Vector<Handle<Buffer>> m_stagingBuffers;

        struct DeferCommand
        {
            uint64_t              frameIndex;
            std::function<void()> callback;
        };

        TL::Deque<DeferCommand> m_deferCommandQueue[2];
    };
} // namespace RHI
