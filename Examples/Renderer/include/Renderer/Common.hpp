#pragma once

#include <RHI/RHI.hpp>

namespace Engine
{
    template<typename T>
    using Result     = RHI::Result<T>;
    using ResultCode = RHI::ResultCode;

    template<typename T>
    class Singleton
    {
    public:
        inline static T* ptr = TL::Construct<T>();

        Singleton() { Singleton::ptr = (T*)this; }

        ~Singleton()
        {
            TL::Destruct(this);
            Singleton::ptr = nullptr;
        }
    };
} // namespace Engine