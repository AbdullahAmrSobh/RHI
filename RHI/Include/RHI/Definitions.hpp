#pragma once

#include <cstdint>

namespace RHI
{
    // clear values

    struct DepthStencilValue
    {
        float   depthValue;
        uint8_t stencilValue;
    };

    template<typename T>
    struct ColorValue
    {
        T r, g, b, a;
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

    struct LoadStoreOperations
    {
        ClearValue     clearValue;
        LoadOperation  loadOperation;
        StoreOperation storeOperation;
    };
} // namespace RHI
