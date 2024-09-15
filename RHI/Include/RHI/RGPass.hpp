#pragma once

#include "RHI/Export.hpp"
#include "RHI/Image.hpp"
#include "RHI/RGResources.hpp"

#include <TL/Containers.hpp>

namespace RHI
{
    class CommandList;

    enum class PassFlags
    {
        None      = 0,
        Graphics  = 1 << 0,
        Compute   = 1 << 1,
        Transfer  = 1 << 2,
        Immeidate = 1 << 3
    };

    struct PassCreateInfo
    {
        const char*          name;
        TL::Flags<PassFlags> flags;
    };

    class RHI_EXPORT Pass final
    {
    public:
        // Pass();
        // Pass(Pass&&)      = default;
        // Pass(const Pass&) = delete;
        // ~Pass()           = default;

        const char*                       GetName() const;

        void                              Resize(ImageSize2D size);

        ImageSize2D                       GetSize() const;

        // void                              UseImage(Handle<ImageAttachment> attachment, const ImageViewInfo& viewInfo, ImageUsage usage, TL::Flags<ShaderStage> stage, Access access);

        // void                              UseBuffer(Handle<BufferAttachment> attachment, const BufferViewInfo& viewInfo, BufferUsage usage, TL::Flags<ShaderStage> stage, Access access);

        // TL_NODISCARD Handle<ImageView>    GetImageView(Handle<ImageAttachment> attachment) const;

        // TL_NODISCARD Handle<BufferView>   GetBufferView(Handle<BufferAttachment> attachment) const;

        TL::String                        m_name;
        ImageSize2D                       m_renderTargetSize;
        TL::Vector<ImagePassAttachment*>  m_colorAttachments;
        ImagePassAttachment*              m_depthStencilAttachment;
        TL::Vector<ImagePassAttachment*>  m_imageAttachments;
        TL::Vector<BufferPassAttachment*> m_bufferAttachments;
        TL::Vector<CommandList*>          m_commandLists;
    };

    inline const char* Pass::GetName() const
    {
        return m_name.c_str();
    }

    inline void Pass::Resize(ImageSize2D size)
    {
        m_renderTargetSize = size;
    }

    inline ImageSize2D Pass::GetSize() const
    {
        return m_renderTargetSize;
    }

    // inline void Pass::UseImage(Handle<ImageAttachment> attachment, const ImageViewInfo& viewInfo, ImageUsage usage, TL::Flags<ShaderStage> stage, Access access)
    // {
    // }

    // inline void Pass::UseBuffer(Handle<BufferAttachment> attachment, const BufferViewInfo& viewInfo, BufferUsage usage, TL::Flags<ShaderStage> stage, Access access)
    // {
    // }

    // inline Handle<ImageView> Pass::GetImageView(Handle<ImageAttachment> attachment) const
    // {
    // }

    // inline Handle<BufferView> Pass::GetBufferView(Handle<BufferAttachment> attachment) const
    // {
    // }

} // namespace RHI