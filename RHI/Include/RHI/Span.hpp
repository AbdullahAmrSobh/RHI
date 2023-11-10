#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <type_traits>
#include <vector>

#include "RHI/Assert.hpp"

namespace RHI
{
    namespace TL
    {

        template<typename ElementType>
        class Span
        {
            static_assert(std::is_object_v<ElementType>, "ElementType must be an object type");
            static_assert(!std::is_abstract_v<ElementType>, "ElementType cannot be an abstract class type");

        public:
            using SizeType  = std::size_t;
            using Pointer   = ElementType*;
            using Reference = ElementType&;
            using Iterator  = Pointer;

            constexpr Span(ElementType& type) noexcept
                : m_data(&type)
                , m_count(1)
            {
            }

            constexpr Span(Pointer element, SizeType count) noexcept
                : m_data(element)
                , m_count(count)
            {
            }

            constexpr Span(Pointer firstElement, Pointer lastElement) noexcept
                : m_data(firstElement)
                , m_count(lastElement - firstElement)
            {
                RHI_ASSERT(lastElement >= firstElement);
            }

            constexpr Span(const std::initializer_list<ElementType>& elements) noexcept
                requires std::is_const_v<ElementType>
                : m_data(elements.begin())
                , m_count(elements.size())
            {
            }

            constexpr Span(std::vector<ElementType>& elements) noexcept
                : m_data(elements.data())
                , m_count(elements.size())
            {
            }

            constexpr Span(const std::vector<ElementType>& elements) noexcept
                : m_data(elements.data())
                , m_count(elements.size())
            {
            }

            template<size_t N>
            constexpr Span(ElementType (&array)[N])
                : m_data(array)
                , m_count(sizeof(array) / sizeof(array[0]))
            {
            }

            RHI_NODISCARD constexpr size_t size() const noexcept
            {
                return m_count;
            }

            RHI_NODISCARD constexpr SizeType size_bytes() const noexcept
            {
                return m_count * sizeof(ElementType);
            }

            RHI_NODISCARD constexpr bool empty() const noexcept
            {
                return m_count == 0;
            }

            RHI_NODISCARD constexpr Reference operator[](const SizeType index) const noexcept
            {
                return m_data[index];
            }

            RHI_NODISCARD constexpr Reference front() const noexcept
            {
                return m_data[0];
            }

            RHI_NODISCARD constexpr Reference back() const noexcept
            {
                return m_data[m_count - 1];
            }

            RHI_NODISCARD constexpr Pointer data() const noexcept
            {
                return m_data;
            }

            RHI_NODISCARD constexpr Iterator begin() const noexcept
            {
                return m_data;
            }

            RHI_NODISCARD constexpr Iterator end() const noexcept
            {
                return m_data + m_count;
            }

        private:
            Pointer  m_data;
            SizeType m_count;
        };

    } // namespace TL
} // namespace RHI