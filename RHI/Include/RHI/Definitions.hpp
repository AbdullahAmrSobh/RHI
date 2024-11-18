#pragma once

#include "RHI/Handle.hpp"

#include <cstdint>

namespace RHI
{
    struct Image;

    // clear values

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

    enum class ResolveMode
    {
        None,
        Min,
        Max,
        Avg,
    };

    struct DepthStencilValue
    {
        float   depthValue;
        uint8_t stencilValue;
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
        DepthStencilValue    depthStencil;
    };

    // load store operations

    struct ColorAttachmentInfo
    {
        Handle<Image>  attachment        = NullHandle;
        ClearValue     clearValue        = {};
        LoadOperation  loadOperation     = LoadOperation::Discard;
        StoreOperation storeOperation    = StoreOperation::Store;
        ResolveMode    resolveMode       = ResolveMode::None;
        Handle<Image>  resolveAttachment = NullHandle;
    };

    struct DepthAttachmentInfo
    {
        Handle<Image>     attachment            = NullHandle;
        DepthStencilValue clearValue            = {1.0f, 0u};
        LoadOperation     depthLoadOperation    = LoadOperation::Discard;
        StoreOperation    depthStoreOperation   = StoreOperation::Store;
        LoadOperation     stencilLoadOperation  = LoadOperation::Discard;
        StoreOperation    stencilStoreOperation = StoreOperation::Store;
        ResolveMode       resolveMode           = ResolveMode::None;
        Handle<Image>     resolveAttachment     = NullHandle;
    };
} // namespace RHI
