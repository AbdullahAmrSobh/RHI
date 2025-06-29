#pragma once
#include <RHI/RHI.hpp>

#include "OffsetAllocator/offsetAllocator.hpp"

namespace Engine
{
    template<typename T>
    using Result     = RHI::Result<T>;
    using ResultCode = RHI::ResultCode;

    using Suballocator  = OffsetAllocator::Allocator;
    using Suballocation = OffsetAllocator::Allocation;

    template<typename T>
    class Singleton
    {
    public:
        inline static T* ptr = nullptr;

        Singleton() { Singleton::ptr = (T*)this; }

        ~Singleton() { Singleton::ptr = nullptr; }
    };
} // namespace Engine