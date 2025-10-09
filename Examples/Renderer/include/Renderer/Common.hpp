#pragma once

#include <RHI/RHI.hpp>

namespace Engine
{
    template<typename T>
    class Singleton
    {
    public:
        inline static T* ptr = TL::construct<T>();

        Singleton() { Singleton::ptr = (T*)this; }

        ~Singleton()
        {
            // TL::Destruct(ptr);
            Singleton::ptr = nullptr;
        }
    };
} // namespace Engine