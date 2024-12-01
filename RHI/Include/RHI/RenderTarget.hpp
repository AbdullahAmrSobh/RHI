#pragma once

#include "RHI/Handle.hpp"

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
        DepthStencilValue    depthStencil;
    };

    struct RenderTargetInfo
    {
        RenderGraphImage* attachment            = nullptr;
        ClearValue        clearValue            = {};
        LoadOperation     loadOperation         = LoadOperation::Discard;
        StoreOperation    storeOperation        = StoreOperation::Store;
        LoadOperation     stencilLoadOperation  = LoadOperation::Discard;
        StoreOperation    stencilStoreOperation = StoreOperation::Store;
        RenderGraphImage* resolveAttachment     = nullptr;
        ResolveMode       resolveMode           = ResolveMode::None;
    };
} // namespace RHI
