#pragma once

#include "RHI/Export.hpp"
#include "RHI/Handle.hpp"
#include "RHI/Result.hpp"
#include "RHI/RenderGraph.hpp"
#include "RHI/Swapchain.hpp"
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
        size_t stagingMemoryLimit;
        bool   unifiedMemory;
    };

    class RHI_EXPORT Context
    {
    public:
        Context();
        virtual ~Context();

        TL_NODISCARD Limits                   GetLimits() const;

        TL_NODISCARD TL::Ptr<RenderGraph>     CreateRenderGraph();

        void                                  CompileRenderGraph(RenderGraph& renderGraph);

        void                                  ExecuteRenderGraph(RenderGraph& renderGraph, Fence* signalFence = nullptr);

        TL_NODISCARD TL::Ptr<Swapchain>       CreateSwapchain(const SwapchainCreateInfo& createInfo);

        TL_NODISCARD TL::Ptr<ShaderModule>    CreateShaderModule(TL::Span<const uint32_t> shaderBlob);

        TL_NODISCARD TL::Ptr<Fence>           CreateFence();

        TL_NODISCARD TL::Ptr<CommandPool>     CreateCommandPool(CommandPoolFlags flags);

        TL_NODISCARD Handle<BindGroupLayout>  CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo);

        void                                  DestroyBindGroupLayout(Handle<BindGroupLayout> handle);

        TL_NODISCARD Handle<BindGroup>        CreateBindGroup(Handle<BindGroupLayout> handle, uint32_t bindlessElementsCount = UINT32_MAX);

        void                                  DestroyBindGroup(Handle<BindGroup> handle);

        void                                  UpdateBindGroup(Handle<BindGroup> handle, TL::Span<const BindGroupUpdateInfo> bindings);

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
        virtual TL::Ptr<Swapchain>       Internal_CreateSwapchain(const SwapchainCreateInfo& createInfo)                                  = 0;
        virtual TL::Ptr<ShaderModule>    Internal_CreateShaderModule(TL::Span<const uint32_t> shaderBlob)                                 = 0;
        virtual TL::Ptr<Fence>           Internal_CreateFence()                                                                           = 0;
        virtual TL::Ptr<CommandPool>     Internal_CreateCommandPool(CommandPoolFlags flags)                                               = 0;
        virtual Handle<BindGroupLayout>  Internal_CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)                      = 0;
        virtual void                     Internal_DestroyBindGroupLayout(Handle<BindGroupLayout> handle)                                  = 0;
        virtual Handle<BindGroup>        Internal_CreateBindGroup(Handle<BindGroupLayout> handle, uint32_t bindlessElementsCount)         = 0;
        virtual void                     Internal_DestroyBindGroup(Handle<BindGroup> handle)                                              = 0;
        virtual void                     Internal_UpdateBindGroup(Handle<BindGroup> handle, TL::Span<const BindGroupUpdateInfo> bindings) = 0;
        virtual Handle<PipelineLayout>   Internal_CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)                        = 0;
        virtual void                     Internal_DestroyPipelineLayout(Handle<PipelineLayout> handle)                                    = 0;
        virtual Handle<GraphicsPipeline> Internal_CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)                    = 0;
        virtual void                     Internal_DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)                                = 0;
        virtual Handle<ComputePipeline>  Internal_CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)                      = 0;
        virtual void                     Internal_DestroyComputePipeline(Handle<ComputePipeline> handle)                                  = 0;
        virtual Handle<Sampler>          Internal_CreateSampler(const SamplerCreateInfo& createInfo)                                      = 0;
        virtual void                     Internal_DestroySampler(Handle<Sampler> handle)                                                  = 0;
        virtual Result<Handle<Image>>    Internal_CreateImage(const ImageCreateInfo& createInfo)                                          = 0;
        virtual void                     Internal_DestroyImage(Handle<Image> handle)                                                      = 0;
        virtual Result<Handle<Buffer>>   Internal_CreateBuffer(const BufferCreateInfo& createInfo)                                        = 0;
        virtual void                     Internal_DestroyBuffer(Handle<Buffer> handle)                                                    = 0;
        virtual Handle<ImageView>        Internal_CreateImageView(const ImageViewCreateInfo& createInfo)                                  = 0;
        virtual void                     Internal_DestroyImageView(Handle<ImageView> handle)                                              = 0;
        virtual Handle<BufferView>       Internal_CreateBufferView(const BufferViewCreateInfo& createInfo)                                = 0;
        virtual void                     Internal_DestroyBufferView(Handle<BufferView> handle)                                            = 0;
        virtual DeviceMemoryPtr          Internal_MapBuffer(Handle<Buffer> handle)                                                        = 0;
        virtual void                     Internal_DispatchGraph(RenderGraph& renderGraph, Fence* signalFence)                             = 0;
        virtual void                     Internal_UnmapBuffer(Handle<Buffer> handle)                                                      = 0;

    protected:
        TL::Ptr<Limits> m_limits;
    };
} // namespace RHI