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

namespace internal
{

inline constexpr std::size_t dynamic_extent = SIZE_MAX;

template<typename ElementType>
struct SpanExtent
{
public:

    SpanExtent() = default;
    
    constexpr SpanExtent(ElementType* elements, size_t count) noexcept
        : m_elements {elements}
        , m_size(count)
    {
    }

    constexpr SpanExtent(ElementType* begin, ElementType* end) noexcept
        : m_elements {begin}
        , m_size {end - begin}
    {
    }

private:
    size_t       m_size     = 0;
    ElementType* m_elements = nullptr;
};

}  // namespace internal

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
        : m_storage(element, count)
    {
    }

    constexpr Span(pointer firstElement, pointer lastElement) noexcept
        : m_storage(firstElement, lastElement - firstElement)
    {
        assert(lastElement > firstElement);
    }

    constexpr Span(std::initializer_list<ElementType> elements) noexcept
    {
    }

    constexpr Span(std::vector<ElementType>& elements) noexcept
        : m_storage(elements.data(), elements.size())
    {
    }

    RHI_NODISCARD constexpr size_t size() const noexcept
    {
        return m_storage.m_size;
    }

    RHI_NODISCARD constexpr size_type size_bytes() const noexcept
    {
        return m_storage.m_size * sizeof(element_type);
    }

    RHI_NODISCARD constexpr bool empty() const noexcept
    {
        return m_storage.m_size == 0;
    }

    RHI_NODISCARD constexpr reference operator[](const size_type index) const noexcept
    {
        return m_storage.m_elements[index];
    }

    RHI_NODISCARD constexpr reference front() const noexcept
    {
        return m_storage.m_elements[0];
    }

    RHI_NODISCARD constexpr reference back() const noexcept
    {
        return m_storage.m_elements[m_storage.m_size - 1];
    }

    RHI_NODISCARD constexpr pointer data() const noexcept
    {
        return m_storage.m_elements;
    }

    RHI_NODISCARD constexpr iterator begin() const noexcept
    {
        return m_storage.m_elements;
    }

    RHI_NODISCARD constexpr iterator end() const noexcept
    {
        return m_storage.m_elements + m_storage.m_size;
    }

private:
    using StorageType = internal::SpanExtent<ElementType>;

    StorageType m_storage;
};

}  // namespace TL
}  // namespace RHI