#pragma once

#include <cstdint>

namespace RHI
{
    // All RHI allocators will be made through the provided allocator interface. 
    class Allocator
    {
    public:
        using pointer                                               = void*;
        using const_pointer                                               = const pointer;

        virtual ~Allocator()                                        = default;

        virtual pointer Allocate(size_t size, uint8_t alignment)    = 0;
        virtual pointer Free(pointer pointer)                       = 0;
        virtual pointer Reallocate(pointer pointer, size_t newSize) = 0;
    };
} // namespace RHI