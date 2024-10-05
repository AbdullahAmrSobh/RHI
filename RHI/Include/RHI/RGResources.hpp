#pragma once

#include <RHI/Handle.hpp>
#include <RHI/Definitions.hpp>
#include <RHI/Image.hpp>
#include <RHI/Buffer.hpp>
#include <RHI/BindGroup.hpp>

#include <TL/Flags.hpp>
#include <TL/Containers.hpp>
#include <TL/UniquePtr.hpp>

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

    struct PassAttachment
    {
        Handle<Pass>             pass;
        TL::Flags<Access>        access;        // how will this resource be accessed
        TL::Flags<PipelineStage> pipelineStage; // at which stage of the pipeline, it would be accessed
        TL::Flags<ShaderStage>   shaderStage;   // optional shader stage (if not present will assume shader stage = All graphics or all compute)
        BindingType              bindingType;   // optional binding element type (if not present will assume generic read/write)
        PassAttachment*          next;          //
        PassAttachment*          prev;          //
    };

    struct ImagePassAttachment
    {
        Handle<Pass>                   pass;
        Handle<struct ImageAttachment> attachment;
        ImageUsage                     usage;
        Access                         access;
        TL::Flags<ShaderStage>         stages;
        ImageViewInfo                  viewInfo;
        ImagePassAttachment*           next;
        ImagePassAttachment*           prev;
        LoadStoreOperations            loadStoreOperation;
    };

    struct BufferPassAttachment
    {
        Handle<Pass>                    pass;
        Handle<struct BufferAttachment> attachment;
        BufferUsage                     usage;
        Access                          access;
        TL::Flags<ShaderStage>          stages;
        BufferViewInfo                  viewInfo;
        BufferPassAttachment*           next;
        BufferPassAttachment*           prev;
    };

    struct ImageAttachment
    {
        TL::String                                           name;
        Handle<Image>                                        resource;
        Swapchain*                                           swapchain;
        ImageCreateInfo                                      info;
        ImagePassAttachment*                                 first;
        ImagePassAttachment*                                 last;
        bool                                                 isTransient;

        TL::UnorderedMap<Handle<Pass>, ImagePassAttachment*> list;

        ImagePassAttachment*                                 Find(Handle<Pass> pass) const
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
        TL::String                                            name;
        Handle<Buffer>                                        resource;
        BufferCreateInfo                                      info;
        BufferPassAttachment*                                 first;
        BufferPassAttachment*                                 last;

        TL::UnorderedMap<Handle<Pass>, BufferPassAttachment*> list;

        BufferPassAttachment*                                 Find(Handle<Pass> pass) const
        {
            if (auto it = list.find(pass); it != list.end())
            {
                return it->second;
            }

            return nullptr;
        }
    };

} // namespace RHI