#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <type_traits>
#include <vector>

#include "RHI/Common.hpp"

namespace RHI
{
namespace TL
{

template<typename ElementType>
class Span
{
    static_assert(std::is_object<ElementType>::value,
                  "A span's ElementType must be an object type (not a "
                  "reference type or void)");
    static_assert(!std::is_abstract<ElementType>::value,
                  "A span's ElementType cannot be an abstract class type");

public:
    // constants and types
    using element_type     = ElementType;
    using value_type       = typename std::remove_cv<ElementType>::type;
    using size_type        = std::size_t;
    using difference_type  = std::ptrdiff_t;
    using pointer          = element_type*;
    using const_pointer    = const element_type*;
    using reference        = element_type&;
    using const_reference  = const element_type&;
    using iterator         = pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;

    Span() = default;

    constexpr Span(pointer element, size_type count) noexcept
        : m_values(element)
        , m_count(count)
    {
    }

    constexpr Span(pointer firstElement, pointer lastElement) noexcept
        : m_values(firstElement)
        , m_count(lastElement - firstElement)
    {
        assert(lastElement > firstElement);
    }

    constexpr Span(std::initializer_list<ElementType> elements) noexcept
        : m_values(elements.begin())
        , m_count(elements.end() - elements.end())
    {
    }

    constexpr Span(std::vector<ElementType>& elements) noexcept
        : m_values(elements.data())
        , m_count(elements.size())
    {
    }

    RHI_NODISCARD constexpr size_t size() const noexcept
    {
        return m_count;
    }

    RHI_NODISCARD constexpr size_type size_bytes() const noexcept
    {
        return m_count * sizeof(element_type);
    }

    RHI_NODISCARD constexpr bool empty() const noexcept
    {
        return m_count == 0;
    }

    RHI_NODISCARD constexpr reference operator[](const size_type index) const noexcept
    {
        return m_values[index];
    }

    RHI_NODISCARD constexpr reference front() const noexcept
    {
        return m_values[0];
    }

    RHI_NODISCARD constexpr reference back() const noexcept
    {
        return m_values[m_count - 1];
    }

    RHI_NODISCARD constexpr pointer data() const noexcept
    {
        return m_values;
    }

    RHI_NODISCARD constexpr iterator begin() const noexcept
    {
        return m_values;
    }

    RHI_NODISCARD constexpr iterator end() const noexcept
    {
        return m_values + m_count;
    }

private:
    const ElementType* m_values;
    size_type    m_count;
};

}  // namespace TL
}  // namespace RHI