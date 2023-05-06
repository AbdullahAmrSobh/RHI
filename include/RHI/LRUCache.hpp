#pragma once
#include <list>
#include <memory>
#include <unordered_map>
#include <functional>

namespace RHI
{

inline size_t HashCombine(std::size_t seed, size_t val)
{
    std::hash<size_t> hasher;
    seed ^= hasher(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

template<typename T>
inline size_t HashAny(T t)
{
    const char*     data = reinterpret_cast<const char*>(&t);
    std::size_t     size = sizeof(T);
    std::size_t     hash = 0;
    std::hash<char> hasher;
    for (std::size_t i = 0; i < size; ++i)
    {
        hash ^= hasher(data[i]) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }
    return hash;
}

template<typename T>
class LRUCache
{
public:
    LRUCache(size_t capacity)
        : m_capacity(capacity)
    {
    }

    std::shared_ptr<T> Get(size_t key)
    {
        auto it = m_cache.find(key);
        if (it == m_cache.end())
        {
            return nullptr;
        }
        // Move the accessed key to the front of the list
        m_keyOrder.splice(m_keyOrder.begin(), m_keyOrder, it->second);
        return it->second->second;
    }

    void Insert(size_t key, std::shared_ptr<T> value)
    {
        auto it = m_cache.find(key);
        if (it != m_cache.end())
        {
            // Key already exists in cache, move it to the front of the list
            m_keyOrder.splice(m_keyOrder.begin(), m_keyOrder, it->second);
            it->second->second = value;
        }
        else
        {
            // Key not in cache, add it to the front of the list
            m_keyOrder.emplace_front(key, value);
            m_cache[key] = m_keyOrder.begin();
            if (m_cache.size() > m_capacity)
            {
                // Remove the least recently used element from the cache
                auto last = m_keyOrder.end();
                last--;
                m_cache.erase(last->first);
                m_keyOrder.pop_back();
            }
        }
    }

private:
    size_t                                                                                          m_capacity;
    std::list<std::pair<size_t, std::shared_ptr<T>>>                                                m_keyOrder;
    std::unordered_map<size_t, typename std::list<std::pair<size_t, std::shared_ptr<T>>>::iterator> m_cache;
};

}  // namespace RHI