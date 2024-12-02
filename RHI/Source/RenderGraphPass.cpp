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
        , m_size({0, 0}) // Default size is empty.
        , m_colorAttachments(*allocator)
        , m_depthStencilAttachment()
        , m_accessedResources(*allocator)
    {
    }

    const char* Pass::GetName() const
    {
        // return m_name.c_str();
        return m_name;
    }

    void Pass::Resize(ImageSize2D size)
    {
        m_size = size;
    }

    ImageSize2D Pass::GetSize() const
    {
        return m_size;
    }

    TL::Span<const RenderTargetInfo> Pass::GetColorAttachment() const
    {
        return m_colorAttachments;
    }

    const RenderTargetInfo* Pass::GetDepthStencilAttachment() const
    {
        return m_depthStencilAttachment ? &(*m_depthStencilAttachment) : nullptr;
    }

    TL::Span<PassAccessedResource* const> Pass::GetAccessedResources() const
    {
        return m_accessedResources;
    }

    PassAccessedResource* Pass::AddResourceAccess(TL::IAllocator& allocator)
    {
        auto* newResource = allocator.Construct<PassAccessedResource>();
        m_accessedResources.push_back(newResource);
        return newResource;
    }
} // namespace RHI
