#pragma once

#include <cstdint>
#include <cstddef>
#include <stdlib.h>

#include "RHI/Export.hpp"

#include <tracy/Tracy.hpp>

RHI_EXPORT namespace RHI::TL
{
    template<typename _Ty>
    struct DefaultAllocator
    {
        static_assert(!std::is_const_v<_Ty>, "The C++ Standard forbids containers of const elements "
                                             "because allocator<const T> is ill-formed.");
        static_assert(!std::is_function_v<_Ty>, "The C++ Standard forbids allocators for function elements "
                                                "because of [allocator.requirements].");
        static_assert(!std::is_reference_v<_Ty>, "The C++ Standard forbids allocators for reference elements "
                                                 "because of [allocator.requirements].");

        using _From_primary   = DefaultAllocator;

        using value_type      = _Ty;

        using size_type       = size_t;
        using difference_type = ptrdiff_t;

        constexpr DefaultAllocator() noexcept {}

        constexpr DefaultAllocator(const DefaultAllocator&) noexcept = default;

        template<class _Other>
        constexpr DefaultAllocator(const DefaultAllocator<_Other>&) noexcept
        {
        }

        constexpr ~DefaultAllocator()                                              = default;

        constexpr DefaultAllocator&             operator=(const DefaultAllocator&) = default;

        [[nodiscard]] constexpr RHI_EXPORT _Ty* allocate(_CRT_GUARDOVERFLOW const size_t count)
        {
            static_assert(sizeof(value_type) > 0, "value_type must be complete before calling allocate.");
            auto ptr = static_cast<_Ty*>(_aligned_malloc(sizeof(value_type) * count, alignof(value_type)));
            TracyAlloc(ptr, count * sizeof(value_type));
            return ptr;
        }

        constexpr void deallocate(_Ty* const ptr, const size_t count)
        {
            _STL_ASSERT(ptr != nullptr || count == 0, "null pointer cannot point to a block of non-zero size");
            TracyFree(ptr);
            _aligned_free(ptr);
        }
    };

} // namespace RHI::TL