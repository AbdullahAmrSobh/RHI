#pragma once

#include <TL/Assert.hpp>

#include <string>

#define RHI_DEFINE_POD_HASH(Type)                       \
    template<>                                          \
    struct hash<Type>                                   \
    {                                                   \
        std::size_t operator()(const Type& value) const \
        {                                               \
            return ::RHI::HashAny<Type>(value);         \
        }                                               \
    };

namespace RHI
{
    template<typename T>
    inline static bool IsPow2(T x)
    {
        return (x & (x - 1)) == 0;
    }

    template<typename T>
    inline static T AlignUp(T val, T alignment)
    {
        TL_ASSERT(IsPow2(alignment));
        return (val + alignment - 1) & ~(alignment - 1);
    }

    template<typename T>
    inline static uint64_t HashAny(const T& data)
    {
        auto stream = std::string(reinterpret_cast<const char*>(&data), sizeof(data));
        auto hasher = std::hash<std::string>{};
        return hasher(stream);
    }

    inline static uint64_t HashCombine(uint64_t seed, uint64_t value)
    {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }

} // namespace RHI