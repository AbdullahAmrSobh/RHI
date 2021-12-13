#pragma once
#include <cstdint>
#include <memory>
#include <type_traits>

#define RHI_NODISCARD [[nodiscard]]

namespace RHI
{

template <typename T>
using Unique = std::unique_ptr<T>;

template <typename T>
using Shared = std::shared_ptr<T>;

template <typename T, typename... Args>
inline Unique<T> CreateUnique(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
inline Shared<T> CreateShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}


} // namespace RHI
