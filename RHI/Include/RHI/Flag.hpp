#pragma once

#include <type_traits>

namespace RHI
{

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

template<typename Enum>
Flags<Enum> operator|(Enum lhs, Enum rhs)
{
    using underlying_type = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<underlying_type>(lhs) | static_cast<underlying_type>(rhs));
}

template<typename Enum>
Flags<Enum> operator&(Enum lhs, Enum rhs)
{
    using underlying_type = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<underlying_type>(lhs) & static_cast<underlying_type>(rhs));
}

template<typename Enum>
Flags<Enum> operator^(Enum lhs, Enum rhs)
{
    using underlying_type = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<underlying_type>(lhs) ^ static_cast<underlying_type>(rhs));
}

template<typename Enum>
Flags<Enum>& operator|=(Enum& lhs, Enum rhs)
{
    return lhs | rhs;
}

template<typename Enum>
Flags<Enum>& operator&=(Enum& lhs, Enum rhs)
{
    return lhs & rhs;
}

template<typename Enum>
Flags<Enum>& operator^=(Enum& lhs, Enum rhs)
{
    return lhs & rhs;
}

}  // namespace RHI
