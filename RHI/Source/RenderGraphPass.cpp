#include "RHI/RenderGraphPass.hpp"

#include "RHI/RenderGraphResources.hpp"

namespace RHI
{
    Pass::Pass(const PassCreateInfo& createInfo, TL::IAllocator* allocator) noexcept
        : m_name(createInfo.name)
        , m_queueType(QueueType::Graphics)
        , m_onSetupCallback(createInfo.setupCallback)
        , m_onCompileCallback(createInfo.compileCallback)
        , m_onExecuteCallback(createInfo.executeCallback)
        , m_size(createInfo.size)
        , m_colorAttachments(*allocator)
        , m_depthStencilAttachment()
        , m_resourceTransitions(*allocator)
    {
    }

    const char* Pass::GetName() const
    {
        // return m_name.c_str();
        return m_name;
    }

    ImageSize2D Pass::GetSize() const
    {
        return m_size;
    }

    TL::Span<const ColorRGAttachment> Pass::GetColorAttachment() const
    {
        return m_colorAttachments;
    }

    const DepthStencilRGAttachment* Pass::GetDepthStencilAttachment() const
    {
        return m_depthStencilAttachment ? &(*m_depthStencilAttachment) : nullptr;
    }

    TL::Span<RenderGraphResourceTransition* const> Pass::GetRenderGraphResourceTransitions() const
    {
        return m_resourceTransitions;
    }

    /// @brief Adds a new resource access to the pass.
    RenderGraphResourceTransition* Pass::AddTransition(
        TL::IAllocator&          allocator,
        RenderGraphResource&     resource,
        ImageUsage               usage,
        TL::Flags<PipelineStage> stage,
        TL::Flags<Access>        access,
        ImageSubresourceRange    subresourceRange)
    {
        auto transition                      = m_resourceTransitions.emplace_back(allocator.Construct<RenderGraphResourceTransition>());
        transition->pass                     = this;
        transition->next                     = nullptr;
        transition->prev                     = resource.GetLastAccess();
        transition->resource                 = &resource;
        transition->asImage.usage            = usage;
        transition->asImage.stage            = stage;
        transition->asImage.access           = access;
        transition->asImage.subresourceRange = subresourceRange;
        resource.PushAccess(transition);
        return transition;
    }

    RenderGraphResourceTransition* Pass::AddTransition(
        TL::IAllocator&          allocator,
        RenderGraphResource&     resource,
        BufferUsage              usage,
        TL::Flags<PipelineStage> stage,
        TL::Flags<Access>        access,
        BufferSubregion          subregion)
    {
        auto transition                = m_resourceTransitions.emplace_back(allocator.Construct<RenderGraphResourceTransition>());
        transition->pass               = this;
        transition->next               = nullptr;
        transition->prev               = resource.GetLastAccess();
        transition->resource           = &resource;
        transition->asBuffer.usage     = usage;
        transition->asBuffer.stage     = stage;
        transition->asBuffer.access    = access;
        transition->asBuffer.subregion = subregion;
        resource.PushAccess(transition);
        return transition;
    }

} // namespace RHI
