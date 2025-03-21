#pragma once

#include "RHI/Export.hpp"
#include "RHI/Handle.hpp"
#include "RHI/Result.hpp"
#include "RHI/Swapchain.hpp"
#include "RHI/CommandList.hpp"
#include "RHI/Pipeline.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Queue.hpp"

#include <TL/Span.hpp>
#include <TL/UniquePtr.hpp>

namespace RHI
{
    using DeviceMemoryPtr = void*;
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

    struct StagingBuffer
    {
        DeviceMemoryPtr ptr    = nullptr;
        Handle<Buffer>  buffer = NullHandle;
        size_t          offset = 0;
        size_t          size   = 0;
    };

    struct ImageUploadInfo
    {
        Handle<Image>  image           = NullHandle;
        Handle<Buffer> srcBuffer       = NullHandle;
        size_t         srcBufferOffset = 0;
        size_t         sizeBytes       = 0;
        uint32_t       baseMipLevel    = 0;
        uint32_t       levelCount      = 1;
        uint32_t       baseArrayLayer  = 0;
        uint32_t       layerCount      = 1;
    };

    class RHI_EXPORT Device
    {
        friend Device* CreateVulkanDevice(const ApplicationInfo& appInfo);
        friend void    DestroyVulkanDevice(Device* device);

    public:
        RHI_INTERFACE_BOILERPLATE(Device);

        /// @brief Get the device limits.
        /// @return Device Limits struct.
        DeviceLimits                     GetLimits() const;

        virtual RenderGraph*             CreateRenderGraph(const RenderGraphCreateInfo& createInfo)                       = 0;

        virtual void                     DestroyRenderGraph(RenderGraph* renderGraph)                                     = 0;

        /// @brief Creates a swapchain.
        /// @param createInfo Swapchain creation parameters.
        /// @return Pointer to the created swapchain.
        virtual Swapchain*               CreateSwapchain(const SwapchainCreateInfo& createInfo)                           = 0;

        /// @brief Destroys a swapchain.
        /// @param swapchain Pointer to the swapchain to destroy.
        virtual void                     DestroySwapchain(Swapchain* swapchain)                                           = 0;

        /// @brief Creates a shader module.
        /// @param createInfo Shader module creation parameters.
        /// @return Pointer to the created shader module.
        virtual ShaderModule*            CreateShaderModule(const ShaderModuleCreateInfo& createInfo)                     = 0;

        /// @brief Destroys a shader module object and frees associated resources
        /// @param shaderModule Pointer to the shader module to destroy. Must not be null and must be a valid shader module created by this
        /// device
        virtual void                     DestroyShaderModule(ShaderModule* shaderModule)                                  = 0;

        /// @brief Creates a command list.
        /// @param createInfo Command list creation parameters.
        /// @return Pointer to the created command list.
        virtual CommandList*             CreateCommandList(const CommandListCreateInfo& createInfo)                       = 0;

        /// @brief Creates a bind group layout.
        /// @param createInfo Bind group layout creation parameters.
        /// @return Handle to the created bind group layout.
        virtual Handle<BindGroupLayout>  CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo)               = 0;

        /// @brief Destroys a bind group layout.
        /// @param handle Handle to the bind group layout to destroy.
        virtual void                     DestroyBindGroupLayout(Handle<BindGroupLayout> handle)                           = 0;

        /// @brief Creates a bind group.
        /// @param handle Handle to the associated bind group layout.
        /// @return Handle to the created bind group.
        virtual Handle<BindGroup>        CreateBindGroup(const BindGroupCreateInfo& createInfo)                           = 0;

        /// @brief Destroys a bind group.
        /// @param handle Handle to the bind group to destroy.
        virtual void                     DestroyBindGroup(Handle<BindGroup> handle)                                       = 0;

        /// @brief Updates a bind group.
        /// @param handle Handle to the bind group to update.
        /// @param updateInfo Update parameters for the bind group.
        virtual void                     UpdateBindGroup(Handle<BindGroup> handle, const BindGroupUpdateInfo& updateInfo) = 0;

