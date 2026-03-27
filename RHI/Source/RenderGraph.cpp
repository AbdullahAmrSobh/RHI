#include "RHI/RenderGraph.hpp"

#include "RHI/Device.hpp"
#include "RHI/Reflect.hpp"

#include <TL/Utils.hpp>
#include <TL/Defer.hpp>

#include <cstdint>
#include <tracy/Tracy.hpp>

#include "RHI/internal/renderdoc_app.h"

namespace RHI
{
    struct Renderdoc
    {
        ResultCode Init(class Device* device)
        {
            m_device = device;

            auto [library, result] = TL::Library::open("renderdoc.dll");
            if (result.IsError())
            {
                TL_LOG_ERROR("Failed to load RenderDoc API: {}", result.GetMessage());
                return ResultCode::ErrorUnknown;
            }

            m_library    = library;
            auto* getAPI = m_library.getProc<pRENDERDOC_GetAPI>("RENDERDOC_GetAPI");
            if (!getAPI)
            {
                TL_LOG_ERROR("Failed to get RENDERDOC_GetAPI function pointer");
                TL::Library::close(m_library);
                return ResultCode::ErrorUnknown;
            }

            auto api = reinterpret_cast<RENDERDOC_API_1_6_0**>(&m_renderdocAPi);
            if (!getAPI(eRENDERDOC_API_Version_1_6_0, reinterpret_cast<void**>(api)))
            {
                TL_LOG_ERROR("Failed to initialize RenderDoc API");
                TL::Library::close(m_library);
                return ResultCode::ErrorUnknown;
            }

            if (api)
                (*api)->MaskOverlayBits(0, 0);

            TL_LOG_INFO("Renderdoc connected");
            return ResultCode::Success;
        }

        void Shutdown()
        {
            if (auto* api = (RENDERDOC_API_1_6_0*)m_renderdocAPi)
                api->Shutdown();
            TL::Library::close(m_library);
        }

        void FrameTriggerMultiCapture(uint32_t numFrames)
        {
            if (auto* api = (RENDERDOC_API_1_6_0*)m_renderdocAPi)
                api->TriggerMultiFrameCapture(numFrames);
        }

        bool FrameIsCapturing()
        {
            if (auto* api = (RENDERDOC_API_1_6_0*)m_renderdocAPi)
                return api->IsFrameCapturing();
            return false;
        }

        void FrameStartCapture()
        {
            if (auto* api = (RENDERDOC_API_1_6_0*)m_renderdocAPi)
            {
                auto device = (void*)m_device->GetNativeHandle(NativeHandleType::Device, (uint64_t)m_device);
                api->StartFrameCapture(device, nullptr);
            }
        }

        void FrameEndCapture()
        {
            if (auto* api = (RENDERDOC_API_1_6_0*)m_renderdocAPi)
            {
                auto device = (void*)m_device->GetNativeHandle(NativeHandleType::Device, (uint64_t)m_device);
                api->EndFrameCapture(device, nullptr);
            }
        }

    private:
        Device*              m_device;       ///< Associated device.
        TL::Library          m_library;      ///< Handle to the Renderdoc library.
        RENDERDOC_API_1_6_0* m_renderdocAPi; ///< Pointer to the Renderdoc API interface.
    };

    namespace Colors
    {
        static constexpr uint32_t Red               = 0x7F000000;
        static constexpr uint32_t Green             = 0x007F7F00;
        static constexpr uint32_t Blue              = 0x00007FFF;
        static constexpr uint32_t GraphicsQueue     = 0xFF3232B3;
        static constexpr uint32_t ComputeQueue      = 0xFF32B332;
        static constexpr uint32_t AsyncComputeQueue = 0xFF3D993D;
        static constexpr uint32_t TransferQueue     = 0xFFB36432;
    }; // namespace Colors

    struct ImageUsageAndPipelineStage
    {
        ImageUsage    usage;
        PipelineStage stage;
        Access        access;
    };

    inline static constexpr ImageUsageAndPipelineStage
    getUsageAndStage(TL::Flags<RGImageUsage> usageMask)
    {
        ImageUsageAndPipelineStage out{ImageUsage::None, PipelineStage::TopOfPipe, Access::None};

        // Build image usage flags
        ImageUsage iu          = ImageUsage::None;
        bool       needRead    = false;
        bool       needWrite   = false;
        bool       anyStageSet = false;

        // SRV usages -> shader resource
        if (usageMask & (RGImageUsage::SrvGeometry | RGImageUsage::SrvPixel | RGImageUsage::SrvCompute | RGImageUsage::SrvTraceRays))
        {
            iu          = ImageUsage::ShaderResource;
            needRead    = true;
            anyStageSet = true;
        }
        // UAV usages -> storage resource
        else if (usageMask & (RGImageUsage::UavGeometry | RGImageUsage::UavPixel | RGImageUsage::UavCompute | RGImageUsage::UavTraceRays))
        {
            iu          = ImageUsage::StorageResource;
            needRead    = true;
            needWrite   = true;
            anyStageSet = true;
        }
        // RTV/DSV usages
        else if (usageMask & RGImageUsage::RtvDsvRead)
        {
            iu          = ImageUsage::Color;
            needRead    = true;
            anyStageSet = true;
            out.stage   = PipelineStage::ColorAttachmentOutput;
        }
        else if (usageMask & RGImageUsage::RtvDsvWrite)
        {
            iu          = ImageUsage::Color;
            needWrite   = true;
            anyStageSet = true;
            out.stage   = PipelineStage::ColorAttachmentOutput;
        }

        // Copy destination
        if (usageMask & RGImageUsage::CopyDestination)
        {
            iu          = ImageUsage::CopyDst;
            needWrite   = true;
            out.stage   = PipelineStage::Copy;
            anyStageSet = true;
        }
        // Copy source
        if (usageMask & RGImageUsage::CopySource)
        {
            iu          = ImageUsage::CopySrc;
            needRead    = true;
            out.stage   = PipelineStage::Copy;
            anyStageSet = true;
        }
        // Present
        if (usageMask & RGImageUsage::Present)
        {
            iu          = ImageUsage::Present;
            // Prefer BottomOfPipe only if no shader stages are required. We'll combine stages below.
            out.stage   = PipelineStage::BottomOfPipe;
            anyStageSet = true;
        }

        // Decide pipeline stage based on specific flags. Prefer more-specific stages when possible.
        PipelineStage stageVar = PipelineStage::TopOfPipe;
        // Prefer raytracing if requested
        if (usageMask & (RGImageUsage::SrvTraceRays | RGImageUsage::UavTraceRays))
            stageVar = PipelineStage::RayTracingShader;
        else if (usageMask & (RGImageUsage::SrvCompute | RGImageUsage::UavCompute))
            stageVar = PipelineStage::ComputeShader;
        else if (usageMask & (RGImageUsage::SrvPixel | RGImageUsage::UavPixel))
            stageVar = PipelineStage::PixelShader;
        else if (usageMask & (RGImageUsage::SrvGeometry | RGImageUsage::UavGeometry))
            stageVar = PipelineStage::VertexShader;

        // RTV/DSV, copy, present influence common shader stages.
        if (usageMask & (RGImageUsage::RtvDsvRead | RGImageUsage::RtvDsvWrite))
            stageVar = PipelineStage::ColorAttachmentOutput;
        if (usageMask & (RGImageUsage::CopyDestination | RGImageUsage::CopySource))
            stageVar = PipelineStage::Copy;
        // If present was requested but shader stages are also requested, keep shader stages and OR with BottomOfPipe later when forming barriers.
        if ((usageMask & RGImageUsage::Present) && !(usageMask & (RGImageUsage::SrvCompute | RGImageUsage::SrvPixel | RGImageUsage::SrvGeometry | RGImageUsage::SrvTraceRays | RGImageUsage::UavCompute | RGImageUsage::UavPixel | RGImageUsage::UavGeometry | RGImageUsage::UavTraceRays)))
            stageVar = PipelineStage::BottomOfPipe;

        if (!anyStageSet)
            stageVar = PipelineStage::AllCommands;

        out.stage = stageVar;

        if (needRead && needWrite)
            out.access = Access::ReadWrite;
        else if (needWrite)
            out.access = Access::Write;
        else if (needRead)
            out.access = Access::Read;
        else
            out.access = Access::None;

        out.usage = iu;
        return out;
    }

