#pragma once

#include "RHI/Export.hpp"
#include "RHI/Handle.hpp"
#include "RHI/Result.hpp"
#include "RHI/RenderGraph.hpp"
#include "RHI/Swapchain.hpp"
#include "RHI/CommandList.hpp"
#include <RHI/Image.hpp>
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

    struct DeviceLimits
    {

    };

    class RHI_EXPORT Device
    {
    public:
        Device();
        virtual ~Device();

        // clang-format off
        TL_NODISCARD DeviceLimits             GetLimits() const;

        TL_NODISCARD TL::Ptr<RenderGraph>     CreateRenderGraph();

        TL_NODISCARD TL::Ptr<Swapchain>       CreateSwapchain(const SwapchainCreateInfo& createInfo);

        TL_NODISCARD TL::Ptr<ShaderModule>    CreateShaderModule(TL::Span<const uint32_t> shaderBlob);

        TL_NODISCARD TL::Ptr<CommandList>     CreateCommandList(QueueType queueType);

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

        TL_NODISCARD DeviceMemoryPtr          MapBuffer(Handle<Buffer> handle);

        void                                  UnmapBuffer(Handle<Buffer> handle);

        TL_NODISCARD Queue*                   GetQueue(QueueType queueType);

        void                                  CollectResources();

        void                                  WaitTimelineValue(uint64_t value);
        // clang-format on

    protected:
        virtual TL::Ptr<Swapchain>       Impl_CreateSwapchain(const SwapchainCreateInfo& createInfo)                           = 0;
        virtual TL::Ptr<ShaderModule>    Impl_CreateShaderModule(TL::Span<const uint32_t> shaderBlob)                          = 0;
        virtual TL::Ptr<CommandList>     Impl_CreateCommandList(QueueType queueType)                                           = 0;
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
        virtual DeviceMemoryPtr          Impl_MapBuffer(Handle<Buffer> handle)                                                 = 0;
        virtual void                     Impl_UnmapBuffer(Handle<Buffer> handle)                                               = 0;
        virtual Queue*                   Impl_GetQueue(QueueType queueType)                                                    = 0;
        virtual void                     Impl_CollectResources()                                                               = 0;
        virtual void                     Impl_WaitTimelineValue(uint64_t value)                                                = 0;

    protected:
        TL::Ptr<DeviceLimits> m_limits;
    };
} // namespace RHI