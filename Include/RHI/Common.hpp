#pragma once
#include "RHI/Pch.hpp"

namespace RHI
{

enum class ResultCode
{
    Success,
    Fail,
    Timeout,
    NotReady,
    HostOutOfMemory,
    DeviceOutOfMemory,
    ExtensionNotAvailable,
    InvalidArguments,
    FeatureNotAvailable,
    InvalidObject,
};

inline size_t hash_combine(std::size_t first, std::size_t second)
{
    size_t                   seed = first;
    static std::hash<size_t> hasher;
    seed ^= hasher(second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

template<typename Descriptor>
constexpr inline size_t HashDescriptor(const Descriptor& descriptor, size_t seed = 0)
{
    std::hash<uint8_t> hasher {};
    auto               bytes = std::bit_cast<std::array<uint8_t, sizeof(Descriptor)>>(descriptor);
    for (uint8_t byte : bytes)
    {
        seed = hash_combine(seed, hasher(byte));
    }
    return seed;
}

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

template<typename FlagBitsType>
struct FlagTraits
{
    enum
    {
        allFlags = 0
    };
};

template<typename BitType>
class Flags
{
public:
    using MaskType = typename std::underlying_type<BitType>::type;

    // constructors
    constexpr Flags() noexcept
        : m_mask(0)
    {
    }

    constexpr Flags(BitType bit) noexcept
        : m_mask(static_cast<MaskType>(bit))
    {
    }

    constexpr Flags(Flags<BitType> const& rhs) noexcept = default;

    constexpr explicit Flags(MaskType flags) noexcept
        : m_mask(flags)
    {
    }

    // relational operators
#if defined(RHI_ENABLE_SPACESHIP_OPERATOR)
    auto operator<=>(Flags<BitType> const&) const = default;
#else
    constexpr bool operator<(Flags<BitType> const& rhs) const noexcept
    {
        return m_mask < rhs.m_mask;
    }

    constexpr bool operator<=(Flags<BitType> const& rhs) const noexcept
    {
        return m_mask <= rhs.m_mask;
    }

    constexpr bool operator>(Flags<BitType> const& rhs) const noexcept
    {
        return m_mask > rhs.m_mask;
    }

    constexpr bool operator>=(Flags<BitType> const& rhs) const noexcept
    {
        return m_mask >= rhs.m_mask;
    }

    constexpr bool operator==(Flags<BitType> const& rhs) const noexcept
    {
        return m_mask == rhs.m_mask;
    }

    constexpr bool operator!=(Flags<BitType> const& rhs) const noexcept
    {
        return m_mask != rhs.m_mask;
    }
#endif

    // logical operator
    constexpr bool operator!() const noexcept
    {
        return !m_mask;
    }

    // bitwise operators
    constexpr Flags<BitType> operator&(Flags<BitType> const& rhs) const noexcept
    {
        return Flags<BitType>(m_mask & rhs.m_mask);
    }

    constexpr Flags<BitType> operator|(Flags<BitType> const& rhs) const noexcept
    {
        return Flags<BitType>(m_mask | rhs.m_mask);
    }

    constexpr Flags<BitType> operator^(Flags<BitType> const& rhs) const noexcept
    {
        return Flags<BitType>(m_mask ^ rhs.m_mask);
    }

    constexpr Flags<BitType> operator~() const noexcept
    {
        return Flags<BitType>(m_mask ^ FlagTraits<BitType>::allFlags);
    }

    // assignment operators
    constexpr Flags<BitType>& operator=(Flags<BitType> const& rhs) noexcept = default;

    constexpr Flags<BitType>& operator|=(Flags<BitType> const& rhs) noexcept
    {
        m_mask |= rhs.m_mask;
        return *this;
    }

    constexpr Flags<BitType>& operator&=(Flags<BitType> const& rhs) noexcept
    {
        m_mask &= rhs.m_mask;
        return *this;
    }

    constexpr Flags<BitType>& operator^=(Flags<BitType> const& rhs) noexcept
    {
        m_mask ^= rhs.m_mask;
        return *this;
    }

    // cast operators
    explicit constexpr operator bool() const noexcept
    {
        return !!m_mask;
    }

    explicit constexpr operator MaskType() const noexcept
    {
        return m_mask;
    }

private:
    MaskType m_mask;
};

template<typename T>
inline constexpr uint32_t CountElements(const T& t)
{
    return static_cast<uint32_t>(t.size());
}

enum class AccessType
{
    Read,
    ReadWrite,
};

struct Extent2D
{
    uint32_t sizeX;
    uint32_t sizeY;
};

struct Extent3D
{
    uint32_t sizeX;
    uint32_t sizeY;
    uint32_t sizeZ;
};

struct Rect
{
    int32_t  x;
    int32_t  y;
    uint32_t sizeX;
    uint32_t sizeY;
};

struct Viewport
{
    Rect  drawingArea;
    float minDepth;
    float maxDepth;
};

template<typename T>
inline constexpr T Select(std::span<const T> range, T desired, T fallback)
{
    for (T t : range)
        if (t == desired)
            return t;

    return fallback;
}

}  // namespace RHI
