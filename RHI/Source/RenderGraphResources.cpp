#include "RHI/RenderGraphResources.hpp"

#include "RHI/Format.hpp"

namespace RHI
{
    RenderGraphResource::RenderGraphResource(const char* name, RenderGraphResourceAccessType type, TL::Flags<RenderGraphResourceFlags> flags)
        : m_name(name)
        , m_first(nullptr)
        , m_last(nullptr)
        , m_flags(flags)
        , m_type(type)
        , m_format(Format::Unknown)
        , m_handle(NullHandle)
        , m_usage({})
    {
        m_handle.asImage = {};
        m_usage.asImage  = {};
    }

    void RenderGraphResource::PushAccess(AccessedResource* access)
    {
        if (!m_last)
        {
            m_first = m_last = access;
        }
        else
        {
            access->prev = m_last;
            m_last->next = access;
            m_last       = access;
        }

        switch (access->type)
        {
        case RenderGraphResourceAccessType::None:
            TL_UNREACHABLE();
            break;
        case RenderGraphResourceAccessType::Image:
            m_usage.asImage |= access->asImage.usage;
            break;
        case RenderGraphResourceAccessType::Buffer:
            m_usage.asBuffer |= access->asBuffer.usage;
            break;
        case RenderGraphResourceAccessType::RenderTarget:
            {
                auto formatInfo = GetFormatInfo(m_format);
                if (formatInfo.hasDepth || formatInfo.hasStencil)
                {
                    if (formatInfo.hasDepth) m_usage.asImage |= ImageUsage::Depth;
                    if (formatInfo.hasStencil) m_usage.asImage |= ImageUsage::Stencil;
                }
                else
                {
                    m_usage.asImage |= ImageUsage::Color;
                }
                break;
            }
        case RenderGraphResourceAccessType::Resolve:
        case RenderGraphResourceAccessType::SwapchainPresent: break;
        }
    }

    RenderGraphImage::RenderGraphImage(const char* name, Handle<Image> image, Format format)
        : RenderGraphResource(name, RenderGraphResourceAccessType::Image, RenderGraphResourceFlags::Imported)
    {
        m_handle.asImage = image;
        m_format         = format;
    }

    RenderGraphImage::RenderGraphImage(const char* name, Format format)
        : RenderGraphResource(name, RenderGraphResourceAccessType::Image, RenderGraphResourceFlags::Transient)
    {
        m_format = format;
    }

    RenderGraphBuffer::RenderGraphBuffer(const char* name, Handle<Buffer> buffer)
        : RenderGraphResource(name, RenderGraphResourceAccessType::Buffer, RenderGraphResourceFlags::Imported)
    {
        m_handle.asBuffer = buffer;
    }

    RenderGraphBuffer::RenderGraphBuffer(const char* name)
        : RenderGraphResource(name, RenderGraphResourceAccessType::Buffer, RenderGraphResourceFlags::Transient)
    {
    }
} // namespace RHI