        /// @brief Creates a pipeline layout.
        /// @param createInfo Pipeline layout creation parameters.
        /// @return Handle to the created pipeline layout.
        virtual Handle<PipelineLayout>   CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)                 = 0;

        /// @brief Destroys a pipeline layout.
        /// @param handle Handle to the pipeline layout to destroy.
        virtual void                     DestroyPipelineLayout(Handle<PipelineLayout> handle)                             = 0;

        /// @brief Creates a graphics pipeline.
        /// @param createInfo Graphics pipeline creation parameters.
        /// @return Handle to the created graphics pipeline.
        virtual Handle<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)             = 0;

        /// @brief Destroys a graphics pipeline.
        /// @param handle Handle to the graphics pipeline to destroy.
        virtual void                     DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle)                         = 0;

        /// @brief Creates a compute pipeline.
        /// @param createInfo Compute pipeline creation parameters.
        /// @return Handle to the created compute pipeline.
        virtual Handle<ComputePipeline>  CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)               = 0;

        /// @brief Destroys a compute pipeline.
        /// @param handle Handle to the compute pipeline to destroy.
        virtual void                     DestroyComputePipeline(Handle<ComputePipeline> handle)                           = 0;

        /// @brief Creates a sampler.
        /// @param createInfo Sampler creation parameters.
        /// @return Handle to the created sampler.
        virtual Handle<Sampler>          CreateSampler(const SamplerCreateInfo& createInfo)                               = 0;

        /// @brief Destroys a sampler.
        /// @param handle Handle to the sampler to destroy.
        virtual void                     DestroySampler(Handle<Sampler> handle)                                           = 0;

        /// @brief Creates an image.
        /// @param createInfo Image creation parameters.
        /// @return Result containing the handle to the created image.
        virtual Result<Handle<Image>>    CreateImage(const ImageCreateInfo& createInfo)                                   = 0;

        /// @brief Destroys an image.
        /// @param handle Handle to the image to destroy.
        virtual void                     DestroyImage(Handle<Image> handle)                                               = 0;

        /// @brief Creates a buffer.
        /// @param createInfo Buffer creation parameters.
        /// @return Result containing the handle to the created buffer.
        virtual Result<Handle<Buffer>>   CreateBuffer(const BufferCreateInfo& createInfo)                                 = 0;

        /// @brief Destroys a buffer.
        /// @param handle Handle to the buffer to destroy.
        virtual void                     DestroyBuffer(Handle<Buffer> handle)                                             = 0;

        /// @brief Maps a buffer to CPU memory.
        /// @param handle Handle to the buffer to map.
        /// @return Pointer to the mapped memory.
        virtual DeviceMemoryPtr          MapBuffer(Handle<Buffer> handle)                                                 = 0;

        /// @brief Unmaps a buffer from CPU memory.
        /// @param handle Handle to the buffer to unmap.
        virtual void                     UnmapBuffer(Handle<Buffer> handle)                                               = 0;

        /// @brief Allocates a staging buffer.
        /// @param size Buffer size in bytes.
        /// @return Staging buffer descriptor.
        virtual StagingBuffer            StagingAllocate(size_t size)                                                     = 0;

        /// @brief Uploads data to an image.
        /// @param uploadInfo Image upload parameters.
        /// @return Timeline semaphore value after upload.
        virtual uint64_t                 UploadImage(const ImageUploadInfo& uploadInfo)                                   = 0;

        /// @brief Collects unused resources for reuse.
        virtual void                     CollectResources()                                                               = 0;

        // clang-format off
        virtual bool                     WaitForQueueTimelineValue(QueueType queueType, uint64_t value, uint64_t waitDuration) = 0;
        // clang-format on

    protected:
        TL::Ptr<DeviceLimits> m_limits; ///< Device-specific limits.
    };

    RHI_EXPORT Result<Handle<Image>> CreateImageWithContent(Device& device, const ImageCreateInfo& createInfo, TL::Block content);
} // namespace RHI