#include "RHI/RenderGraphPass.hpp"

#include "RHI/RenderGraphResources.hpp"

namespace RHI
{
    Pass::Pass(const PassCreateInfo& createInfo, TL::IAllocator* allocator)
        : m_allocator(allocator)
        , m_name(createInfo.name)
        , m_queueType(createInfo.queue)
        , m_size(createInfo.size)
        , m_onSetupCallback(createInfo.setupCallback)
        , m_onCompileCallback(createInfo.compileCallback)
        , m_onExecuteCallback(createInfo.executeCallback)
    {
    }

    Pass::~Pass()
    {
        // default
    }

    TL::Span<GraphTransition* const> Pass::GetTransitions() const
    {
        return m_transitions;
    }

    TL::Span<ImageGraphTransition* const> Pass::GetImageTransitions() const
    {
        return m_imageTransitions;
    }

    TL::Span<BufferGraphTransition* const> Pass::GetBufferTransitions() const
    {
        return m_bufferTransitions;
    }

    ColorAttachment GetColorAttachment(const ColorRGAttachment& attachment)
    {
        return ColorAttachment{
            .view        = attachment.view->GetImage(),
            .loadOp      = attachment.loadOp,
            .storeOp     = attachment.storeOp,
            .clearValue  = attachment.clearValue,
            .resolveMode = attachment.resolveMode,
            .resolveView = attachment.resolveView ? attachment.resolveView->GetImage() : NullHandle,
        };
    }

    DepthStencilAttachment GetDepthStencilAttachment(const DepthStencilRGAttachment& attachment)
    {
        return DepthStencilAttachment{
            .view           = attachment.view->GetImage(),
            .depthLoadOp    = attachment.depthLoadOp,
            .depthStoreOp   = attachment.depthStoreOp,
            .stencilLoadOp  = attachment.stencilLoadOp,
            .stencilStoreOp = attachment.stencilStoreOp,
            .clearValue     = attachment.clearValue,
        };
    }

    void Pass::Execute(CommandList& commandList)
    {
        PrepareBarriers();

        commandList.PushDebugMarker(m_name, {});

        auto prilogueMemoryBarriers = GetMemoryBarriers(BarrierSlot_Prilogue);
        auto prilogueImageBarriers  = GetImageBarriers(BarrierSlot_Prilogue);
        auto prilogueBufferBarriers = GetBufferBarriers(BarrierSlot_Prilogue);
        commandList.AddPipelineBarrier(prilogueMemoryBarriers, prilogueImageBarriers, prilogueBufferBarriers);

        if (m_queueType == QueueType::Graphics)
        {
            TL::Vector<ColorAttachment>          colorAttachments;
            TL::Optional<DepthStencilAttachment> depthStencilAttachment;

            for (auto attachment : m_renderPass.m_colorAttachments)
            {
                if (attachment.view != nullptr)
                    colorAttachments.push_back(GetColorAttachment(attachment));
            }

            if (m_renderPass.m_depthStencilAttachment.has_value())
            {
                depthStencilAttachment = GetDepthStencilAttachment(m_renderPass.m_depthStencilAttachment.value());
            }

            RenderPassBeginInfo beginInfo{
                .size                   = m_size,
                .offset                 = {},
                .colorAttachments       = colorAttachments,
                .depthStencilAttachment = depthStencilAttachment,
            };
            commandList.BeginRenderPass(beginInfo);
        }
        m_onExecuteCallback(commandList);
        if (m_queueType == QueueType::Graphics)
        {
            commandList.EndRenderPass();
        }

        auto epilogueMemoryBarriers = GetMemoryBarriers(BarrierSlot_Epilogue);
        auto epilogueImageBarriers  = GetImageBarriers(BarrierSlot_Epilogue);
        auto epilogueBufferBarriers = GetBufferBarriers(BarrierSlot_Epilogue);
        commandList.AddPipelineBarrier(epilogueMemoryBarriers, epilogueImageBarriers, epilogueBufferBarriers);
        commandList.PopDebugMarker();
    }

