#pragma once

#include <cstdint>
#include <type_traits>
#include <memory>

namespace RHI
{
    enum class LoadOperation
    {
        DontCare, // The attachment load operation undefined.
        Load,     // Load attachment content.
        Discard,  // Discard attachment content.
    };

    enum class StoreOperation
    {
        DontCare, // Attachment Store operation is undefined
        Store,    // Writes to the attachment are stored
        Discard,  // Writes to the attachment are discarded
    };

    struct LoadStoreOperations
    {
        LoadOperation  loadOperation  = LoadOperation::Discard;
        StoreOperation storeOperation = StoreOperation::Store;

        inline bool    operator==(const LoadStoreOperations& other) const
        {
            return loadOperation == other.loadOperation && storeOperation == other.storeOperation;
        }

        inline bool operator!=(const LoadStoreOperations& other) const
        {
            return !(*this == other);
        }
    };

    struct DepthStencilValue
    {
        float       depthValue   = 1.0f;
        uint8_t     stencilValue = 0xff;

        inline bool operator==(const DepthStencilValue& other) const
        {
            return depthValue == other.depthValue && stencilValue == other.stencilValue;
        }

        inline bool operator!=(const DepthStencilValue& other) const
        {
            return !(*this == other);
        }
    };

    template<typename T>
    struct ColorValue
    {
        ColorValue() = default;

        ColorValue(T r = {}, T g = {}, T b = {}, T a = {})
            : r(r)
            , g(g)
            , b(b)
            , a(a)
        {
        }

        T           r, g, b, a;

        inline bool operator==(const ColorValue& other) const
        {
            return r == other.r && g == other.g && b == other.b && a == other.a;
        }

        inline bool operator!=(const ColorValue& other) const
        {
            return !(*this == other);
        }
    };

    union ClearValue
    {
        ClearValue()
        {
            memset(this, 0, sizeof(ClearValue));
        }

        template<typename T>
        ClearValue(ColorValue<T> value)
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
        ClearValue(T r, T g, T b, T a)
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

        ClearValue(DepthStencilValue value)
            : depthStencil(value)
        {
        }

        ColorValue<uint8_t>  u8;
        ColorValue<uint16_t> u16;
        ColorValue<uint32_t> u32;
        ColorValue<float>    f32;
        DepthStencilValue    depthStencil;
    };
} // namespace RHI