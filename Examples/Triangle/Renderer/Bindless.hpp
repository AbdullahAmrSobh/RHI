#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>

namespace Engine
{
    using BindlessTextureHandle = uint32_t;

    class Bindless
    {
    public:
        BindlessTextureHandle CreateTexture(const char* name, RHI::ImageSize2D size, RHI::Format format, TL::Block data);

        void DestroyTexture(BindlessTextureHandle handle);

        RHI::Handle<RHI::Image> GetTexture(BindlessTextureHandle handle) const;

        TL::Span<const RHI::Handle<RHI::Image>> GetTextures() const;

    private:
        RHI::Device*                        m_device;
        TL::Vector<RHI::Handle<RHI::Image>> m_images;
        TL::Vector<uint32_t>                m_freeSlots;
        bool                                m_dirty;
    };
} // namespace Engine