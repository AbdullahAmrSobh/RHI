#pragma once

#include <cstdint>
#include <type_traits>

namespace RHI
{
    enum QueueType : uint32_t
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
        ClearValue();

        template<typename T>
        ClearValue(ColorValue<T> value);

        template<typename T>
        ClearValue(T r, T g, T b, T a);

        ClearValue(DepthStencilValue value);

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

namespace RHI
{
    inline ClearValue::ClearValue()
    {
        f32.r = 0;
        f32.g = 0;
        f32.b = 0;
        f32.a = 1;
    }

    template<typename T>
    inline ClearValue::ClearValue(ColorValue<T> value)
    {
        static_assert(std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, float>, "invalid T argument");

        if constexpr (std::is_same_v<T, uint8_t>)
            u8 = value;
        else if constexpr (std::is_same_v<T, uint16_t>)
            u16 = value;
        else if constexpr (std::is_same_v<T, uint32_t>)
            u32 = value;
        else if constexpr (std::is_same_v<T, float>)
            f32 = value;
    }

    template<typename T>
    inline ClearValue::ClearValue(T r, T g, T b, T a)
    {
        static_assert(std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, float>, "invalid T argument");

        if constexpr (std::is_same_v<T, uint8_t>)
        {
            u8.r = r;
            u8.g = g;
            u8.b = b;
            u8.a = a;
        }
        else if constexpr (std::is_same_v<T, uint16_t>)
        {
            u16.r = r;
            u16.g = g;
            u16.b = b;
            u16.a = a;
        }
        else if constexpr (std::is_same_v<T, uint32_t>)
        {
            u32.r = r;
            u32.g = g;
            u32.b = b;
            u32.a = a;
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            f32.r = r;
            f32.g = g;
            f32.b = b;
            f32.a = a;
        }
    }

    inline ClearValue::ClearValue(DepthStencilValue value)
        : depthStencil(value)
    {
    }
} // namespace RHI