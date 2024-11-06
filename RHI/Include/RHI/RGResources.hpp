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

    struct RGImagePassAccess
    {
        Handle<Pass>             pass;
        Handle<RGImage>          image;
        Handle<Image>            imageView;
        ImageUsage               usage;
        TL::Flags<Access>        pipelineAccess;
        TL::Flags<PipelineStage> pipelineStages;
        RGImagePassAccess*       next;
        RGImagePassAccess*       prev;
        LoadStoreOperations      loadStoreOperation;
        TL::Flags<ImageAspect>   aspect;
    };

    struct RGBufferPassAccess
    {
        Handle<Pass>             pass;
        Handle<RGBuffer>         buffer;
        Handle<Buffer>           bufferView;
        BufferUsage              usage;
        TL::Flags<Access>        pipelineAccess;
        TL::Flags<PipelineStage> pipelineStages;
        RGBufferPassAccess*      next;
        RGBufferPassAccess*      prev;
    };

    struct RGImage
    {
        TL::String         name;
        Handle<Image>      resource;
        Swapchain*         swapchain;
        ImageCreateInfo    info;
        RGImagePassAccess* first;
        RGImagePassAccess* last;
    };

    struct RGBuffer
    {
        TL::String          name;
        Handle<Buffer>      resource;
        BufferCreateInfo    info;
        RGBufferPassAccess* first;
        RGBufferPassAccess* last;
    };

} // namespace RHI