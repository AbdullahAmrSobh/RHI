#pragma once

#include "RHI/Common/Assert.hpp"
#include "RHI/Common/Containers.h"

#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <vector>

namespace RHI::TL
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

        constexpr Span() noexcept
            : m_data(nullptr)
            , m_count(0)
        {
        }

        template<typename BaseElementType>
        constexpr Span(Span<BaseElementType> span)
            : m_data(span.data())
            , m_count(span.size())
        {
            using T1 = std::remove_pointer_t<BaseElementType>;
            using T2 = std::remove_pointer_t<ElementType>;
            static_assert(std::is_base_of_v<T1, T2>, "Invalid cast");
        }

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

        constexpr Span(std::vector<std::remove_cv_t<ElementType>>& elements) noexcept
            : m_data(elements.data())
            , m_count(elements.size())
        {
        }

        constexpr Span(TL::Vector<std::remove_cv_t<ElementType>>& elements) noexcept
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

    template<typename T>
    inline static Span<const uint8_t> ToBytes(const T& t)
    {
        return Span<const uint8_t>((uint8_t*)&t, sizeof(T));
    }

    template<typename T>
    inline static Span<const uint8_t> ToBytes(const Span<T>& t)
    {
        return Span<const uint8_t>((uint8_t*)t.data(), t.size_bytes());
    }
} // namespace RHI::TL