#pragma once

#include "RHI/Handle.hpp"
#include "RHI/PipelineAccess.hpp"

#include <cstdint>

namespace RHI
{
    struct Image;
    class RenderGraphImage;

    enum class LoadOperation : uint8_t
    {
        DontCare, // The attachment load operation undefined.
        Load,     // Load attachment content.
        Discard,  // Discard attachment content.
    };

    enum class StoreOperation : uint8_t
    {
        DontCare, // Attachment Store operation is undefined
        Store,    // Writes to the attachment are stored
        Discard,  // Writes to the attachment are discarded
    };

    enum class ResolveMode : uint8_t
    {
        None,
        Min,
        Max,
        Avg,
    };

    struct DepthStencilValue
    {
        float   depthValue   = 1.0f;
        uint8_t stencilValue = 0u;
    };

    template<typename T>
    struct ColorValue
    {
        T r = 1;
        T g = 1;
        T b = 1;
        T a = 1;
    };

    union ClearValue
    {
        ColorValue<uint8_t>  u8;
        ColorValue<uint16_t> u16;
        ColorValue<uint32_t> u32;
        ColorValue<float>    f32;
    };

    struct ColorAttachment
    {
        Handle<Image>  view        = NullHandle;
        LoadOperation  loadOp      = LoadOperation::Discard;
        StoreOperation storeOp     = StoreOperation::Store;
        ClearValue     clearValue  = {.f32 = {0.0f, 0.0f, 0.0f, 1.0f}};
        ResolveMode    resolveMode = ResolveMode::None;
        Handle<Image>  resolveView = NullHandle;
    };

    struct DepthStencilAttachment
    {
        Handle<Image>     view           = NullHandle;
        LoadOperation     depthLoadOp    = LoadOperation::Discard;
        StoreOperation    depthStoreOp   = StoreOperation::Store;
        LoadOperation     stencilLoadOp  = LoadOperation::Discard;
        StoreOperation    stencilStoreOp = StoreOperation::Store;
        DepthStencilValue clearValue     = {0.0f, 0};
    };

    struct RenderPassBeginInfo
    {
        TL::Span<const ColorAttachment>      colorAttachments;
        TL::Optional<DepthStencilAttachment> depthStencilAttachment;
    };

    inline static RHI::Access LoadStoreToAccess(LoadOperation loadOp, StoreOperation storeOp)
    {
        if (loadOp == LoadOperation::Load && storeOp == StoreOperation::Store) return RHI::Access::ReadWrite;
        else if (loadOp == LoadOperation::Load && storeOp != StoreOperation::Store) return RHI::Access::Read;
        else return RHI::Access::Write;
    }
} // namespace RHI
