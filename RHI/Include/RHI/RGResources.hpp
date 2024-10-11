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

    enum class RGResourceType : uint8_t
    {
        None,
        Imported,
        Transient,
        Swapchain,
    };

    struct RGImageCreateInfo
    {
        const char* name;        ///< Name of the image.
        ImageType   type;        ///< Type of the image.
        ImageSize3D size;        ///< Size of the image.
        Format      format;      ///< Format of the image.
        SampleCount sampleCount; ///< Number of samples per pixel.
        uint32_t    mipLevels;   ///< Number of mipmap levels.
        uint32_t    arrayCount;  ///< Number of array layers.
    };

    struct RGBufferCreateInfo
    {
        const char* name;     ///< Name of the buffer.
        size_t      byteSize; ///< Size of the buffer in bytes.
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
        TL::Flags<Access>              pipelineAccess;
        TL::Flags<PipelineStage>       pipelineStages;
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
        TL::Flags<Access>               pipelineAccess;
        TL::Flags<PipelineStage>        pipelineStages;
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