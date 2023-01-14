#pragma once
#include "RHI/Pch.hpp"

#include "RHI/Flag.hpp"
#include "RHI/ResultCode.hpp"

namespace RHI
{

template<typename T>
using Unique = std::unique_ptr<T>;

template<typename T>
using Shared = std::shared_ptr<T>;

template<typename T, typename... Args>
inline Unique<T> CreateUnique(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
inline Shared<T> CreateShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

using Unexpected = tl::unexpected<ResultCode>;
template<class T>
using Expected = tl::expected<T, ResultCode>;

template<typename T>
inline constexpr uint32_t CountElements(const T& t)
{
    return static_cast<uint32_t>(t.size());
}

template<typename T>
inline constexpr T Select(std::span<const T> range, T desired, T fallback)
{
    for (T t : range)
        if (t == desired)
            return t;

    return fallback;
}

inline size_t HashCombine(std::size_t first, std::size_t second)
{
    size_t                   seed = first;
    static std::hash<size_t> hasher;
    seed ^= hasher(second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

template<typename Descriptor>
inline constexpr size_t HashDescriptor(const Descriptor& descriptor, size_t seed = 0)
{
    std::hash<uint8_t> hasher {};
    auto               bytes = std::bit_cast<std::array<uint8_t, sizeof(Descriptor)>>(descriptor);
    for (uint8_t byte : bytes)
    {
        seed = HashCombine(seed, hasher(byte));
    }
    return seed;
}

}  // namespace RHI
