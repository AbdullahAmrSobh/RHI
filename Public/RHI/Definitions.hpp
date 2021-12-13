#pragma once
#include "RHI/Core/Expected.hpp"
#include "RHI/Common.hpp"
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

template<class T> 
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
	
	virtual void Info(std::string_view message) = 0;
	virtual void Warn(std::string_view message) = 0;
	virtual void Error(std::string_view message) = 0;
	virtual void Fatel(std::string_view message) = 0;
};

} // namespace RHI
