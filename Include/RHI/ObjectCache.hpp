#pragma once
#include "RHI/Common.hpp"

namespace RHI
{

template<typename ObjectType>
class ObjectCache
{
public:
    ObjectCache() = default;

    std::shared_ptr<ObjectType> Find(size_t key) const
    {
        auto it = m_cache.find(key);
        if (it == m_cache.end())
        {
            return nullptr;
        }
        return it->second;
    }

    void Insert(size_t key, std::shared_ptr<ObjectType> object)
    {
        m_cache.try_emplace(key, object);
    }

    void Clear()
    {
        m_cache.clear();
    }

    template<typename... Args>
    using CreateFunction = std::function<std::shared_ptr<ObjectType>(Args... args)>;

private:
    std::unordered_map<size_t, std::shared_ptr<ObjectType>> m_cache;
};
}  // namespace RHI