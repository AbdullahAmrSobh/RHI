#pragma once

#include "RHI/Export.hpp"
#include "RHI/Handle.hpp"
#include "RHI/Resources.hpp"
#include "RHI/CommandList.hpp"
#include "RHI/Swapchain.hpp"

#include <TL/Span.hpp>
#include <TL/UniquePtr.hpp>

namespace RHI
{
    enum class BackendType
    {
        Vulkan1_3,
        DirectX12_2,
        WebGPU,
    };

    struct RenderGraphCreateInfo;

    struct Version
    {
        uint16_t major = 0;
        uint16_t minor = 0;
        uint32_t patch = 0;
    };

    struct ApplicationInfo
    {
        const char* applicationName    = nullptr; // The name of the users application.
        Version     applicationVersion = {};      // The version of the users application.
        const char* engineName         = nullptr; // The version of the users application.
        Version     engineVersion      = {};      // The version of the users application.
    };

    struct DeviceLimits
    {
        // TODO: remove defaults
        uint32_t minUniformBufferOffsetAlignment = 256;
    };

    struct QueueWaitInfo
    {
        QueueType     queueType;
        uint64_t      timelineValue;
        PipelineStage waitStage;
    };

    struct QueueSubmitInfo
    {
        QueueType                     queueType            = QueueType::Graphics;
        TL::Span<CommandList* const>  commandLists         = {};
        PipelineStage                 signalStage          = {};
        TL::Span<const QueueWaitInfo> waitInfos            = {};
        Swapchain*                    m_swapchainToAcquire = nullptr;
        Swapchain*                    m_swapchainToSignal  = nullptr;
    };

    class RHI_EXPORT Device
    {
    public:
        RHI_INTERFACE_BOILERPLATE(Device);

        /// @brief Get the device limits.
        /// @return Device Limits struct.
        DeviceLimits                     GetLimits() const;

        RenderGraph*                     CreateRenderGraph(const RenderGraphCreateInfo& createInfo);

        void                             DestroyRenderGraph(RenderGraph* renderGraph);

        /// @brief Creates a swapchain.
        /// @param createInfo Swapchain creation parameters.
        /// @return Pointer to the created swapchain.
        virtual Swapchain*               CreateSwapchain(const SwapchainCreateInfo& createInfo)                                                                           = 0;

        /// @brief Destroys a swapchain.
        /// @param swapchain Pointer to the swapchain to destroy.
        virtual void                     DestroySwapchain(Swapchain* swapchain)                                                                                           = 0;

        /// @brief Creates a shader module.
        /// @param createInfo Shader module creation parameters.
        /// @return Pointer to the created shader module.
        virtual ShaderModule*            CreateShaderModule(const ShaderModuleCreateInfo& createInfo)                                                                     = 0;

        /// @brief Destroys a shader module object and frees associated resources
        /// @param shaderModule Pointer to the shader module to destroy. Must not be null and must be a valid shader module created by this
        /// device
        virtual void                     DestroyShaderModule(ShaderModule* shaderModule)                                                                                  = 0;

        /// @brief Creates a command list.
        /// @param createInfo Command list creation parameters.
        /// @return Pointer to the created command list.
        virtual CommandList*             CreateCommandList(const CommandListCreateInfo& createInfo)                                                                       = 0;

        /// @brief Creates a bind group layout.
        /// @param createInfo Bind group layout creation parameters.
        /// @return Handle to the created bind group layout.
        virtual Handle<BindGroupLayout>  CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)                                                               = 0;

        /// @brief Destroys a bind group layout.
        /// @param handle Handle to the bind group layout to destroy.
        virtual void                     DestroyBindGroupLayout(Handle<BindGroupLayout> handle)                                                                           = 0;

        /// @brief Creates a bind group.
        /// @param handle Handle to the associated bind group layout.
        /// @return Handle to the created bind group.
        virtual Handle<BindGroup>        CreateBindGroup(const BindGroupCreateInfo& createInfo)                                                                           = 0;

        /// @brief Destroys a bind group.
        /// @param handle Handle to the bind group to destroy.
        virtual void                     DestroyBindGroup(Handle<BindGroup> handle)                                                                                       = 0;

