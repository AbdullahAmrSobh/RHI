#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

#include "RHI/Common/Allocator.hpp"

#include <tracy/Tracy.hpp>

namespace RHI::TL
{
    template<typename T>
    using Vector = std::vector<T, DefaultAllocator<T>>;

    template<typename Key, typename Value>
    using UnorderedMap = std::unordered_map<Key, Value, DefaultAllocator<std::pair<Key, Value>>>;

    template<typename Key, typename Value>
    using Map = std::map<Key, Value, DefaultAllocator<std::pair<Key, Value>>>;

    template<typename T>
    using UnorderedSet = std::unordered_set<T, DefaultAllocator<T>>;

    template<typename T>
    using Set = std::set<T, DefaultAllocator<T>>;
} // namespace RHI::TL