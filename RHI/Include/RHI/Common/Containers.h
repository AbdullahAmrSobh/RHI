#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <deque>

#include "RHI/Common/Allocator.hpp"

#include <tracy/Tracy.hpp>

namespace RHI::TL
{
    template<typename T>
    using Vector = std::vector<T, DefaultAllocator<T>>;

    template<typename Key, typename Value, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
    using UnorderedMap = std::unordered_map<Key, Value, Hasher, KeyEq, DefaultAllocator<std::pair<const Key, Value>>>;

    template<typename Key, typename Value>
    using Map = std::map<Key, Value, DefaultAllocator<std::pair<Key, Value>>>;

    template<typename T>
    using UnorderedSet = std::unordered_set<T, DefaultAllocator<T>>;

    template<typename T>
    using Set = std::set<T, DefaultAllocator<T>>;

    template<typename T>
    using Deque     = std::deque<T, DefaultAllocator<T>>;

    using String    = std::basic_string<char, std::char_traits<char>, DefaultAllocator<char>>;
    using WString   = std::basic_string<wchar_t, std::char_traits<wchar_t>, DefaultAllocator<wchar_t>>;
    using U8string  = std::basic_string<char8_t, std::char_traits<char8_t>, DefaultAllocator<char8_t>>;
    using U16string = std::basic_string<char16_t, std::char_traits<char16_t>, DefaultAllocator<char16_t>>;
    using U32string = std::basic_string<char32_t, std::char_traits<char32_t>, DefaultAllocator<char32_t>>;
} // namespace RHI::TL