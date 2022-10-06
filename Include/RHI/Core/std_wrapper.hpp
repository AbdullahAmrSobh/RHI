// This header wraps the STL types, so it is easier to replace them when needed.

#include <array>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <utility>
#include <type_traits>

#include "Span.hpp"

#define RHI_STL_NAMESPACE_NAME tl

namespace RHI 
{
namespace RHI_STL_NAMESPACE_NAME
{
    template <typename T, typename Deleter = std::default_delete<T>>
    using Unique = std::unique_ptr<T, Deleter>;

    template <typename T>
    using Rfc = std::shared_ptr<T>;

    template <typename T, size_t Size>
    using Array = std::array<T, Size>;

    template <typename T, typename Allocator>
    using Vector = std::vector<T, Allocator>;

    template <typename T>
    using Span = nonstd::span<T>;

    using String = std::string; 

    using StringView = std::string_view;

    template<typename Key, typename T>
    using Map = std::map<Key, T>;

    template <typename Key, typename T>
    using UnorderedMap = std::map<Key, T>;

} // namespace RHI_STL_NAMESPACE_NAME
}