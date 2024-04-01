#pragma once

#include <memory>

#include "RHI/Common/Containers.h"

namespace RHI
{
    template<typename T>
    using Ptr = std::unique_ptr<T>;

    template<class T, class... Args>
    inline constexpr Ptr<T> CreatePtr(Args... args)
    {
        return std::make_unique<T, Args...>(std::forward<Args>(args)...);
    }

    template<typename T, typename U, typename... Args>
    inline constexpr T* EmplacePtr(TL::Vector<Ptr<U>>& container, Args... args)
    {
        return (T*)container.emplace_back(CreatePtr<T>(args...)).get();
    }
} // namespace RHI