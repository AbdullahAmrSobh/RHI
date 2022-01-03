#pragma once
#include "RHI/Common.hpp"
#include "RHI/Core/Expected.hpp"
#include <functional>

namespace RHI
{

enum class EBackendType
{
    None,
    Vulkan,
    D3D12,
    Metal,
};

enum class EResultCode
{
    Success,
    NotReady,
    Fail,
    Timeout,
    OutOfMemory,
    DeviceOutOfMemory,

    ExtensionNotPresent,
    LayerNotPresent,

    InvalidArguments,
    FailedToCreateDevice,
    DeviceLost,
};

enum class EDebugMessageSeverity
{
    Info,
    Warnnig,
    Error,
    Fatel,
};

template <class T>
using Expected = tl::expected<T, EResultCode>;

template <typename FlagBitsType>
struct FlagTraits
{
    enum
    {
        allFlags = 0
    };
};

template <typename BitType>
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
    constexpr bool operator<(Flags<BitType> const& rhs) const noexcept { return m_mask < rhs.m_mask; }

    constexpr bool operator<=(Flags<BitType> const& rhs) const noexcept { return m_mask <= rhs.m_mask; }

    constexpr bool operator>(Flags<BitType> const& rhs) const noexcept { return m_mask > rhs.m_mask; }

    constexpr bool operator>=(Flags<BitType> const& rhs) const noexcept { return m_mask >= rhs.m_mask; }

    constexpr bool operator==(Flags<BitType> const& rhs) const noexcept { return m_mask == rhs.m_mask; }

    constexpr bool operator!=(Flags<BitType> const& rhs) const noexcept { return m_mask != rhs.m_mask; }
#endif

    // logical operator
    constexpr bool operator!() const noexcept { return !m_mask; }

    // bitwise operators
    constexpr Flags<BitType> operator&(Flags<BitType> const& rhs) const noexcept { return Flags<BitType>(m_mask & rhs.m_mask); }

    constexpr Flags<BitType> operator|(Flags<BitType> const& rhs) const noexcept { return Flags<BitType>(m_mask | rhs.m_mask); }

    constexpr Flags<BitType> operator^(Flags<BitType> const& rhs) const noexcept { return Flags<BitType>(m_mask ^ rhs.m_mask); }

    constexpr Flags<BitType> operator~() const noexcept { return Flags<BitType>(m_mask ^ FlagTraits<BitType>::allFlags); }

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
    explicit constexpr operator bool() const noexcept { return !!m_mask; }

    explicit constexpr operator MaskType() const noexcept { return m_mask; }

private:
    MaskType m_mask;
};

class IDebugMessenger
{
public:
    virtual ~IDebugMessenger() = default;

    virtual void Info(std::string_view message)  = 0;
    virtual void Warn(std::string_view message)  = 0;
    virtual void Error(std::string_view message) = 0;
    virtual void Fatel(std::string_view message) = 0;
};

enum class EPixelFormat
{
    None,
    RGB,
    RGBA,
    BGRA,
    Depth,
    DepthStencil,
};

enum class ECompareOp
{
    Never          = 0,
    Less           = 1,
    Equal          = 2,
    LessOrEqual    = 3,
    Greater        = 4,
    NotEqual       = 5,
    GreaterOrEqual = 6,
    Always         = 7,

};

// Structs
struct Color
{
    float r, g, b, a;

    Color()
        : r(0.f)
        , g(0.f)
        , b(0.f)
        , a(0.f)
    {
    }
    Color(float c)
        : r(c)
        , g(c)
        , b(c)
        , a(c)
    {
    }
    Color(float _r, float _g, float _b, float _a)
        : r(_r)
        , g(_g)
        , b(_b)
        , a(_a)
    {
    }

    bool operator==(const Color& _b) const { return r == _b.r && g == _b.g && b == _b.b && a == _b.a; }
    bool operator!=(const Color& _b) const { return !(*this == _b); }
};

struct Rect
{
    uint32_t x;
    uint32_t y;
    uint32_t sizeX;
    uint32_t sizeY;
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

struct Viewport
{
    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;

    Viewport()
        : minX(0.f)
        , maxX(0.f)
        , minY(0.f)
        , maxY(0.f)
        , minZ(0.f)
        , maxZ(1.f)
    {
    }

    Viewport(float width, float height)
        : minX(0.f)
        , maxX(width)
        , minY(0.f)
        , maxY(height)
        , minZ(0.f)
        , maxZ(1.f)
    {
    }

    Viewport(float _minX, float _maxX, float _minY, float _maxY, float _minZ, float _maxZ)
        : minX(_minX)
        , maxX(_maxX)
        , minY(_minY)
        , maxY(_maxY)
        , minZ(_minZ)
        , maxZ(_maxZ)
    {
    }

    bool operator==(const Viewport& b) const
    {
        return minX == b.minX && minY == b.minY && minZ == b.minZ && maxX == b.maxX && maxY == b.maxY && maxZ == b.maxZ;
    }
    bool operator!=(const Viewport& b) const { return !(*this == b); }

    [[nodiscard]] float Width() const { return maxX - minX; }
    [[nodiscard]] float Height() const { return maxY - minY; }
};

} // namespace RHI
