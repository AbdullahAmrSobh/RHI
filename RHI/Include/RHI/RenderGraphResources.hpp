#pragma once

#include "RHI/Handle.hpp"
#include "RHI/Image.hpp"
#include "RHI/Buffer.hpp"
#include "RHI/BindGroup.hpp"
#include "RHI/PipelineAccess.hpp"
#include "RHI/RenderTarget.hpp"

#include <TL/Flags.hpp>
#include <TL/Containers.hpp>
#include <TL/UniquePtr.hpp>

#include <cstdint>

namespace RHI
{
    struct RenderGraphImage;
    struct RenderGraphBuffer;

    class Swapchain;
    class Pass;

    enum class RenderGraphResourceFlags : uint8_t
    {
        None,
        Imported,
        Transient,
        Swapchain,
    };

    enum class RendergraphResourceType
    {
        None,
        Image,
        Buffer,
        RenderTarget,
    };

    struct RenderGraphImagePassAccess
    {
        Handle<Pass>                pass;
        Handle<RenderGraphImage>    image;
        Handle<Image>               imageView;
        ImageUsage                  usage;
        TL::Flags<Access>           pipelineAccess;
        TL::Flags<PipelineStage>    pipelineStages;
        RenderGraphImagePassAccess* next;
        RenderGraphImagePassAccess* prev;
    };

    struct RenderGraphBufferPassAccess
    {
        Handle<Pass>                 pass;
        Handle<RenderGraphBuffer>    buffer;
        Handle<Buffer>               bufferView;
        BufferUsage                  usage;
        TL::Flags<Access>            pipelineAccess;
        TL::Flags<PipelineStage>     pipelineStages;
        RenderGraphBufferPassAccess* next;
        RenderGraphBufferPassAccess* prev;
    };

    struct RenderGraphResource
    {
    };

    struct RenderGraphImage : RenderGraphResource
    {
        TL::String                  name;
        Handle<Image>               resource;
        Swapchain*                  swapchain;
        RenderGraphImagePassAccess* first;
        RenderGraphImagePassAccess* last;
    };

    struct RenderGraphBuffer : RenderGraphResource
    {
        TL::String                   name;
        Handle<Buffer>               resource;
        RenderGraphBufferPassAccess* first;
        RenderGraphBufferPassAccess* last;
    };

} // namespace RHI