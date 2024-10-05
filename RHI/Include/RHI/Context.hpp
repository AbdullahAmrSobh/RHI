#pragma once

#include "RHI/Export.hpp"
#include "RHI/Handle.hpp"
#include "RHI/Result.hpp"
#include "RHI/RenderGraph.hpp"
#include "RHI/Swapchain.hpp"
#include "RHI/CommandPool.hpp"
#include "RHI/CommandList.hpp"
#include <RHI/Image.hpp>
#include <RHI/Fence.hpp>
#include <RHI/Pipeline.hpp>
#include <RHI/Sampler.hpp>
#include <RHI/Shader.hpp>
#include <RHI/Queue.hpp>

#include <TL/Span.hpp>
#include <TL/UniquePtr.hpp>

namespace RHI
{
    class ResourceTracker;

    using DeviceMemoryPtr = void*;

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
        // General resource limits
        uint32_t                  maxImage2DSize;        ///< Maximum size of 2D images
        uint32_t                  maxImageCubeSize;      ///< Maximum size of cube images
        uint32_t                  maxImage3DSize;        ///< Maximum size of 3D images
        uint32_t                  maxImageArrayLayers;   ///< Maximum number of layers in an image array
        uint32_t                  maxUniformBufferSize;  ///< Maximum size of uniform buffers
        uint32_t                  maxStorageBufferSize;  ///< Maximum size of storage buffers
        uint32_t                  maxConstantBufferSize; ///< Maximum size of constant buffers
        // Bind group limits (shader resource binding limits)
        uint32_t                  maxBindGroups;            ///< Maximum number of bind groups in a pipeline
        uint32_t                  maxResourcesPerBindGroup; ///< Maximum number of resources per bind group (images, buffers, etc.)
        uint32_t                  maxSamplersPerBindGroup;  ///< Maximum number of samplers in a bind group
        uint32_t                  maxImagesPerBindGroup;    ///< Maximum number of images in a bind group
        uint32_t                  maxBuffersPerBindGroup;   ///< Maximum number of buffers in a bind group
        // Descriptor indexing limits
        uint32_t                  maxDynamicBuffers;    ///< Maximum number of dynamic buffers that can be bound
        uint32_t                  maxDescriptorsPerSet; ///< Maximum number of descriptors per bind group
        // Buffer alignment requirements
        uint32_t                  uniformBufferAlignment; ///< Required alignment for uniform buffers
        uint32_t                  storageBufferAlignment; ///< Required alignment for storage buffers
        uint32_t                  dynamicBufferAlignment; ///< Required alignment for dynamic uniform/storage buffers
        // Memory allocation limits
        static constexpr uint32_t maxMemoryHeaps = 16;            ///< Maximum number of memory heaps (set to a constant)
        uint64_t                  maxMemoryAllocationSize;        ///< Maximum size of a single memory allocation
        uint32_t                  memoryTypeCount;                ///< Number of different memory types available
        uint64_t                  memoryHeapSize[maxMemoryHeaps]; ///< Total size of each memory heap (for each memory type)
    };

    class RHI_EXPORT Context
    {
    public:
        Context();
        virtual ~Context();

        TL_NODISCARD Limits                   GetLimits() const;

        TL_NODISCARD TL::Ptr<RenderGraph>     CreateRenderGraph();

        void                                  CompileRenderGraph(RenderGraph& renderGraph);

        TL_NODISCARD TL::Ptr<Swapchain>       CreateSwapchain(const SwapchainCreateInfo& createInfo);

        TL_NODISCARD TL::Ptr<ShaderModule>    CreateShaderModule(TL::Span<const uint32_t> shaderBlob);

        TL_NODISCARD TL::Ptr<Fence>           CreateFence();

        TL_NODISCARD TL::Ptr<CommandPool>     CreateCommandPool(CommandPoolFlags flags);

        TL_NODISCARD Handle<BindGroupLayout>  CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo);

        void                                  DestroyBindGroupLayout(Handle<BindGroupLayout> handle);

        TL_NODISCARD Handle<BindGroup>        CreateBindGroup(Handle<BindGroupLayout> handle);

        void                                  DestroyBindGroup(Handle<BindGroup> handle);

        void                                  UpdateBindGroup(Handle<BindGroup> handle, const BindGroupUpdateInfo& updateInfo);

        TL_NODISCARD Handle<PipelineLayout>   CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo);

        void                                  DestroyPipelineLayout(Handle<PipelineLayout> handle);

        TL_NODISCARD Handle<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo);

        void                                  DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle);

        TL_NODISCARD Handle<ComputePipeline>  CreateComputePipeline(const ComputePipelineCreateInfo& createInfo);

        void                                  DestroyComputePipeline(Handle<ComputePipeline> handle);

        TL_NODISCARD Handle<Sampler>          CreateSampler(const SamplerCreateInfo& createInfo);

        void                                  DestroySampler(Handle<Sampler> handle);

        TL_NODISCARD Result<Handle<Image>>    CreateImage(const ImageCreateInfo& createInfo);

        void                                  DestroyImage(Handle<Image> handle);

        TL_NODISCARD Result<Handle<Buffer>>   CreateBuffer(const BufferCreateInfo& createInfo);

        void                                  DestroyBuffer(Handle<Buffer> handle);

        TL_NODISCARD Handle<ImageView>        CreateImageView(const ImageViewCreateInfo& createInfo);

        void                                  DestroyImageView(Handle<ImageView> handle);

        TL_NODISCARD Handle<BufferView>       CreateBufferView(const BufferViewCreateInfo& createInfo);

        void                                  DestroyBufferView(Handle<BufferView> handle);

        TL_NODISCARD DeviceMemoryPtr          MapBuffer(Handle<Buffer> handle);

        void                                  UnmapBuffer(Handle<Buffer> handle);

        TL_NODISCARD Handle<Semaphore>        CreateSemaphore(const SemaphoreCreateInfo& createInfo);

        void                                  DestroySemaphore(Handle<Semaphore> handle);

        TL_NODISCARD Queue*                   GetQueue(QueueType queueType);

        void                                  CollectResources();

    protected:
        virtual TL::Ptr<Swapchain>       Impl_CreateSwapchain(const SwapchainCreateInfo& createInfo)                           = 0;
        virtual TL::Ptr<ShaderModule>    Impl_CreateShaderModule(TL::Span<const uint32_t> shaderBlob)                          = 0;
        virtual TL::Ptr<Fence>           Impl_CreateFence()                                                                    = 0;
        virtual TL::Ptr<CommandPool>     Impl_CreateCommandPool(CommandPoolFlags flags)                                        = 0;
        virtual Handle<BindGroupLayout>  Impl_CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)               = 0;
        virtual void                     Impl_DestroyBindGroupLayout(Handle<BindGroupLayout> handle)                           = 0;
        virtual Handle<BindGroup>        Impl_CreateBindGroup(Handle<BindGroupLayout> handle)                                  = 0;
        virtual void                     Impl_DestroyBindGroup(Handle<BindGroup> handle)                                       = 0;
        virtual void                     Impl_UpdateBindGroup(Handle<BindGroup> handle, const BindGroupUpdateInfo& updateInfo) = 0;
        virtual Handle<PipelineLayout>   Impl_CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)                 = 0;
        virtual void                     Impl_DestroyPipelineLayout(Handle<PipelineLayout> handle)                             = 0;
        virtual Handle<GraphicsPipeline> Impl_CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)             = 0;
        virtual void                     Impl_DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)                         = 0;
        virtual Handle<ComputePipeline>  Impl_CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)               = 0;
        virtual void                     Impl_DestroyComputePipeline(Handle<ComputePipeline> handle)                           = 0;
        virtual Handle<Sampler>          Impl_CreateSampler(const SamplerCreateInfo& createInfo)                               = 0;
        virtual void                     Impl_DestroySampler(Handle<Sampler> handle)                                           = 0;
        virtual Result<Handle<Image>>    Impl_CreateImage(const ImageCreateInfo& createInfo)                                   = 0;
        virtual void                     Impl_DestroyImage(Handle<Image> handle)                                               = 0;
        virtual Result<Handle<Buffer>>   Impl_CreateBuffer(const BufferCreateInfo& createInfo)                                 = 0;
        virtual void                     Impl_DestroyBuffer(Handle<Buffer> handle)                                             = 0;
        virtual Handle<ImageView>        Impl_CreateImageView(const ImageViewCreateInfo& createInfo)                           = 0;
        virtual void                     Impl_DestroyImageView(Handle<ImageView> handle)                                       = 0;
        virtual Handle<BufferView>       Impl_CreateBufferView(const BufferViewCreateInfo& createInfo)                         = 0;
        virtual void                     Impl_DestroyBufferView(Handle<BufferView> handle)                                     = 0;
        virtual DeviceMemoryPtr          Impl_MapBuffer(Handle<Buffer> handle)                                                 = 0;
        virtual void                     Impl_UnmapBuffer(Handle<Buffer> handle)                                               = 0;
        virtual Handle<Semaphore>        Impl_CreateSemaphore(const SemaphoreCreateInfo& createInfo)                           = 0;
        virtual void                     Impl_DestroySemaphore(Handle<Semaphore> handle)                                       = 0;
        virtual Queue*                   Impl_GetQueue(QueueType queueType)                                                    = 0;
        virtual void                     Impl_CollectResources()                                                               = 0;

    protected:
        TL::Ptr<Limits> m_limits;
    };
} // namespace RHI