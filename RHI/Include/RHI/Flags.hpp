#pragma once

#include <type_traits>

namespace RHI
{

    template<typename FlagBitsType>
    struct FlagTraits
    {
        enum
        {
            AllFlags = 0
        };
    };

    template<typename BitType>
    class Flags
    {
    public:
        using MaskType = typename std::underlying_type<BitType>::type;

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

        constexpr bool operator!() const noexcept
        {
            return !m_mask;
        }

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
            return Flags<BitType>(m_mask ^ FlagTraits<BitType>::AllFlags);
        }

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
    Flags<T> operator|(T lhs, T rhs)
    {
        using underlaying_type = typename std::underlying_type<T>::type;
        auto lhs_num           = static_cast<underlaying_type>(lhs);
        auto rhs_num           = static_cast<underlaying_type>(rhs);

        return Flags<T>(lhs_num | rhs_num);
    }

    template<typename T>
    Flags<T> operator&(T lhs, T rhs)
    {
        using underlaying_type = typename std::underlying_type<T>::type;
        auto lhs_num           = static_cast<underlaying_type>(lhs);
        auto rhs_num           = static_cast<underlaying_type>(rhs);

        return Flags<T>(lhs_num & rhs_num);
    }

    template<typename T>
    Flags<T> operator^(T lhs, T rhs)
    {
        using underlaying_type = typename std::underlying_type<T>::type;
        auto lhs_num           = static_cast<underlaying_type>(lhs);
        auto rhs_num           = static_cast<underlaying_type>(rhs);

        return Flags<T>(lhs_num ^ rhs_num);
    }

} // namespace RHI