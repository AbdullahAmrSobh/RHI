#include "RHI/RenderGraphResources.hpp"

#include "RHI/Format.hpp"

namespace RHI
{
    RenderGraphResource::RenderGraphResource(const char* name, Type type)
        : m_name(name)
        , m_first(nullptr)
        , m_last(nullptr)
        , m_type(type)
        , m_format(Format::Unknown)
        , m_handle(NullHandle)
        , m_usage({})
    {
        m_handle.asImage = {};
        m_usage.asImage  = {};
    }

    void RenderGraphResource::PushAccess(GraphTransition* access)
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

        switch (access->resource->m_type)
        {
        case Type::Image:
            m_usage.asImage |= static_cast<ImageGraphTransition*>(access)->usage;
            break;
        case Type::Buffer:
            m_usage.asBuffer |= static_cast<BufferGraphTransition*>(access)->usage;
            break;
        }
    }

    RenderGraphImage::RenderGraphImage(const char* name, Handle<Image> image, Format format)
        : RenderGraphResource(name, Type::Image)
    {
        m_handle.asImage = image;
        m_format         = format;
    }

    RenderGraphImage::RenderGraphImage(const char* name, Format format)
        : RenderGraphResource(name, Type::Image)
    {
        m_format = format;
    }

    RenderGraphBuffer::RenderGraphBuffer(const char* name, Handle<Buffer> buffer)
        : RenderGraphResource(name, Type::Buffer)
    {
        m_handle.asBuffer = buffer;
    }

    RenderGraphBuffer::RenderGraphBuffer(const char* name)
        : RenderGraphResource(name, Type::Buffer)
    {
    }
} // namespace RHI
