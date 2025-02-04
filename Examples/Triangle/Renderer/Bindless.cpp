#include "Bindless.hpp"

namespace Engine
{
    BindlessTextureHandle Bindless::CreateTexture(const char* name, RHI::ImageSize2D size, RHI::Format format, TL::Block data)
    {
        BindlessTextureHandle handle;
        if (m_freeSlots.empty())
        {
            handle = static_cast<BindlessTextureHandle>(m_images.size());
            m_images.push_back({});
        }
        else
        {
            handle = m_freeSlots.back();
            m_freeSlots.pop_back();
        }

        RHI::ImageCreateInfo createInfo{
            .name       = name,
            .usageFlags = RHI::ImageUsage::CopyDst | RHI::ImageUsage::ShaderResource,
            .type       = RHI::ImageType::Image2D,
            .size       = {size.width, size.height, 1},
            .format     = format,
        };
        auto [image, result] = RHI::CreateImageWithContent(*m_device, createInfo, data);

        if (m_freeSlots.empty() == false)
        {
            handle = m_freeSlots.back();
            m_freeSlots.pop_back();
            m_images[handle] = image;
        }
        else
        {
            handle = m_images.size();
            m_images.push_back(image);
        }

        m_dirty = true;

        return handle;
    }

    void Bindless::DestroyTexture(BindlessTextureHandle handle)
    {
        m_device->DestroyImage(m_images[handle]);
        m_freeSlots.push_back(handle);

        m_dirty = true;
    }

    RHI::Handle<RHI::Image> Bindless::GetTexture(BindlessTextureHandle handle) const
    {
        return m_images[handle];
    }

    TL::Span<const RHI::Handle<RHI::Image>> Bindless::GetTextures() const
    {
        return m_images;
    }
} // namespace Engine