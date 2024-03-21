    #pragma once

    #include <cstdint>
    #include <cstddef>
    #include <deque>

    #include "RHI/Common/Span.hpp"

    namespace RHI::TL
    {
        template<typename Key, typename Value>
        class LRUCache
        {
        public:
            LRUCache(size_t capacity) = default;

            void            SetCapacity(size_t newCapacity);
            void            Put(Key key, Value value);
            Value*          Get(const Key& key);

            TL::Span<Value> Evict();
        };
    } // namespace RHI::TL