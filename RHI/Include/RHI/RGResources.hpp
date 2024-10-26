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
    struct RGImage;
    struct RGBuffer;

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

    struct RGImagePassAccess
    {
        Handle<Pass>             pass;
        Handle<RGImage>          image;
        ImageUsage               usage;
        TL::Flags<Access>        pipelineAccess;
        TL::Flags<PipelineStage> pipelineStages;
        ImageViewInfo            viewInfo;
        RGImagePassAccess*       next;
        RGImagePassAccess*       prev;
        LoadStoreOperations      loadStoreOperation;
    };

    struct RGBufferPassAccess
    {
        Handle<Pass>             pass;
        Handle<RGBuffer>         buffer;
        BufferUsage              usage;
        TL::Flags<Access>        pipelineAccess;
        TL::Flags<PipelineStage> pipelineStages;
        BufferViewInfo           viewInfo;
        RGBufferPassAccess*      next;
        RGBufferPassAccess*      prev;
    };

    struct RGImage
    {
        TL::String                                         name;
        Handle<Image>                                      resource;
        Swapchain*                                         swapchain;
        ImageCreateInfo                                    info;
        RGImagePassAccess*                                 first;
        RGImagePassAccess*                                 last;
        bool                                               isTransient;

        TL::UnorderedMap<Handle<Pass>, RGImagePassAccess*> list;

        RGImagePassAccess*                                 Find(Handle<Pass> pass) const
        {
            if (auto it = list.find(pass); it != list.end())
            {
                return it->second;
            }

            return nullptr;
        }
    };

    struct RGBuffer
    {
        TL::String                                          name;
        Handle<Buffer>                                      resource;
        BufferCreateInfo                                    info;
        RGBufferPassAccess*                                 first;
        RGBufferPassAccess*                                 last;

        TL::UnorderedMap<Handle<Pass>, RGBufferPassAccess*> list;

        RGBufferPassAccess*                                 Find(Handle<Pass> pass) const
        {
            if (auto it = list.find(pass); it != list.end())
            {
                return it->second;
            }

            return nullptr;
        }
    };

} // namespace RHI