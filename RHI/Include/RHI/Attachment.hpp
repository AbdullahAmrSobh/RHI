#pragma once

#include <RHI/Common/Handle.hpp>
#include <RHI/Common/Flags.hpp>
#include <RHI/Common/Ptr.hpp>
#include <RHI/Common/Containers.h>
#include <RHI/Definitions.hpp>
#include <RHI/Resources.hpp>

#include <cstdint>

namespace RHI
{
    class Swapchain;
    class Pass;

    enum class AttachmentType : uint8_t
    {
        None,
        Imported,
        Transient,
        Swapchain,
    };

    struct ImageViewInfo
    {
        ImageViewType         type;
        ImageSubresourceRange subresources;
        ComponentMapping      swizzle;
    };

    struct BufferViewInfo
    {
        BufferSubregion subregion;
        Format          format;
    };

    struct ImagePassAttachment
    {
        Handle<Pass>                   pass;
        Handle<struct ImageAttachment> attachment;
        ImageUsage                     usage;
        Access                         access;
        Flags<ShaderStage>             stages;
        ImageViewInfo                  viewInfo;
        ImagePassAttachment*           next;
        ImagePassAttachment*           prev;
    };

    struct BufferPassAttachment
    {
        Handle<Pass>                    pass;
        Handle<struct BufferAttachment> attachment;
        BufferUsage                     usage;
        Access                          access;
        Flags<ShaderStage>              stages;
        BufferViewInfo                  viewInfo;
        BufferPassAttachment*           next;
        BufferPassAttachment*           prev;
    };

    struct ImageAttachment
    {
        TL::String           name;
        Handle<Image>        resource;
        Swapchain*           swapchain;
        ImageCreateInfo      info;
        ImagePassAttachment* first;
        ImagePassAttachment* last;
        bool                 isTransient;

        TL::UnorderedMap<Handle<Pass>, ImagePassAttachment*> list;

        ImagePassAttachment* Find(Handle<Pass> pass) const
        {
            if (auto it = list.find(pass); it != list.end())
            {
                return it->second;
            }

            return nullptr;
        }
    };

    struct BufferAttachment
    {
        TL::String            name;
        Handle<Buffer>        resource;
        BufferCreateInfo      info;
        BufferPassAttachment* first;
        BufferPassAttachment* last;

        TL::UnorderedMap<Handle<Pass>, BufferPassAttachment*> list;

        BufferPassAttachment* Find(Handle<Pass> pass) const
        {
            if (auto it = list.find(pass); it != list.end())
            {
                return it->second;
            }

            return nullptr;
        }
    };

} // namespace RHI