    void Pass::UseResource(RenderGraphImage& resource, ImageSubresourceRange subresourceRange, ImageUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access)
    {
        auto transition              = m_allocator->Allocate<ImageGraphTransition>();
        transition->pass             = this;
        transition->prev             = resource.GetLastAccess();
        transition->next             = nullptr;
        transition->resource         = &resource;
        transition->usage            = usage;
        transition->stage            = stage;
        transition->access           = access;
        transition->subresourceRange = subresourceRange;
        resource.PushAccess(transition);
        m_imageTransitions.push_back(transition);
    }

    void Pass::UseResource(RenderGraphBuffer& resource, BufferSubregion subregion, BufferUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access)
    {
        auto transition       = m_allocator->Allocate<BufferGraphTransition>();
        transition->pass      = this;
        transition->prev      = resource.GetLastAccess();
        transition->next      = nullptr;
        transition->resource  = &resource;
        transition->usage     = usage;
        transition->stage     = stage;
        transition->access    = access;
        transition->subregion = subregion;
        resource.PushAccess(transition);
        m_bufferTransitions.push_back(transition);
    }

    void Pass::PresentSwapchain(RenderGraphImage& resource)
    {
        auto transition              = m_allocator->Allocate<ImageGraphTransition>();
        transition->pass             = this;
        transition->prev             = resource.GetLastAccess();
        transition->next             = nullptr;
        transition->resource         = &resource;
        transition->usage            = ImageUsage::Present;
        transition->stage            = PipelineStage::ColorAttachmentOutput;
        transition->access           = Access::ReadWrite;
        transition->subresourceRange = {};
        resource.PushAccess(transition);
        m_imageTransitions.push_back(transition);

        {
            auto& imageBarrier        = m_imageBarriers[BarrierSlot_Epilogue].emplace_back();
            imageBarrier.image        = resource.GetImage();
            imageBarrier.srcState     = {transition->usage, transition->stage, transition->access};
            imageBarrier.dstState     = {transition->usage, transition->stage, transition->access};
            imageBarrier.subresources = {};
        }
    }

    void Pass::AddRenderTarget(const ColorRGAttachment& attachment)
    {
        m_renderPass.m_colorAttachments.push_back(attachment);
        UseResource(*attachment.view, {}, ImageUsage::Color, PipelineStage::ColorAttachmentOutput, Access::ReadWrite);
    }

    void Pass::AddRenderTarget(const DepthStencilRGAttachment& attachment)
    {
        m_renderPass.m_depthStencilAttachment = attachment;
        UseResource(*attachment.view, {}, ImageUsage::DepthStencil, PipelineStage::EarlyFragmentTests | PipelineStage::LateFragmentTests, Access::ReadWrite);
    }

    TL::Span<const BarrierInfo> Pass::GetMemoryBarriers(BarrierSlot slot) const
    {
        return m_memoryBarriers[slot];
    }

    TL::Span<const ImageBarrierInfo> Pass::GetImageBarriers(BarrierSlot slot) const
    {
        return m_imageBarriers[slot];
    }

    TL::Span<const BufferBarrierInfo> Pass::GetBufferBarriers(BarrierSlot slot) const
    {
        return m_bufferBarriers[slot];
    }

    void Pass::PrepareBarriers()
    {
        for (auto imageTransition : m_imageTransitions)
        {
            auto  image               = (RenderGraphImage*)imageTransition->resource;
            auto& imageBarrier        = m_imageBarriers[BarrierSlot_Prilogue].emplace_back();
            imageBarrier.image        = image->GetImage();
            imageBarrier.srcState     = {imageTransition->usage, imageTransition->stage, imageTransition->access};
            imageBarrier.dstState     = {imageTransition->usage, imageTransition->stage, imageTransition->access};
            imageBarrier.subresources = imageTransition->subresourceRange;
        }

        for (auto bufferTransition : m_bufferTransitions)
        {
            auto buffer = (RenderGraphBuffer*)bufferTransition->resource;

            auto& bufferBarrier     = m_bufferBarriers[BarrierSlot_Prilogue].emplace_back();
            bufferBarrier.buffer    = buffer->GetBuffer();
            bufferBarrier.srcState  = {bufferTransition->usage, bufferTransition->stage, bufferTransition->access};
            bufferBarrier.dstState  = {bufferTransition->usage, bufferTransition->stage, bufferTransition->access};
            bufferBarrier.subregion = bufferTransition->subregion;
        }
    }

} // namespace RHI