        /// @brief Updates a bind group.
        /// @param handle Handle to the bind group to update.
        /// @param updateInfo Update parameters for the bind group.
        virtual void                     UpdateBindGroup(Handle<BindGroup> handle, const BindGroupUpdateInfo& updateInfo)                                                 = 0;

        /// @brief Creates a buffer.
        /// @param createInfo Buffer creation parameters.
        /// @return Result containing the handle to the created buffer.
        virtual Result<Handle<Buffer>>   CreateBuffer(const BufferCreateInfo& createInfo)                                                                                 = 0;

        /// @brief Destroys a buffer.
        /// @param handle Handle to the buffer to destroy.
        virtual void                     DestroyBuffer(Handle<Buffer> handle)                                                                                             = 0;

        /// @brief Creates an image.
        /// @param createInfo Image creation parameters.
        /// @return Result containing the handle to the created image.
        virtual Result<Handle<Image>>    CreateImage(const ImageCreateInfo& createInfo)                                                                                   = 0;

        virtual Handle<Image>            CreateImageView(const ImageViewCreateInfo& createInfo)                                                                           = 0;

        /// @brief Creates a sampler.
        /// @param createInfo Sampler creation parameters.
        /// @return Handle to the created sampler.
        virtual Handle<Sampler>          CreateSampler(const SamplerCreateInfo& createInfo)                                                                               = 0;

        /// @brief Destroys a sampler.
        /// @param handle Handle to the sampler to destroy.
        virtual void                     DestroySampler(Handle<Sampler> handle)                                                                                           = 0;

        /// @brief Creates a pipeline layout.
        /// @param createInfo Pipeline layout creation parameters.
        /// @return Handle to the created pipeline layout.
        virtual Handle<PipelineLayout>   CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)                                                                 = 0;

        /// @brief Destroys a pipeline layout.
        /// @param handle Handle to the pipeline layout to destroy.
        virtual void                     DestroyPipelineLayout(Handle<PipelineLayout> handle)                                                                             = 0;

        /// @brief Creates a graphics pipeline.
        /// @param createInfo Graphics pipeline creation parameters.
        /// @return Handle to the created graphics pipeline.
        virtual Handle<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)                                                             = 0;

        /// @brief Destroys a graphics pipeline.
        /// @param handle Handle to the graphics pipeline to destroy.
        virtual void                     DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)                                                                         = 0;

        /// @brief Creates a compute pipeline.
        /// @param createInfo Compute pipeline creation parameters.
        /// @return Handle to the created compute pipeline.
        virtual Handle<ComputePipeline>  CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)                                                               = 0;

        /// @brief Destroys a compute pipeline.
        /// @param handle Handle to the compute pipeline to destroy.
        virtual void                     DestroyComputePipeline(Handle<ComputePipeline> handle)                                                                           = 0;

        /// @brief Destroys an image.
        /// @param handle Handle to the image to destroy.
        virtual void                     DestroyImage(Handle<Image> handle)                                                                                               = 0;

        /// @brief A resource update scope, resources will
        virtual void                     BeginResourceUpdate(RenderGraph* renderGraph)                                                                                    = 0;

        /// @brief Ends the resource update scope, resources will be flushed to the GPU.
        virtual void                     EndResourceUpdate()                                                                                                              = 0;

        /// @brief Writes data to a buffer.
        virtual void                     BufferWrite(Handle<Buffer> buffer, size_t offset, TL::Block block)                                                               = 0;

        /// @brief Writes data to an image.
        virtual void                     ImageWrite(Handle<Image> image, ImageOffset3D offset, ImageSize3D size, uint32_t mipLevel, uint32_t arrayLayer, TL::Block block) = 0;

        virtual uint64_t                 QueueSubmit(const QueueSubmitInfo& submitInfo)                                                                                   = 0;

    protected:
        BackendType           m_backend;
        TL::Ptr<DeviceLimits> m_limits; ///< Device-specific limits.
    };

    RHI_EXPORT Result<Handle<Image>> CreateImageWithContent(Device& device, const ImageCreateInfo& createInfo, TL::Block content);
} // namespace RHI