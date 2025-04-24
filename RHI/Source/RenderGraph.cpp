#include "RHI/RenderGraph.hpp"

#include "RHI/Device.hpp"

#include <TL/Allocator/Allocator.hpp>
#include <TL/Allocator/Mimalloc.hpp>
#include <TL/Containers.hpp>

#include <cstdint>
#include <tracy/Tracy.hpp>

namespace RHI
{
    inline static RHI::Access LoadStoreToAccess(LoadOperation loadOp, StoreOperation storeOp)
    {
        if (loadOp == LoadOperation::Load && storeOp == StoreOperation::Store)
        {
            return RHI::Access::ReadWrite;
        }
        else if (loadOp == LoadOperation::Load && storeOp != StoreOperation::Store)
        {
            return RHI::Access::Read;
        }
        else
        {
            return RHI::Access::Write;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Pass
    ///////////////////////////////////////////////////////////////////////////

    ResultCode Pass::Init(RenderGraph* rg, const PassCreateInfo& ci)
    {
        m_renderGraph     = rg;
        m_name            = ci.name;
        m_type            = ci.type;
        m_setupCallback   = ci.setupCallback;
        m_compileCallback = ci.compileCallback;
        m_executeCallback = ci.executeCallback;
        m_imageSize       = ci.size;
        m_isCompiled      = false;
        return ResultCode::Success;
    }

    void Pass::Shutdown()
    {
    }

    void Pass::UseImage(const ImageUseInfo& useInfo)
    {
    }

    void Pass::UseBuffer(const BufferUseInfo& useInfo)
    {
    }

    void Pass::UseColorAttachment(ColorRGAttachment colorAttachment)
    {
        UseImage({
            .image             = colorAttachment.view,
            .subresourcesRange = {},
            .usage             = ImageUsage::Color,
            .stage             = PipelineStage::ColorAttachmentOutput,
            .access            = LoadStoreToAccess(colorAttachment.loadOp, colorAttachment.storeOp),
        });
        if (colorAttachment.resolveView)
        {
            UseImage({
                .image             = colorAttachment.resolveView,
                .subresourcesRange = {},
                .usage             = ImageUsage::Resolve,
                .stage             = PipelineStage::Resolve,
                .access            = Access::Write,
            });
        }
        m_colorAttachments.push_back(colorAttachment);
    }

    void Pass::UseDepthStencil(DepthStencilRGAttachment depthStencilAttachment)
    {
        auto depthAccess    = LoadStoreToAccess(depthStencilAttachment.depthLoadOp, depthStencilAttachment.depthStoreOp);
        auto stencilAccess  = LoadStoreToAccess(depthStencilAttachment.stencilLoadOp, depthStencilAttachment.stencilStoreOp);
        auto combinedAccess = depthAccess | stencilAccess;
        UseImage({
            .image             = depthStencilAttachment.view,
            .subresourcesRange = {},
            .usage             = ImageUsage::DepthStencil,
            .stage             = PipelineStage::EarlyFragmentTests,
            .access            = (Access)(int)combinedAccess,
        });
        m_depthStencilAttachment = depthStencilAttachment;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    ResultCode RenderGraph::Init(const RenderGraphCreateInfo& ci)
    {
        return ResultCode::Success;
    }

    void RenderGraph::Shutdown()
    {
    }

    Handle<RenderGraphImage> RenderGraph::ImportSwapchain(const char* name, Swapchain& swapchain, Format format)
    {
        if (auto handle = FindImportedImage(swapchain.GetImage()))
            return handle;

        auto [imageHandle, result] = m_imageOwner.Create(name, swapchain.GetImage(), format);
        TL_ASSERT(IsSuccess(result));
        m_graphImportedSwapchainsLookup[&swapchain]       = imageHandle;
        m_graphImportedImagesLookup[swapchain.GetImage()] = imageHandle;
        return imageHandle;
    }

    Handle<RenderGraphImage> RenderGraph::ImportImage(const char* name, Handle<Image> image, Format format)
    {
        if (auto handle = FindImportedImage(image))
            return handle;

        auto [imageHandle, result] = m_imageOwner.Create(name, image, format);
        TL_ASSERT(IsSuccess(result));
        m_graphImportedImagesLookup[image] = imageHandle;
        return imageHandle;
    }

    Handle<RenderGraphBuffer> RenderGraph::ImportBuffer(const char* name, Handle<Buffer> buffer)
    {
        if (auto handle = FindImportedBuffer(buffer))
            return handle;

        auto [bufferHandle, result] = m_bufferOwner.Create(name, buffer);
        TL_ASSERT(IsSuccess(result));
        m_graphImportedBuffersLookup[buffer] = bufferHandle;
        return bufferHandle;
    }

    Handle<RenderGraphImage> RenderGraph::CreateImage(const char* name, ImageType type, ImageSize3D size, Format format, uint32_t mipLevels, , uint32_t arrayCount, SampleCount samples)
    {
        auto [image, result] = m_imageOwner.Create(name, type, size, format, mipLevels, arrayCount, samples);
        TL_ASSERT(IsSuccess(result));

        // m_graphImportedImagesLookup[image] = imageHandle;

        return image;
    }

    Handle<RenderGraphBuffer> RenderGraph::CreateBuffer(const char* name, size_t size)
    {
        auto [buffer, result] = m_bufferOwner.Create(name, size);
        TL_ASSERT(IsSuccess(result));

        // m_graphImportedBuffersLookup[buffer] = bufferHandle;

        return buffer;
    }

    Handle<RenderGraphImage> RenderGraph::FindImportedImage(Handle<Image> image) const
    {
        if (auto it = m_graphImportedImagesLookup.find(image); it != m_graphImportedImagesLookup.end())
            return it->second;
        return NullHandle;
    }

    Handle<RenderGraphBuffer> RenderGraph::FindImportedBuffer(Handle<Buffer> buffer) const
    {
        if (auto it = m_graphImportedBuffersLookup.find(buffer); it != m_graphImportedBuffersLookup.end())
            return it->second;
        return NullHandle;
    }

    void RenderGraph::DestroyImage(Handle<RenderGraphImage> handle)
    {
    }

    void RenderGraph::DestroyBuffer(Handle<RenderGraphBuffer> handle)
    {
    }

    void RenderGraph::BeginFrame(ImageSize2D frameSize)
    {
        TL_ASSERT(m_isCompiled == true);
        TL_ASSERT(m_isExecuting == false);
        m_isExecuting = true;
    }

    void RenderGraph::EndFrame()
    {
        TL_ASSERT(m_isExecuting == true);
        m_isExecuting = false;

        if (m_isCompiled == false)
        {
            Compile();
        }
    }

    ImageSize2D RenderGraph::GetFrameSize() const
    {
        TL_ASSERT(m_isExecuting);
        return m_frameSize;
    }

    const RenderGraphImage* RenderGraph::GetImage(Handle<RenderGraphImage> handle) const
    {
        TL_ASSERT(m_isCompiled == true);
        return m_imageOwner.Get(handle);
    }

    const RenderGraphBuffer* RenderGraph::GetBuffer(Handle<RenderGraphBuffer> handle) const
    {
        TL_ASSERT(m_isCompiled == true);
        return m_bufferOwner.Get(handle);
    }

    Handle<Image> RenderGraph::GetImageHandle(Handle<RenderGraphImage> handle) const
    {
        TL_ASSERT(m_isCompiled == true);
        return GetImage(handle)->imageHandle;
    }

    Handle<Buffer> RenderGraph::GetBufferHandle(Handle<RenderGraphBuffer> handle) const
    {
        TL_ASSERT(m_isCompiled == true);
        return GetBuffer(handle)->bufferHandle;
    }

    Pass* RenderGraph::AddPass(const PassCreateInfo& createInfo)
    {
        auto pass   = m_graphPasses.emplace_back(TL::CreatePtr<Pass>()).get();
        auto result = pass->Init(createInfo);
        pass->m_setupCallback(*pass);
        TL_ASSERT(result == ResultCode::Success);
        return pass;
    }

    void RenderGraph::QueueBufferUpload(Handle<RenderGraphBuffer> buffer, TL::Block data)
    {
        QueueBufferUpload(buffer, 0, data);
    }

    void RenderGraph::QueueBufferUpload(Handle<RenderGraphBuffer> buffer, uint32_t offset, TL::Block data)
    {
        TL_UNREACHABLE(); // not implemented yet
    }

    void RenderGraph::TransientResourcesInit(bool enableAliasing)
    {
        TransientResourcesShutdown();

        if (enableAliasing)
        {
            TL_UNREACHABLE();
        }
        else
        {
            for (auto transientImageHandle : m_graphTransientImagesLookup)
            {
                auto image            = m_imageOwner.Get(transientImageHandle);
                auto [handle, result] = m_device->CreateImage({
                    .name        = image->m_name.c_str(),
                    .usageFlags  = image->m_usageFlags,
                    .type        = image->m_type,
                    .size        = image->m_size,
                    .format      = image->m_format,
                    .sampleCount = image->m_sampleCount,
                    .mipLevels   = image->m_mipLevels,
                    .arrayCount  = image->m_arrayCount,
                });
                TL_ASSERT(result == ResultCode::Success);
                image->m_handle = handle;
            }

            for (auto transientBufferHandle : m_graphTransientBuffersLookup)
            {
                auto buffer           = m_bufferOwner.Get(transientBufferHandle);
                auto [handle, result] = m_device->CreateBuffer({
                    .name       = buffer->m_name.c_str(),
                    .hostMapped = true,
                    .usageFlags = buffer->m_usageFlags,
                    .byteSize   = buffer->m_size,
                });
                TL_ASSERT(result == ResultCode::Success);
                buffer->m_handle = handle;
            }
        }
    }

    void RenderGraph::TransientResourcesShutdown()
    {
        for (auto transientImageHandle : m_graphTransientImagesLookup)
        {
            auto image = m_imageOwner.Get(transientImageHandle);
            m_device->DestroyImage(image->m_handle);
        }
        for (auto transientBufferHandle : m_graphTransientBuffersLookup)
        {
            auto buffer = m_bufferOwner.Get(transientBufferHandle);
            m_device->DestroyBuffer(buffer->m_handle);
        }
    }

    void RenderGraph::Compile()
    {
        ZoneScoped;

        // Sort the graph topologically
        {
        }

        // Build
        {
        }
    }

    //     void RenderGraph::Execute()
    //     {
    //     }

    //     void RenderGraph::ExecutePassGroup(CommandList& commandList, RenderGraphExecuteGroup& group)
    //     {
    // #if 0
    //         ZoneScoped;

    //         constexpr ClearValue queueColor[3] =
    //             {
    //                 {.f32{0.0f, 0.5f, 0.0f, 1.0f}},
    //                 {.f32{0.0f, 0.0f, 0.5f, 1.0f}},
    //                 {.f32{0.5f, 0.0f, 0.0f, 1.0f}},
    //             };
    //         for (auto pass : group.GetPassList())
    //         {
    //             commandList.PushDebugMarker(pass->GetName(), queueColor[(uint32_t)pass->GetType()]);

    //             commandList.AddPipelineBarrier(
    //                 pass->GetBarriers(Pass::BarrierSlot::Prilogue),
    //                 pass->GetImageBarriers(Pass::BarrierSlot::Prilogue),
    //                 pass->GetBufferBarriers(Pass::BarrierSlot::Prilogue));

    //             if (pass->GetType() == PassType::Graphics)
    //             {
    //                 TL::Vector<ColorAttachment>          colorAttachments{};
    //                 TL::Optional<DepthStencilAttachment> depthStencilAttachment{};

    //                 for (auto colorAttachment : pass->GetColorAttachments())
    //                 {
    //                     colorAttachments.push_back(
    //                         ColorAttachment{
    //                             .view        = GetImageHandle(colorAttachment.view),
    //                             .loadOp      = colorAttachment.loadOp,
    //                             .storeOp     = colorAttachment.storeOp,
    //                             .clearValue  = colorAttachment.clearValue,
    //                             .resolveMode = colorAttachment.resolveMode,
    //                             .resolveView = colorAttachment.resolveView ? GetImageHandle(colorAttachment.resolveView) : NullHandle,
    //                         });
    //                 }

    //                 if (pass->GetDepthStencilAttachment())
    //                 {
    //                     depthStencilAttachment = DepthStencilAttachment{
    //                         .view           = GetImageHandle(pass->GetDepthStencilAttachment()->view),
    //                         .depthLoadOp    = pass->GetDepthStencilAttachment()->depthLoadOp,
    //                         .depthStoreOp   = pass->GetDepthStencilAttachment()->depthStoreOp,
    //                         .stencilLoadOp  = pass->GetDepthStencilAttachment()->stencilLoadOp,
    //                         .stencilStoreOp = pass->GetDepthStencilAttachment()->stencilStoreOp,
    //                         .clearValue     = pass->GetDepthStencilAttachment()->clearValue,
    //                     };
    //                 }

    //                 RenderPassBeginInfo beginInfo{
    //                     .size                   = pass->GetImageSize(),
    //                     .offset                 = {},
    //                     .colorAttachments       = colorAttachments,
    //                     .depthStencilAttachment = depthStencilAttachment,
    //                 };
    //                 commandList.BeginRenderPass(beginInfo);
    //             }
    //             else if (pass->GetType() == PassType::Compute)
    //             {
    //                 ComputePassBeginInfo beginInfo{};
    //                 commandList.BeginComputePass(beginInfo);
    //             }

    //             pass->m_executeCallback(commandList);

    //             if (pass->GetType() == PassType::Graphics)
    //                 commandList.EndRenderPass();
    //             else if (pass->GetType() == PassType::Compute)
    //                 commandList.EndComputePass();

    //             commandList.AddPipelineBarrier(
    //                 pass->GetBarriers(Pass::BarrierSlot::Epilogue),
    //                 pass->GetImageBarriers(Pass::BarrierSlot::Epilogue),
    //                 pass->GetBufferBarriers(Pass::BarrierSlot::Epilogue));

    //             commandList.PopDebugMarker();
    //         }
    // #endif
    //     }

} // namespace RHI