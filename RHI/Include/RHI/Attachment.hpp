#pragma once

#include <RHI/Common/Handle.hpp>
#include <RHI/Common/Flags.hpp>
#include <RHI/Common/Ptr.hpp>
#include <RHI/Common/Containers.h>
#include <RHI/Definitions.hpp>
#include <RHI/Resources.hpp>
#include <RHI/RGInternals.hpp>

#include <cstdint>

namespace RHI
{
    class Swapchain;
    class Pass;

    inline static constexpr ImageSize2D SizeRelative2D = ImageSize2D{};

    enum class AttachmentLifetime : uint8_t
    {
        None,
        Imported,
        Transient,
        Swapchain,
    };

    struct ImageViewInfo
    {
        ImageViewType         viewType;
        ImageSubresourceRange subresourceLayers;
        ComponentMapping      componentMapping;
    };

    struct BufferViewInfo
    {
        BufferSubregion subregion;
        Format          format;
    };

    struct ImageAttachment;
    struct BufferAttachment;

    struct ImagePassAttachment;
    struct BufferPassAttachment;

    using ImageAttachmentList  = TL::List<ImagePassAttachment>;
    using BufferAttachmentList = TL::List<BufferPassAttachment>;

    using ImageAttachmentIterator  = ImageAttachmentList::iterator;
    using BufferAttachmentIterator = ImageAttachmentList::iterator;

    struct ImageAttachment
    {
        RenderGraph*        renderGraph;
        TL::String          name;
        AttachmentLifetime  lifetime;
        RGImageID           resource;
        ImageCreateInfo     info;
        ImageAttachmentList list;
    };

    struct BufferAttachment
    {
        RenderGraph*         renderGraph;
        TL::String           name;
        AttachmentLifetime   lifetime;
        RGBufferID           resource;
        BufferCreateInfo     info;
        BufferAttachmentList list;
    };

    struct ImagePassAttachment
    {
        Handle<Pass>            pass;
        Handle<ImageAttachment> attachment;
        Flags<ImageUsage>       usage;
        Flags<Access>           access;
        Flags<ShaderStage>      stages;
        ImageViewInfo           viewInfo;
        RGImageViewID           view;
        ImageAttachmentIterator it;
    };

    struct BufferPassAttachment
    {
        Handle<Pass>             pass;
        Handle<BufferAttachment> attachment;
        Flags<BufferUsage>       usage;
        Flags<Access>            access;
        Flags<ShaderStage>       stages;
        BufferViewInfo           viewInfo;
        RGImageViewID            view;
        BufferAttachmentIterator it;
    };

    inline static ImagePassAttachment* Emplace(ImageAttachment& attachment)
    {
        return &(attachment.list.emplace_back(ImagePassAttachment()));
    }

    inline static BufferPassAttachment* Emplace(BufferAttachment& attachment)
    {
        return &(attachment.list.emplace_back(BufferPassAttachment()));
    }
} // namespace RHI