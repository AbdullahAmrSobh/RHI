#pragma once
#include <cstdint>

namespace RHI
{
    // Collection of common definitions

    enum class QueueType
    {
        Graphics,
        Compute,
        Transfer,
        Count,
    };

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
        // clang-format off
        ClearValue() { f32 = {}; }
        ClearValue(ColorValue<uint8_t> value)                      : u8(value)           { }
        ClearValue(ColorValue<uint16_t> value)                     : u16(value)          { }
        ClearValue(ColorValue<uint32_t> value)                     : u32(value)          { }
        ClearValue(ColorValue<float> value)                        : f32(value)          { }
        ClearValue(uint8_t r, uint8_t g, uint8_t b, uint8_t a)     : u8{ r, g, b, a }    { }
        ClearValue(uint16_t r, uint16_t g, uint16_t b, uint16_t a) : u16{ r, g, b, a }   { }
        ClearValue(uint32_t r, uint32_t g, uint32_t b, uint32_t a) : u32{ r, g, b, a }   { }
        ClearValue(float r, float g, float b, float a)             : f32{ r, g, b, a }   { }
        ClearValue(DepthStencilValue value)                        : depthStencil(value) { }

        // clang-format on

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
        ClearValue     clearValue     = ClearValue();
        LoadOperation  loadOperation  = LoadOperation::Discard;
        StoreOperation storeOperation = StoreOperation::Store;
    };

} // namespace RHI