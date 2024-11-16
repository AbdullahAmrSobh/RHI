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
        uint32_t minDynamicUniformBufferAlignment = 256;
    };

    struct StagingBuffer
    {
        DeviceMemoryPtr ptr;
        Handle<Buffer>  buffer;
        size_t          offset;
        size_t          size;
    };

    struct ImageUploadInfo
    {
        Handle<Image>  image;
        Handle<Buffer> srcBuffer;
        size_t         srcBufferOffset;
        size_t         sizeBytes;
        uint32_t       baseMipLevel   = 0;
        uint32_t       levelCount     = 1;
        uint32_t       baseArrayLayer = 0;
        uint32_t       layerCount     = 1;
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

        StagingBuffer                         StagingAllocate(size_t size);

        uint64_t                              UploadImage(const ImageUploadInfo& uploadInfo);

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
        virtual StagingBuffer            Impl_StagingAllocate(size_t size)                                                     = 0;
        virtual uint64_t                 Impl_UploadImage(const ImageUploadInfo& uploadInfo)                                   = 0;
        virtual void                     Impl_CollectResources()                                                               = 0;
        virtual void                     Impl_WaitTimelineValue(uint64_t value)                                                = 0;

    protected:
        TL::Ptr<DeviceLimits> m_limits;
    };

    namespace Utils
    {
        inline static size_t CalcaulteImageSize(Format format, ImageSize3D size, uint32_t mipLevelsCount, uint32_t arrayCount)
        {
            size_t imageSizeBytes = 0;
            size_t formatByteSize = GetFormatByteSize(format);
            for (uint32_t mip = 0; mip < mipLevelsCount; ++mip)
            {
                uint32_t mipWidth  = std::max(1u, size.width >> mip);
                uint32_t mipHeight = std::max(1u, size.height >> mip);
                uint32_t mipDepth  = std::max(1u, size.depth >> mip);
                imageSizeBytes += mipWidth * mipHeight * mipDepth * formatByteSize;
            }
            imageSizeBytes *= arrayCount;
            return imageSizeBytes;
        }

        inline static Result<Handle<Image>> CreateImageWithContent(Device& device, const ImageCreateInfo& createInfo, TL::Block content)
        {
            auto imageSizeBytes = CalcaulteImageSize(createInfo.format, createInfo.size, createInfo.mipLevels, createInfo.arrayCount);

            TL_ASSERT(imageSizeBytes != 0);
            TL_ASSERT(imageSizeBytes == content.size);
            TL_ASSERT(createInfo.usageFlags & ImageUsage::CopyDst);

            if (auto [image, result] = device.CreateImage(createInfo); IsSuccess(result))
            {
                auto stagingBuffer = device.StagingAllocate(imageSizeBytes);
                memcpy(stagingBuffer.ptr, content.ptr, content.size);
                device.UploadImage({
                    .image           = image,
                    .srcBuffer       = stagingBuffer.buffer,
                    .srcBufferOffset = stagingBuffer.offset,
                    .sizeBytes       = content.size,
                });
                return image;
            }
            else
            {
                return result;
            }
        }
    } // namespace Utils
} // namespace RHI