    struct BufferUsageAndPipelineStage
    {
        BufferUsage   usage;
        PipelineStage stage;
        Access        access;
    };

    inline static constexpr BufferUsageAndPipelineStage
    getUsageAndStage(TL::Flags<RGBufferUsage> usageMask)
    {
        BufferUsageAndPipelineStage out{BufferUsage::None, PipelineStage::TopOfPipe, Access::None};

        BufferUsage bu        = BufferUsage::None;
        bool        needRead  = false;
        bool        needWrite = false;

        // Constants (uniform) and SRV-like -> read-only uniform if constant, otherwise treated as uniform/read
        if (usageMask & RGBufferUsage::AllConstant)
        {
            bu       = BufferUsage::Uniform;
            needRead = true;
        }
        else if (usageMask & RGBufferUsage::AllSrv)
        {
            // no direct map for SRV; treat as uniform/read for now
            bu       = BufferUsage::Uniform;
            needRead = true;
        }

        // UAV or write -> storage + read/write
        else if (usageMask & RGBufferUsage::AllUav)
        {
            bu        = BufferUsage::Storage;
            needRead  = true;
            needWrite = true;
        }
        else if (usageMask & RGBufferUsage::AllWrite)
        {
            bu        = BufferUsage::Storage;
            needWrite = true;
        }

        // Copy usages
        else if (usageMask & RGBufferUsage::CopyDestination)
        {
            bu        = BufferUsage::CopyDst;
            needWrite = true;
            out.stage = PipelineStage::Copy;
        }
        else if (usageMask & RGBufferUsage::CopySource)
        {
            bu        = BufferUsage::CopySrc;
            needRead  = true;
            out.stage = PipelineStage::Copy;
        }

        // Indirect draws/dispatches
        if (usageMask & RGBufferUsage::AllIndirect)
        {
            bu        = BufferUsage::Indirect;
            // Indirect draws are executed by the DrawIndirect pipeline stage
            out.stage = PipelineStage::DrawIndirect;
            needRead  = true;
        }

        // Default stage
        if (out.stage == PipelineStage::TopOfPipe)
            out.stage = PipelineStage::AllCommands;

        if (needRead && needWrite)
            out.access = Access::ReadWrite;
        else if (needWrite)
            out.access = Access::Write;
        else if (needRead)
            out.access = Access::Read;
        else
            out.access = Access::None;

        out.usage = bu;
        return out;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Pass Resource types
    ///////////////////////////////////////////////////////////////////////////

    RGFrameImage* RenderGraph::CreateFrameImage(TL::StringView name)
    {
        auto image  = TL::constructFrom<RGFrameImage>(&m_arena);
        image->name = name;
        return image;
    }

    RGFrameBuffer* RenderGraph::CreateFrameBuffer(TL::StringView name)
    {
        auto buffer  = TL::constructFrom<RGFrameBuffer>(&m_arena);
        buffer->name = name;
        return buffer;
    }

    RGImage* RenderGraph::EmplacePassImage(RGFrameImage* frameImage, RGPass* pass, ImageBarrierState initialState)
    {
        auto image             = TL::constructFrom<RGImage>(&m_arena);
        image->m_frameResource = frameImage;
        image->m_state         = initialState;
        image->m_producer      = pass;
        image->m_prevHandle    = frameImage->lastProducer;

        if (frameImage->lastProducer)
        {
            frameImage->lastProducer->m_nextHandle = image;
        }

        frameImage->lastProducer = image;
        return image;
    }

    RGBuffer* RenderGraph::EmplacePassBuffer(RGFrameBuffer* frameBuffer, RGPass* pass, BufferBarrierState initialState)
    {
        auto buffer             = TL::constructFrom<RGBuffer>(&m_arena);
        buffer->m_frameResource = frameBuffer;
        buffer->m_state         = initialState;
        buffer->m_producer      = pass;
        buffer->m_prevHandle    = frameBuffer->lastProducer;

        if (frameBuffer->lastProducer)
        {
            frameBuffer->lastProducer->m_nextHandle = buffer;
        }

        frameBuffer->lastProducer = buffer;
        return buffer;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Pass
    ///////////////////////////////////////////////////////////////////////////

    RGPass::RGPass(RenderGraph* rg, TL::StringView name, RGPassType type, ImageSize2D size2D)
        : m_name(name)
        , m_type(type)
        , m_renderGraph(rg)
        , m_dependencyLevelIndex(0)
        , m_indexInUnorderedList(0)
        , m_executionQueueIndex(0)
        , m_producers(rg->m_arena)
        , m_imageDependencies(rg->m_arena)
        , m_bufferDependencies(rg->m_arena)
        , m_gfxPassInfo(rg->m_arena)
        , m_barriers(rg->m_arena)
    {
        // default state
        m_state.culled        = false;
        m_state.shouldCompile = true;
        m_gfxPassInfo.m_size  = size2D;
    }

    RGPass::~RGPass()
    {
    }

    TL::StringView RGPass::name() const
    {
        return m_name;
    }

    RGImage* RGPass::createRenderTarget(const RGAttachmentCreateInfo& ci)
    {
        bool isColor        = GetFormatInfo(ci.format).hasRed || GetFormatInfo(ci.format).hasGreen || GetFormatInfo(ci.format).hasBlue || GetFormatInfo(ci.format).hasAlpha;
        bool isDepth        = GetFormatInfo(ci.format).hasDepth;
        bool isStencil      = GetFormatInfo(ci.format).hasStencil;
        bool isDepthStencil = isDepth && isStencil;

        RGFrameImage* frameImage = m_renderGraph->CreateFrameImage(ci.name);
        frameImage->format       = ci.format;
        frameImage->sampleCount  = ci.sampleCount;
        frameImage->type         = ImageType::Image2D;
        frameImage->size.width   = this->m_gfxPassInfo.m_size.width;
        frameImage->size.height  = this->m_gfxPassInfo.m_size.height;

        m_renderGraph->m_imagePool.push_back(frameImage);

        // mark usage for the frame image (color or depth/stencil)
        if (isColor)
            frameImage->usageFlags |= ImageUsage::Color;
        else if (isDepthStencil || isDepth || isStencil)
            frameImage->usageFlags |= ImageUsage::DepthStencil;

        auto img = m_renderGraph->EmplacePassImage(frameImage, this, {});

        // record that this pass produces the image
        m_renderGraph->AddDependency(nullptr, this);

        // Use the unified useRenderTarget path to register dependencies and attach info.
        // Pass through resolveMode so attachments can be configured appropriately.
        return useRenderTarget(img, ImageSubresourceRange::All(), LoadOperation::Discard, ci.storeOp, ci.clearValue, ci.resolveMode);
    }

    RGImage* RGPass::useRenderTarget(RGImage* image, const ImageSubresourceRange& range, LoadOperation loadOp, StoreOperation storeOp, ClearValue clearValue, ResolveMode resolveMode)
    {
        // mark that this pass reads/writes the provided render target
        m_renderGraph->AddDependency(image->m_producer, this);

        RGImageDependency dep{};
        dep.image  = image;
        dep.viewID = 0;

        bool isColor = GetFormatInfo(image->m_frameResource->format).hasRed || GetFormatInfo(image->m_frameResource->format).hasGreen || GetFormatInfo(image->m_frameResource->format).hasBlue || GetFormatInfo(image->m_frameResource->format).hasAlpha;
        if (isColor)
            dep.state.usage = ImageUsage::Color;
        else
            dep.state.usage = ImageUsage::DepthStencil;

        dep.state.stage  = PipelineStage::ColorAttachmentOutput;
        dep.state.access = Access::None;
        if (loadOp == LoadOperation::Load)
            dep.state.access = Access::Read;
        if (storeOp == StoreOperation::Store)
            dep.state.access = Access::Write;

        m_imageDependencies.push_back(dep);

        // Update the underlying frame image usage flags so the allocator knows how the image will be used
        image->m_frameResource->usageFlags |= dep.state.usage;

        // Also register this image as an attachment on the graphics pass (color or depth/stencil)
        if (isColor)
        {
            RGColorAttachment attachment{};
            attachment.color       = image;
            attachment.loadOp      = loadOp;
            attachment.storeOp     = storeOp;
            attachment.clearValue  = clearValue;
            attachment.resolveMode = resolveMode;
            attachment.resolveView = nullptr;
            m_gfxPassInfo.m_colorAttachments.push_back(attachment);
        }
        else
        {
            // depth and/or stencil attachment(s)
            if (GetFormatInfo(image->m_frameResource->format).hasDepth)
            {
                m_gfxPassInfo.m_depthStencilAttachment.depthStencil          = image;
                m_gfxPassInfo.m_depthStencilAttachment.depthLoadOp           = loadOp == LoadOperation::Load ? LoadOperation::Load : LoadOperation::Discard;
                m_gfxPassInfo.m_depthStencilAttachment.depthStoreOp          = storeOp;
                m_gfxPassInfo.m_depthStencilAttachment.clearValue.depthValue = clearValue.ds.depthValue;
            }
            if (GetFormatInfo(image->m_frameResource->format).hasStencil)
            {
                m_gfxPassInfo.m_depthStencilAttachment.depthStencil            = image;
                m_gfxPassInfo.m_depthStencilAttachment.stencilLoadOp           = loadOp == LoadOperation::Load ? LoadOperation::Load : LoadOperation::Discard;
                m_gfxPassInfo.m_depthStencilAttachment.stencilStoreOp          = storeOp;
                m_gfxPassInfo.m_depthStencilAttachment.clearValue.stencilValue = clearValue.ds.stencilValue;
            }
        }

        return image;
    }

    RGImage* RGPass::useRenderTarget(RGImage* image, LoadOperation loadOp, StoreOperation storeOp, ClearValue clearValue)
    {
        return useRenderTarget(image, ImageSubresourceRange::All(), loadOp, storeOp, clearValue);
    }

    RGImage* RGPass::createImage(const RGImageCreateInfo& ci, RGImageUsage usage)
    {
        RGFrameImage* frameImage = m_renderGraph->CreateFrameImage(ci.name);
        frameImage->type         = ci.type;
        frameImage->size         = ci.size;
        frameImage->format       = ci.format;
        frameImage->sampleCount  = ci.sampleCount;
        frameImage->mipLevels    = ci.mipLevels;
        frameImage->arrayCount   = ci.arrayCount;
        m_renderGraph->m_imagePool.push_back(frameImage);
        auto img = m_renderGraph->EmplacePassImage(frameImage, this, {});

        // Record that this pass produces the image
        m_renderGraph->AddDependency(nullptr, this);

        auto [imageUsage, stage, access] = getUsageAndStage(usage);

        // Accumulate usage flags on the frame resource
        frameImage->usageFlags |= imageUsage;

        // Add an explicit write dependency for this image
        RGImageDependency dep{};
        dep.image        = img;
        dep.viewID       = 0;
        dep.state.usage  = imageUsage;
        dep.state.stage  = stage;
        dep.state.access = access;
        m_imageDependencies.push_back(dep);
        return img;
    }

    RGImage* RGPass::writeImage(RGImage* image, const ImageSubresourceRange& range, RGImageUsage usage)
    {
        // find or create RGImage for that view (represented by EmplacePassImage)
        RGImage* rgView = m_renderGraph->EmplacePassImage(image->m_frameResource, this, {});

        // add dependency from previous producer to this pass
        m_renderGraph->AddDependency(image->m_producer, this);

        auto [imageUsage, stage, access] = getUsageAndStage(usage);

        // Update the frame resource usage
        image->m_frameResource->usageFlags |= imageUsage;

        RGImageDependency dep{};
        dep.image        = rgView;
        dep.viewID       = 0;
        dep.state.usage  = imageUsage;
        dep.state.stage  = stage;
        dep.state.access = access;
        m_imageDependencies.push_back(dep);
        return rgView;
    }

    RGImage* RGPass::writeImage(RGImage* image, RGImageUsage usage)
    {
        return writeImage(image, ImageSubresourceRange::All(), usage);
    }

    RGImage* RGPass::resolveImage(RGImage* src, const ImageSubresourceRange& range)
    {
        // TODO: implement
        return nullptr;
    }

    RGImage* RGPass::resolveImage(RGImage* src)
    {
        return resolveImage(src, ImageSubresourceRange::All());
    }

    void RGPass::readImage(RGImage* image, const ImageSubresourceRange& range, RGImageUsage usage)
    {
        m_renderGraph->AddDependency(image->m_producer, this);

        auto [imageUsage, stage, access] = getUsageAndStage(usage);

        RGImageDependency dep{};
        dep.image        = image;
        dep.viewID       = 0;
        dep.state.usage  = imageUsage;
        dep.state.stage  = stage;
        dep.state.access = access;
        m_imageDependencies.push_back(dep);
        // Update the frame resource usage
        image->m_frameResource->usageFlags |= imageUsage;
    }

    void RGPass::readImage(RGImage* image, RGImageUsage usage)
    {
        readImage(image, ImageSubresourceRange::All(), usage);
    }

    RGBuffer* RGPass::createBuffer(const RGBufferCreateInfo& ci, RGBufferUsage usage)
    {
        RGFrameBuffer* frameBuffer = m_renderGraph->CreateFrameBuffer(ci.name);
        frameBuffer->size          = ci.size;
        m_renderGraph->m_bufferPool.push_back(frameBuffer);
        auto buf = m_renderGraph->EmplacePassBuffer(frameBuffer, this, {});

        m_renderGraph->AddDependency(nullptr, this);

        auto [bufferUsage, stage, access] = getUsageAndStage(usage);

        // Accumulate usage flags on the frame buffer
        frameBuffer->usageFlags |= bufferUsage;

        RGBufferDependency dep{};
        dep.buffer       = buf;
        dep.subregion    = BufferSubregion{0, ci.size};
        dep.state.usage  = bufferUsage;
        dep.state.stage  = stage;
        dep.state.access = access;
        m_bufferDependencies.push_back(dep);
        return buf;
    }

    RGBuffer* RGPass::readBuffer(RGBuffer* buffer, const BufferSubregion& subregion, RGBufferUsage usage)
    {
        m_renderGraph->AddDependency(buffer->m_producer, this);

        auto [bufferUsage, stage, access] = getUsageAndStage(usage);

        RGBufferDependency dep{};
        dep.buffer       = buffer;
        dep.subregion    = subregion;
        dep.state.usage  = bufferUsage;
        dep.state.stage  = stage;
        dep.state.access = access;
        m_bufferDependencies.push_back(dep);
        // Update the frame buffer usage flags
        buffer->m_frameResource->usageFlags |= bufferUsage;
        return buffer;
    }

    RGBuffer* RGPass::readBuffer(RGBuffer* buffer, RGBufferUsage usage)
    {
        return readBuffer(buffer, BufferSubregion{0, buffer->m_frameResource->size}, usage);
    }

    RGBuffer* RGPass::writeBuffer(RGBuffer* buffer, const BufferSubregion& subregion, RGBufferUsage usage)
    {
        auto buf = m_renderGraph->EmplacePassBuffer(buffer->m_frameResource, this, {});

        m_renderGraph->AddDependency(buffer->m_producer, this);

        auto [bufferUsage, stage, access] = getUsageAndStage(usage);

        RGBufferDependency dep{};
        dep.buffer       = buf;
        dep.subregion    = subregion;
        dep.state.usage  = bufferUsage;
        dep.state.stage  = stage;
        dep.state.access = access;
        m_bufferDependencies.push_back(dep);
        // Update the frame buffer usage flags
        buf->m_frameResource->usageFlags |= bufferUsage;
        return buf;
    }

    RGBuffer* RGPass::writeBuffer(RGBuffer* buffer, RGBufferUsage usage)
    {
        return writeBuffer(buffer, BufferSubregion{0, buffer->m_frameResource->size}, usage);
    }

    void RGPass::Execute(CommandList& commandList)
    {
        ZoneScopedN("RGPass::Execute");
        TL_ASSERT(m_executeCallback);
        m_executeCallback(commandList);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Resource Pool
    ///////////////////////////////////////////////////////////////////////////

    inline static bool operator==(const ImageCreateInfo& ci1, const ImageCreateInfo& ci2)
    {
        return (ci1.usageFlags == ci2.usageFlags) &&
               (ci1.type == ci2.type) &&
               (ci1.size == ci2.size) &&
               (ci1.format == ci2.format) &&
               (ci1.sampleCount == ci2.sampleCount) &&
               (ci1.mipLevels == ci2.mipLevels) &&
               (ci1.arrayCount == ci2.arrayCount);
    }

    inline static bool operator==(const BufferCreateInfo& ci1, const BufferCreateInfo& ci2)
    {
        return (ci1.usageFlags == ci2.usageFlags) &&
               (ci1.byteSize == ci2.byteSize);
    }

    ResultCode RenderGraphResourcePool::Init(Device* device)
    {
        m_device = device;

        // m_imageCache.clear();
        // m_bufferCache.clear();
        return ResultCode::Success;
    }

    void RenderGraphResourcePool::Shutdown()
    {
        for (const auto& [_, handle] : m_imageCache)
        {
            m_device->DestroyImage(handle.second);
        }
        for (const auto& [_, handle] : m_bufferCache)
        {
            m_device->DestroyBuffer(handle.second);
        }

        m_imageCache.clear();
        m_bufferCache.clear();
    }

    Image* RenderGraphResourcePool::InitTransientImage(RGFrameImage* rgImage)
    {
        auto it = m_imageCache.find(rgImage->name);

        if (rgImage->isImported)
            return rgImage->handle;

        ImageCreateInfo imageCI{
            .name        = rgImage->name.c_str(),
            .usageFlags  = rgImage->usageFlags,
            .type        = rgImage->type,
            .size        = rgImage->size,
            .format      = rgImage->format,
            .sampleCount = rgImage->sampleCount,
            .mipLevels   = rgImage->mipLevels,
            .arrayCount  = rgImage->arrayCount,
        };

        // First try to find a in cache
        if (it != m_imageCache.end())
        {
            const auto& [cachedCI, handle] = it->second;
            if (cachedCI == imageCI)
                return handle;
            // Resource with same name, but different properties were found, so recreate the resource.
            TL_LOG_INFO("...Recreating resource {}\n {}", rgImage->name, Debug::ToString(imageCI));
            m_device->DestroyImage(handle);
            m_imageCache.erase(it); // No need (I think) as
        }
        else
        {
            TL_LOG_INFO("Creating image {}", Debug::ToString(imageCI));
        }

        auto [_, handle] = m_imageCache[rgImage->name] = std::pair{imageCI, m_device->CreateImage(imageCI)};
        return handle;
    }

    Buffer* RenderGraphResourcePool::InitTransientBuffer(RGFrameBuffer* rgBuffer)
    {
        auto it = m_bufferCache.find(rgBuffer->name);

        if (rgBuffer->isImported)
        {
            return rgBuffer->handle;
        }

        BufferCreateInfo ci{
            .name       = rgBuffer->name.c_str(),
            .usageFlags = rgBuffer->usageFlags,
            .byteSize   = rgBuffer->size,
        };

        if (it != m_bufferCache.end())
        {
            const auto& [cachedCI, handle] = it->second;
            if (cachedCI == ci)
                return handle;

            // Resource with same name, but different properties were found, so recreate the resource.
            TL_LOG_INFO("...Recreating resource {}\n {}", rgBuffer->name, Debug::ToString(ci));
            m_device->DestroyBuffer(handle);
            m_bufferCache.erase(it);
        }
        else
        {
            TL_LOG_INFO("Creating buffer {}", Debug::ToString(ci));
        }

        auto [_, handle] = m_bufferCache[rgBuffer->name] = std::pair{ci, m_device->CreateBuffer(ci)};
        return handle;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    RenderGraph::RenderGraph()  = default;
    RenderGraph::~RenderGraph() = default;

    RenderGraph* RenderGraph::create(Device* device, const RenderGraphCreateInfo& createInfo)
    {
        ZoneScoped;

        auto* rg           = TL::construct<RenderGraph>();
        rg->m_device       = device;
        rg->m_resourcePool = TL::CreatePtr<RenderGraphResourcePool>();
        auto result        = rg->m_resourcePool->Init(rg->m_device);

        rg->m_activeFrame = 0;
        for (uint32_t i = 0; i < 2; i++)
        {
            PerFrame& perFrame = rg->m_frame[i];
            for (int queueType = (int)QueueType::Graphics; queueType < (int)QueueType::Count; queueType++)
            {
                perFrame.commandPool[queueType] = device->CreateCommandPool({.queue = (QueueType)queueType});
            }
        }
        TL_ASSERT(result == ResultCode::Success);

        for (uint32_t i = 0; i < FramesInFlightCount; i++)
        {
            result = rg->m_stagingBuffer[i].Init(device);
            // TODO: For some weird reason this assertion fails even ttthough the result is success
            // TL_ASSERT(result == ResultCode::Success, "Failed to init staging buffer for frame {}", i);
        }

        for (auto& perQueue : rg->m_perQueue)
        {
            perQueue.queue = device->GetQueue(QueueType::Graphics);
            perQueue.fence = device->CreateFence({});
            perQueue.value = 0;
        }
        return rg;
    }

    void RenderGraph::destroy(RenderGraph* rg)
    {
        ZoneScoped;

        for (auto& staging : rg->m_stagingBuffer)
            staging.Shutdown(rg->m_device);

        for (auto& perQueue : rg->m_perQueue)
            if (perQueue.fence)
                rg->m_device->DestroyFence(perQueue.fence);

        for (uint32_t i = 0; i < 2; i++)
            for (int q = 0; q < (int)QueueType::Count; q++)
                if (rg->m_frame[i].commandPool[q])
                    rg->m_device->DestroyCommandPool(rg->m_frame[i].commandPool[q]);

        rg->m_resourcePool->Shutdown();
        rg->m_arena.reset();
        rg->m_passPool.clear();
        rg->m_imagePool.clear();
        rg->m_bufferPool.clear();
        TL::destruct(rg);
    }

    void RenderGraph::BeginFrame(const RGBeginInfo& beginInfo)
    {
        ZoneScopedC(Colors::Red);

        m_beginInfo            = beginInfo;
        m_state.frameRecording = true;

        m_stagingBuffer[m_activeFrame].Reset();

        for (auto& pool : m_frame[m_activeFrame].commandPool)
        {
            pool->Reset();
        }
    }

    void RenderGraph::EndFrame()
    {
        ZoneScopedC(Colors::Red);

        TL_ASSERT(m_state.frameRecording == true);
        m_state.frameRecording = false;

        Compile();
        Execute();

        DumpGraphViz();

        {
            ZoneScopedN("Clear");

            m_dependencyLevels.clear();
            m_bufferPool.clear();
            m_imagePool.clear();
            m_passPool.clear();
            m_swapchains.clear();
            m_pendingBufferWrites.clear();
            m_pendingImageWrites.clear();
            m_streamingActive = false;
            m_arena.reset();

            m_dependencyLevels    = TL::Vector<DependencyLevel>{m_arena};
            m_bufferPool          = TL::Vector<RGFrameBuffer*>{m_arena};
            m_imagePool           = TL::Vector<RGFrameImage*>{m_arena};
            m_passPool            = TL::Vector<RGPass*>{m_arena};
            m_swapchains          = TL::Vector<SwapchainRGInfo>{m_arena};
            m_pendingBufferWrites = TL::Vector<PendingBufferWrite>{m_arena};
            m_pendingImageWrites  = TL::Vector<PendingImageWrite>{m_arena};

            m_beginInfo = {};
            m_state     = {};

            auto& cmdPools = m_frame[m_activeFrame].commandPool;
            for (auto cmdPool : cmdPools)
            {
                cmdPool->Reset();
            }

            m_activeFrame += 1;
            m_activeFrame = m_activeFrame % FramesInFlightCount;
        }
    }

    RGImage* RenderGraph::importSwapchain(TL::StringView name, Swapchain& swapchain, Format format)
    {
        TL_ASSERT(m_state.frameRecording == true);

        auto frameResource        = TL::constructFrom<RGFrameImage>(&m_arena);
        frameResource->name       = name;
        frameResource->handle     = swapchain.GetImage();
        frameResource->format     = format;
        frameResource->isImported = true;
        m_imagePool.push_back(frameResource);

        m_swapchains.push_back({&swapchain, frameResource});

        return EmplacePassImage(frameResource, nullptr, {});
    }

    RGImage* RenderGraph::importImage(TL::StringView name, Image* image, Format format, ImageBarrierState initialState)
    {
        TL_ASSERT(m_state.frameRecording == true);
        auto frameImage        = TL::constructFrom<RGFrameImage>(&m_arena);
        frameImage->name       = name;
        frameImage->handle     = image;
        frameImage->format     = format;
        frameImage->isImported = true;
        m_imagePool.push_back(frameImage);
        return EmplacePassImage(frameImage, nullptr, initialState);
    }

    RGBuffer* RenderGraph::importBuffer(TL::StringView name, Buffer* buffer, BufferBarrierState initialState)
    {
        TL_ASSERT(m_state.frameRecording == true);
        auto frameBuffer        = TL::constructFrom<RGFrameBuffer>(&m_arena);
        frameBuffer->name       = name;
        frameBuffer->handle     = buffer;
        frameBuffer->isImported = true;
        m_bufferPool.push_back(frameBuffer);
        return EmplacePassBuffer(frameBuffer, nullptr, initialState);
    }

    RGPass* RenderGraph::addPass(TL::StringView name, RGPassType type, ImageSize2D size2D)
    {
        ZoneScoped;
        TL_ASSERT(m_state.frameRecording == true);
        uint32_t indexInUnorderedList = m_passPool.size();

        RGPass* pass = TL::constructFrom<RGPass>(&m_arena, this, name, type, size2D);
        m_passPool.emplace_back(pass);
        pass->m_indexInUnorderedList = indexInUnorderedList;
        return pass;
    }

    void RenderGraph::submitPass(RGPass* pass, PassExecuteCallback&& executeCallback)
    {
        ZoneScoped;
        TL_ASSERT(m_state.frameRecording == true);
        pass->m_executeCallback = std::move(executeCallback);
    }

    // private:

    Image* RenderGraph::GetImageHandle(RGImage* image) const
    {
        TL_ASSERT(m_state.compiled);
        TL_ASSERT(image->m_frameResource->handle);
        return image->m_frameResource->handle;
    }

    Buffer* RenderGraph::GetBufferHandle(RGBuffer* buffer) const
    {
        TL_ASSERT(m_state.compiled);
        TL_ASSERT(buffer->m_frameResource->handle);
        return buffer->m_frameResource->handle;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// StagingBuffer
    ///////////////////////////////////////////////////////////////////////////

    ResultCode StagingBuffer::Init(Device* device, size_t capacity)
    {
        m_capacity = capacity;
        m_offset   = 0;

        BufferCreateInfo ci{
            .name       = "RenderGraph-StagingBuffer",
            .usageFlags = BufferUsage::HostMapped | BufferUsage::CopySrc,
            .byteSize   = capacity,
        };

        m_buffer = device->CreateBuffer(ci);
        TL_ASSERT(m_buffer, "Failed to create staging buffer");

        m_mapped = static_cast<uint8_t*>(device->MapBuffer(m_buffer, 0, capacity));
        TL_ASSERT(m_mapped, "Failed to map staging buffer");

        return ResultCode::Success;
    }

    void StagingBuffer::Shutdown(Device* device)
    {
        if (m_buffer)
        {
            device->UnmapBuffer(m_buffer);
            device->DestroyBuffer(m_buffer);
            m_buffer = nullptr;
            m_mapped = nullptr;
        }
    }

    void StagingBuffer::Reset()
    {
        m_offset = 0;
    }

    StagingBuffer::Allocation StagingBuffer::Allocate(size_t size, size_t alignment)
    {
        size_t alignedOffset = (m_offset + alignment - 1) & ~(alignment - 1);
        if (alignedOffset + size > m_capacity)
        {
            TL_LOG_ERROR("StagingBuffer OOM: requested {} B, {} B available", size, m_capacity - m_offset);
            return {};
        }
        m_offset = alignedOffset + size;
        return {m_mapped + alignedOffset, alignedOffset};
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Streaming
    ///////////////////////////////////////////////////////////////////////////

    void RenderGraph::streamBegin()
    {
        TL_ASSERT(m_state.frameRecording, "streamBegin called outside BeginFrame/EndFrame");
        TL_ASSERT(!m_streamingActive, "streamBegin called twice without streamEnd");
        m_streamingActive = true;
    }

    void RenderGraph::streamEnd()
    {
        TL_ASSERT(m_streamingActive, "streamEnd called without matching streamBegin");
        m_streamingActive = false;
    }

    void RenderGraph::streamBufferWrite(Buffer* buffer, size_t offset, TL::Block block)
    {
        TL_ASSERT(m_streamingActive, "streamBufferWrite called outside streamBegin/streamEnd");
        TL_ASSERT(block.ptr && block.size > 0);

        auto& staging = m_stagingBuffer[m_activeFrame];
        auto  alloc   = staging.Allocate(block.size);
        TL_ASSERT(alloc.isValid(), "Staging buffer exhausted during buffer write");

        memcpy(alloc.ptr, block.ptr, block.size);

        m_pendingBufferWrites.push_back({
            .dstBuffer     = buffer,
            .dstOffset     = offset,
            .stagingOffset = alloc.offset,
            .size          = block.size,
        });
    }

    void RenderGraph::streamImageWrite(Image* image, ImageOffset3D offset, ImageSize3D size, uint32_t mipLevel, uint32_t arrayLayer, TL::Block block)
    {
        TL_ASSERT(m_streamingActive, "streamImageWrite called outside streamBegin/streamEnd");
        TL_ASSERT(block.ptr && block.size > 0);

        auto& staging = m_stagingBuffer[m_activeFrame];
        // Images need 256-byte aligned staging offsets (Vulkan spec requirement)
        auto  alloc   = staging.Allocate(block.size, 256);
        TL_ASSERT(alloc.isValid(), "Staging buffer exhausted during image write");

        memcpy(alloc.ptr, block.ptr, block.size);

        uint32_t depth       = size.depth > 0 ? size.depth : 1;
        uint32_t height      = size.height > 0 ? size.height : 1;
        uint32_t bytesPerRow = (uint32_t)(block.size / (height * depth));

        m_pendingImageWrites.push_back({
            .dstImage      = image,
            .imageOffset   = offset,
            .imageSize     = size,
            .mipLevel      = mipLevel,
            .arrayLayer    = arrayLayer,
            .stagingOffset = alloc.offset,
            .bytesPerRow   = bytesPerRow,
        });
    }

    bool RenderGraph::CheckDependency(const RGPass* producer, const RGPass* consumer) const
    {
        return consumer->m_producers.contains(producer->m_indexInUnorderedList);
    }

    void RenderGraph::AddDependency(const RGPass* producer, RGPass* consumer)
    {
        if (producer == nullptr)
            return; // consumer is actually the first producer of this resource

        if (producer == consumer)
            return;

        consumer->m_producers.insert(producer->m_indexInUnorderedList);
    }

    void RenderGraph::Compile()
    {
        ZoneScoped;

        /// TODO: cleanup
        TL::Context ctx{&m_arena};
        TL::Context::push(&ctx);
        TL_defer
        {
            TL::Context::pop();
        };

        TL::Vector<TL::Vector<uint32_t>> adjacencyLists(m_arena);
        adjacencyLists.resize(m_passPool.size(), TL::Vector<uint32_t>{m_arena});
        for (size_t nodeIdx = 0; nodeIdx < m_passPool.size(); ++nodeIdx)
        {
            auto pass = m_passPool[nodeIdx];
            for (uint32_t otherNodeIdx = 0; otherNodeIdx < m_passPool.size(); ++otherNodeIdx)
            {
                if (nodeIdx == otherNodeIdx) continue;
                auto otherNode = m_passPool[otherNodeIdx];
                if (CheckDependency(pass, otherNode))
                {
                    adjacencyLists[nodeIdx].push_back(otherNodeIdx);
                }
            }
        }

        TL::Vector<uint32_t> sortedPasses(m_arena);
        TopologicalSort(adjacencyLists, sortedPasses);

        uint32_t detectedQueueCount = 0;
        {
            TL::Vector<int32_t> longestDistances(sortedPasses.size(), 0, m_arena);
            uint64_t            dependencyLevelCount = 1;
            // Perform longest node distance search
            for (uint32_t nodeIndex = 0; nodeIndex < sortedPasses.size(); ++nodeIndex)
            {
                uint64_t originalIndex      = m_passPool[sortedPasses[nodeIndex]]->m_indexInUnorderedList;
                uint64_t adjacencyListIndex = originalIndex;

                for (uint64_t adjacentNodeIndex : adjacencyLists[adjacencyListIndex])
                {
                    if (longestDistances[adjacentNodeIndex] < longestDistances[originalIndex] + 1)
                    {
                        int32_t newLongestDistance          = longestDistances[originalIndex] + 1;
                        longestDistances[adjacentNodeIndex] = newLongestDistance;
                        dependencyLevelCount                = std::max(uint64_t(newLongestDistance + 1), dependencyLevelCount);
                    }
                }
            }
            m_dependencyLevels.resize(dependencyLevelCount, {m_arena});
            detectedQueueCount = 1;
            // Dispatch nodes to corresponding dependency levels.
            // Iterate through unordered nodes because adjacency lists contain indices to
            // initial unordered list of nodes and longest distances also correspond to them.
            for (uint32_t nodeIndex = 0; nodeIndex < (uint32_t)m_passPool.size(); nodeIndex++)
            {
                RGPass*          pass            = m_passPool[nodeIndex];
                auto             levelIndex      = longestDistances[nodeIndex];
                DependencyLevel& dependencyLevel = m_dependencyLevels[levelIndex];
                dependencyLevel.m_levelIndex     = (uint32_t)levelIndex;
                dependencyLevel.AddPass(pass);
                pass->m_dependencyLevelIndex = levelIndex;
                detectedQueueCount           = std::max(detectedQueueCount, pass->m_executionQueueIndex + 1);
            }
        }

        CreateTransientResources();
        PassBuildBarriers();

        m_state.compiled = true;
    }

    void RenderGraph::DepthFirstSearch(
        uint32_t                                nodeIndex,
        TL::Vector<bool>&                       visited,
        TL::Vector<bool>&                       onStack,
        bool&                                   isCyclic,
        const TL::Vector<TL::Vector<uint32_t>>& adjacencyLists,
        TL::Vector<uint32_t>&                   sortedPasses)
    {
        if (isCyclic) return;
        visited[nodeIndex]          = true;
        onStack[nodeIndex]          = true;
        uint64_t adjacencyListIndex = m_passPool[nodeIndex]->m_indexInUnorderedList;
        for (uint32_t neighbour : adjacencyLists[adjacencyListIndex])
        {
            if (visited[neighbour] && onStack[neighbour])
            {
                isCyclic = true;
                return;
            }
            if (!visited[neighbour])
            {
                DepthFirstSearch(neighbour, visited, onStack, isCyclic, adjacencyLists, sortedPasses);
            }
        }
        onStack[nodeIndex] = false;
        sortedPasses.push_back(m_passPool[nodeIndex]->m_indexInUnorderedList);
    }

    void RenderGraph::TopologicalSort(const TL::Vector<TL::Vector<uint32_t>>& adjacencyLists, TL::Vector<uint32_t>& sortedPasses)
    {
        TL::Vector<bool> visitedNodes(m_passPool.size(), false, m_arena);
        TL::Vector<bool> onStackNodes(m_passPool.size(), false, m_arena);
        bool             isCyclic = false;
        for (uint32_t nodeIndex = 0; nodeIndex < (uint32_t)m_passPool.size(); ++nodeIndex)
        {
            if (!visitedNodes[nodeIndex])
            {
                DepthFirstSearch(nodeIndex, visitedNodes, onStackNodes, isCyclic, adjacencyLists, sortedPasses);
                TL_ASSERT(!isCyclic, "Detected cyclic dependency in pass: {}", m_passPool[nodeIndex]->name().data());
            }
        }
        // I don't know why/how but it flips the expected order
        std::reverse(sortedPasses.begin(), sortedPasses.end());
    }

    void RenderGraph::CreateTransientResources()
    {
        ZoneScoped;

        for (auto frameImage : m_imagePool)
        {
            if (frameImage->isImported)
                continue;

            frameImage->handle = m_resourcePool->InitTransientImage(frameImage);
        }

        for (auto frameBuffer : m_bufferPool)
        {
            if (frameBuffer->isImported)
                continue;

            frameBuffer->handle = m_resourcePool->InitTransientBuffer(frameBuffer);
        }
    }

    void RenderGraph::PassBuildBarriers()
    {
        ZoneScoped;

        for (auto& level : m_dependencyLevels)
        {
            for (auto* pass : level.GetPasses())
            {
                // if (pass->m_state.culled)
                //     continue;

                // TL_LOG_INFO("Collecting {} barriers", pass->GetName());

                for (auto& dep : pass->m_imageDependencies)
                {
                    auto* resource = dep.image;
                    if (resource->m_state == dep.state)
                        continue;

                    pass->m_barriers.imageBarriers.push_back({
                        .image    = resource->m_frameResource->handle,
                        .srcState = resource->m_state,
                        .dstState = dep.state,
                    });

#if RHI_DEBUG_RG_VERBOSE
                    TL_LOG_INFO(
                        "Image Barrier: {} "
                        "src (access={} stage={} usage={}) -> dst (access={} stage={} usage={})",
                        resource->m_frameResource->name,
                        Debug::ToString(resource->m_state.access),
                        Debug::ToString(resource->m_state.stage),
                        Debug::ToString(resource->m_state.usage),
                        Debug::ToString(dep.state.access),
                        Debug::ToString(dep.state.stage),
                        Debug::ToString(dep.state.usage));
#endif

                    resource->m_state = dep.state;
                }

                for (auto& dep : pass->m_bufferDependencies)
                {
                    auto* resource = dep.buffer;
                    if (resource->m_state == dep.state)
                        continue;

                    pass->m_barriers.bufferBarriers.push_back({
                        .buffer   = resource->m_frameResource->handle,
                        .srcState = resource->m_state,
                        .dstState = dep.state,
                    });

#if RHI_DEBUG_RG_VERBOSE
                    TL_LOG_INFO(
                        "Buffer Barrier: {} "
                        "src (access={} stage={} usage={}) -> dst (access={} stage={} usage={})",
                        resource->m_frameResource->name,
                        Debug::ToString(resource->m_state.access),
                        Debug::ToString(resource->m_state.stage),
                        Debug::ToString(resource->m_state.usage),
                        Debug::ToString(dep.state.access),
                        Debug::ToString(dep.state.stage),
                        Debug::ToString(dep.state.usage));
#endif

                    resource->m_state = dep.state;
                }
            }
        }
    }

    void RenderGraph::ExecutePass(RGPass* pass, CommandList* commandList)
    {
        ZoneScoped;

        bool isCompute = pass->m_type == RGPassType::Compute || pass->m_type == RGPassType::AsyncCompute;

        uint32_t markerColor = 0;
        switch (pass->m_type)
        {
        case RGPassType::Graphics:     markerColor = Colors::GraphicsQueue; break;
        case RGPassType::Compute:      markerColor = Colors::ComputeQueue; break;
        case RGPassType::AsyncCompute: markerColor = Colors::AsyncComputeQueue; break;
        case RGPassType::Transfer:     markerColor = Colors::TransferQueue; break;
        }
        commandList->PushDebugMarker(pass->name().data(), markerColor);

        // const auto& [prebarriers, preImageBarriers, preBufferBarriers] = pass->m_barriers[RGPass::Prilogue];
        const auto& [prebarriers, preImageBarriers, preBufferBarriers] = pass->m_barriers;
        commandList->AddPipelineBarrier(prebarriers, preImageBarriers, preBufferBarriers);

        if (pass->m_type == RGPassType::Graphics)
        {
            TL::Vector<ColorAttachment> attachments(m_arena);
            DepthStencilAttachment      dsAttachment{};
            attachments.reserve(pass->m_gfxPassInfo.m_colorAttachments.size());

            // auto [passWidth, passHeight]    = pass->m_gfxPassInfo.m_size;
            for (auto attachment : pass->m_gfxPassInfo.m_colorAttachments)
            {
                auto [imgWidth, imgHeight, imgDepth] = attachment.color->m_frameResource->size;
                // TL_ASSERT(imgWidth == passWidth && imgHeight == passHeight);
                if (attachment.resolveView)
                {
                    auto [resWidth, resHeight, resDepth] = attachment.resolveView->m_frameResource->size;
                    // TL_ASSERT(resWidth == passWidth && resHeight == passHeight);
                }

                auto color   = GetImageHandle(attachment.color);
                auto resolve = attachment.resolveView ? GetImageHandle(attachment.resolveView) : nullptr;
                attachments.push_back({
                    .view        = color,
                    .loadOp      = attachment.loadOp,
                    .storeOp     = attachment.storeOp,
                    .clearValue  = attachment.clearValue,
                    .resolveMode = attachment.resolveMode,
                    .resolveView = resolve,
                });
            }
            if (auto attachment = pass->m_gfxPassInfo.m_depthStencilAttachment.depthStencil)
            {
                auto [dsWidth, dsHeight, dsDepth] = attachment->m_frameResource->size;
                // TL_ASSERT(dsWidth == passWidth && dsHeight == passHeight);
                dsAttachment.view                 = GetImageHandle(attachment);
                dsAttachment.depthLoadOp          = pass->m_gfxPassInfo.m_depthStencilAttachment.depthLoadOp;
                dsAttachment.depthStoreOp         = pass->m_gfxPassInfo.m_depthStencilAttachment.depthStoreOp;
                dsAttachment.stencilLoadOp        = pass->m_gfxPassInfo.m_depthStencilAttachment.stencilLoadOp;
                dsAttachment.stencilStoreOp       = pass->m_gfxPassInfo.m_depthStencilAttachment.stencilStoreOp;
                dsAttachment.clearValue           = pass->m_gfxPassInfo.m_depthStencilAttachment.clearValue;
            }
            RenderPassBeginInfo beginInfo{
                // .name                   = pass->GetName(),
                .size                   = pass->m_gfxPassInfo.m_size,
                .offset                 = pass->m_gfxPassInfo.m_offset,
                .colorAttachments       = attachments,
                .depthStencilAttachment = dsAttachment,
            };
            commandList->BeginRenderPass(beginInfo);
        }
        else if (isCompute)
        {
            ComputePassBeginInfo beginInfo{
                .name = pass->name().data(),
            };
            commandList->BeginComputePass(beginInfo);
        }

        pass->Execute(*commandList);

        if (pass->m_type == RGPassType::Graphics)
            commandList->EndRenderPass();
        else if (isCompute)
            commandList->EndComputePass();

        // const auto& [postbarriers, postImageBarriers, postBufferBarriers] = pass->m_barriers[RGPass::Epilogue];
        // commandList->AddPipelineBarrier(postbarriers, postImageBarriers, postBufferBarriers);
        commandList->PopDebugMarker();
    }

    void RenderGraph::Execute()
    {
        ZoneScoped;

        TL::Context ctx{&m_arena};
        TL::Context::push(&ctx);
        TL_defer
        {
            TL::Context::pop();
        };

        bool enableThreading   = false;
        bool enableAsyncQueues = false;
        if (enableThreading)
        {
            if (enableAsyncQueues)
                ExecuteMultithreadUsingAsyncQueues();
            else
                ExecuteMultithreadUsingSingleQueue();
        }
        else
        {
            if (enableAsyncQueues)
                ExecuteSingleThreadUsingAsyncQueues();
            else
                ExecuteSingleThreadUsingSignelQueue();
        }
    }

    void RenderGraph::ExecuteSingleThreadUsingSignelQueue()
    {
        PerFrame*    frame       = &m_frame[m_activeFrame];
        CommandPool* commandPool = frame->commandPool[(int)QueueType::Graphics];

        auto queue = m_perQueue[(int)QueueType::Graphics].queue;
        auto fence = m_perQueue[(int)QueueType::Graphics].fence;
        auto value = m_perQueue[(int)QueueType::Graphics].value;

        TL::Vector<FenceSubmitInfo> fenceWaitInfos{m_arena};
        TL::Vector<FenceSubmitInfo> fenceSignalInfos{m_arena};

        FenceSubmitInfo fenceWaitInfo{
            .fence = fence,
            .value = value,
            .stage = PipelineStage::TopOfPipe,
        };
        value++;
        FenceSubmitInfo fenceSignalInfo{
            .fence = fence,
            .value = value,
            .stage = PipelineStage::BottomOfPipe,
        };

        TL::Vector<Swapchain*> swapchains{m_arena};

        if (m_beginInfo.rdocDebugCapture)
        {
            // m_rdoc->FrameStartCapture();
        }

        // CommandListCreateInfo commandAllocateInfo{
        //     .name      = "cmd-rendergraph",
        //     .queueType = QueueType::Graphics,
        // };
        CommandList* commandList = commandPool->Allocate();
        commandList->Begin();

        // Emit streaming uploads collected via streamBegin/streamEnd
        if (!m_pendingBufferWrites.empty() || !m_pendingImageWrites.empty())
        {
            Buffer* stagingBuf = m_stagingBuffer[m_activeFrame].GetBuffer();

            // Pre-copy barriers
            TL::Vector<ImageBarrierInfo>  imagePreBarriers{m_arena};
            TL::Vector<BufferBarrierInfo> bufferPreBarriers{m_arena};
            imagePreBarriers.reserve(m_pendingImageWrites.size());
            bufferPreBarriers.reserve(m_pendingBufferWrites.size());

            for (auto& write : m_pendingImageWrites)
            {
                imagePreBarriers.push_back({
                    .image    = write.dstImage,
                    .srcState = {ImageUsage::None, PipelineStage::TopOfPipe, Access::None},
                    .dstState = {ImageUsage::CopyDst, PipelineStage::Copy, Access::Write},
                });
            }
            for (auto& write : m_pendingBufferWrites)
            {
                bufferPreBarriers.push_back({
                    .buffer   = write.dstBuffer,
                    .srcState = {BufferUsage::None, PipelineStage::TopOfPipe, Access::None},
                    .dstState = {BufferUsage::CopyDst, PipelineStage::Copy, Access::Write},
                });
            }
            commandList->AddPipelineBarrier({}, imagePreBarriers, bufferPreBarriers);

            // Copy commands
            for (auto& write : m_pendingBufferWrites)
            {
                commandList->CopyBuffer({
                    .srcBuffer = stagingBuf,
                    .srcOffset = write.stagingOffset,
                    .dstBuffer = write.dstBuffer,
                    .dstOffset = write.dstOffset,
                    .size      = write.size,
                });
            }
            for (auto& write : m_pendingImageWrites)
            {
                commandList->CopyBufferToImage({
                    .image        = write.dstImage,
                    .subresource  = {.imageAspects = ImageAspect::All, .mipLevel = write.mipLevel, .arrayBase = write.arrayLayer, .arrayCount = 1},
                    .imageSize    = write.imageSize,
                    .imageOffset  = write.imageOffset,
                    .buffer       = stagingBuf,
                    .bufferOffset = write.stagingOffset,
                    .bytesPerRow  = write.bytesPerRow,
                });
            }

            // Post-copy barrier: release transfer writes so render graph barriers compute correctly
            TL::Vector<ImageBarrierInfo>  imagePostBarriers{m_arena};
            TL::Vector<BufferBarrierInfo> bufferPostBarriers{m_arena};
            imagePostBarriers.reserve(m_pendingImageWrites.size());
            bufferPostBarriers.reserve(m_pendingBufferWrites.size());

            for (auto& write : m_pendingImageWrites)
            {
                // Leave images in ShaderResource layout importImage(initialState=ShaderResource) will transition from here.
                // This barrier makes the transfer write visible to all subsequent stages.
                imagePostBarriers.push_back({
                    .image    = write.dstImage,
                    .srcState = {ImageUsage::CopyDst, PipelineStage::Copy, Access::Write},
                    .dstState = {ImageUsage::ShaderResource, PipelineStage::AllCommands, Access::Read},
                });
            }
            for (auto& write : m_pendingBufferWrites)
            {
                bufferPostBarriers.push_back({
                    .buffer   = write.dstBuffer,
                    .srcState = {BufferUsage::CopyDst, PipelineStage::Copy, Access::Write},
                    .dstState = {BufferUsage::CopyDst, PipelineStage::AllCommands, Access::Read},
                });
            }
            commandList->AddPipelineBarrier({}, imagePostBarriers, bufferPostBarriers);
        }

        // execute passes
        for (const auto& level : m_dependencyLevels)
        {
            for (auto pass : level.GetPasses())
            {
                ExecutePass(pass, commandList);
            }
        }

        // transition swapchain images to present state, and submit them for presentation
        TL::Vector<ImageBarrierInfo> imageBarriers{m_arena};
        for (auto swapchain : m_swapchains)
        {
            ImageBarrierInfo presentBarrier{
                .image    = swapchain.frameImage->handle,
                .srcState = swapchain.frameImage->lastProducer->m_state,
                .dstState = {ImageUsage::Present, PipelineStage::BottomOfPipe, Access::None},
            };
            swapchains.push_back({swapchain.swapchain});
            imageBarriers.push_back(presentBarrier);
        }
        commandList->AddPipelineBarrier({}, imageBarriers, {});

        commandList->End();

        QueueSubmitInfo queueSubmitInfo{
            .waitFences        = fenceWaitInfos,
            .commandLists      = commandList,
            .signalFences      = fenceSignalInfos,
            .acquireSwapchains = swapchains,
            .presentSwapchains = swapchains,
        };
        queue->Submit(queueSubmitInfo);

        if (m_beginInfo.rdocDebugCapture)
        {
            // m_rdoc->FrameEndCapture();
        }
    }

    void RenderGraph::ExecuteSingleThreadUsingAsyncQueues()
    {
        TL_UNREACHABLE_MSG("TODO: Implement!");
    }

    void RenderGraph::ExecuteMultithreadUsingSingleQueue()
    {
        TL_UNREACHABLE_MSG("TODO: Implement!");
    }

    void RenderGraph::ExecuteMultithreadUsingAsyncQueues()
    {
        TL_UNREACHABLE_MSG("TODO: Implement!");
    }

    void RenderGraph::DumpGraphViz()
    {
        if (m_beginInfo.dumpGraphVizString == nullptr)
            return;
        TL::String output;
        output += "====== RenderGraph State ======\n";
        for (size_t levelIdx = 0; levelIdx < m_dependencyLevels.size(); ++levelIdx)
        {
            const auto& level = m_dependencyLevels[levelIdx];
            output += std::format("Dependency Level {}:\n", levelIdx);
            for (auto pass : level.GetPasses())
            {
                output += std::format("  Pass: {}\n", pass->name());
                // Print image transitions
                for (const auto& dep : pass->m_imageDependencies)
                {
                    auto img = dep.image;
                    output += std::format("    Image: {} | Usage: {}\n", img->m_frameResource->name, Debug::ToString(dep.state.usage));
                }
                // Print buffer transitions
                for (const auto& dep : pass->m_bufferDependencies)
                {
                    auto buf = dep.buffer;
                    output += std::format("    Buffer: {} | Usage: {}\n", buf->m_frameResource->name, Debug::ToString(dep.state.usage));
                }
            }
        }
        output += "================================\n";
        TL_LOG_INFO("{}", output);
    }

} // namespace RHI