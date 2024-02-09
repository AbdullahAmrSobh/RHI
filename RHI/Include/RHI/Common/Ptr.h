#pragma once

#include <memory>

namespace RHI
{
    template<typename T>
    using Ptr = std::unique_ptr<T>;

    template<class T, class... Args>
    inline constexpr Ptr<T> CreatePtr(Args... args)
    {
        return std::make_unique<T, Args...>(std::forward<Args>(args)...);
    }
} // namespace RHI