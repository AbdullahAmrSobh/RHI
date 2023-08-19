#pragma once
#include <cstdint>

namespace RHI
{

template<typename T>
class LRUCache
{
public:
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using key_type = size_t;
    using size_type = size_t;

    LRUCache(size_type capacity);


};

}  // namespace RHI