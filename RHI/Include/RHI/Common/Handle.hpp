#pragma once

#include <cstdint>
#include <functional>

namespace RHI
{
    template<typename Resource>
    class Handle final
    {
        template<typename U>
        friend class HandlePool;
        template<typename U>
        friend class Handle;

    public:
        Handle()
        {
            m_handle = UINT64_MAX;
        }

        template<typename BaseResource>
        Handle(Handle<BaseResource> baseHandle)
        {
            // static_assert(!std::is_same_v<BaseResource, Resource> && (std::is_base_of_v<BaseResource, Resource> || std::is_base_of_v<Resource, BaseResource>), "Invalid cast");
            m_rawHandle.index = baseHandle.m_rawHandle.index;
            m_rawHandle.genId = baseHandle.m_rawHandle.genId;
        }

        inline operator bool() const
        {
            return m_handle != UINT64_MAX;
        }

        inline bool operator==(Handle other) const
        {
            return m_handle == other.m_handle;
        }

        inline bool operator!=(Handle other) const
        {
            return m_handle != other.m_handle;
        }

        explicit operator uint64_t() const
        {
            return m_handle;
        }

    private:
        Handle(uint64_t id, uint16_t genId)
        {
            m_rawHandle.index = id;
            m_rawHandle.genId = genId;
        }

        struct RawHandle
        {
            uint64_t index : 48;
            uint64_t genId : 16;
        };

        union
        {
            RawHandle m_rawHandle;
            uint64_t  m_handle;
        };
    };

    /// @brief Represents a storage where RHI objects live.
    template<typename Resource>
    class HandlePool final
    {
    public:
        using HandleType   = Handle<Resource>;
        using ResourceType = Resource;
        // using ResourceInfo = Info;

        inline HandlePool(uint32_t capacity = 512);
        ~HandlePool();

        inline void                                   Reset();

        // Gets the resource associated with handle.
        inline Resource*                              Get(Handle<Resource> handle) const;

        // Inserts a new resource and returns its handle.
        inline Handle<Resource>                       Insert(Resource resource);

        // Inserted a zerod resource and returns its handle.
        inline std::pair<Handle<Resource>, Resource&> InsertZerod();

        // Removes a resource from the owner.
        inline void                                   Remove(Handle<Resource> handle);

    private:
        inline void Resize(uint32_t newSize);

    private:
        size_t    m_capacity;

        size_t    m_count;

        Resource* m_resources;

        uint16_t* m_genIds;

        size_t    m_freeSlotsCount;

        uint32_t* m_freeSlots;
    };

    template<typename Resource>
    HandlePool<Resource>::HandlePool(uint32_t capacity)
        : m_capacity(capacity)
        , m_count(0)
        , m_freeSlotsCount(0)
    {
        m_capacity  = capacity;
        m_resources = (Resource*)malloc(capacity * sizeof(Resource));
        m_genIds    = (uint16_t*)malloc(capacity * sizeof(uint16_t));
        m_freeSlots = (uint32_t*)malloc(capacity * sizeof(uint32_t));
        memset(m_resources, 0, sizeof(m_capacity));
        memset(m_genIds, 0, sizeof(m_capacity));
        memset(m_freeSlots, 0, sizeof(m_capacity));
    }

    template<typename Resource>
    inline HandlePool<Resource>::~HandlePool()
    {
        free(m_resources);
        free(m_genIds);
        free(m_freeSlots);
    }

    template<typename Resource>
    inline void HandlePool<Resource>::Reset()
    {
        m_count          = 0;
        m_freeSlotsCount = 0;
        memset(m_resources, 0, sizeof(m_capacity));
        memset(m_genIds, 0, sizeof(m_capacity));
        memset(m_freeSlots, 0, sizeof(m_capacity));
    }

    template<typename Resource>
    inline Resource* HandlePool<Resource>::Get(Handle<Resource> handle) const
    {
        auto index = handle.m_rawHandle.index;
        auto id    = handle.m_rawHandle.genId;

        if (index < m_count && id == m_genIds[index])
        {
            return m_resources + index;
        }

        return nullptr;
    }

    template<typename Resource>
    inline Handle<Resource> HandlePool<Resource>::Insert(Resource resource)
    {
        auto index = m_count;
        if (m_freeSlotsCount)
        {
            index = m_freeSlots[m_freeSlotsCount--];
        }
        else if (m_count == m_capacity)
        {
            Resize(m_count * 1.5);
        }

        m_count++;
        m_resources[index] = resource;
        return { index, ++m_genIds[index] };
    }

    template<typename Resource>
    inline std::pair<Handle<Resource>, Resource&> HandlePool<Resource>::InsertZerod()
    {
        auto index = m_count;
        if (m_freeSlotsCount)
        {
            index = m_freeSlots[m_freeSlotsCount--];
        }
        else if (m_count == m_capacity)
        {
            Resize(m_count * 1.5);
        }

        m_count++;
        return { Handle<Resource>(index, ++m_genIds[index]), m_resources[index] };
    }

    template<typename Resource>
    inline void HandlePool<Resource>::Remove(Handle<Resource> handle)
    {
        m_freeSlots[m_freeSlotsCount++] = handle.m_rawHandle.index;
    }

    template<typename Resource>
    inline void HandlePool<Resource>::Resize(uint32_t newSize)
    {
        m_capacity = newSize;
        realloc(m_resources, newSize * sizeof(Resource));
        realloc(m_genIds, newSize * sizeof(uint16_t));
        realloc(m_freeSlots, newSize * sizeof(uint32_t));
    }

} // namespace RHI

namespace std
{
    template<typename T>
    struct hash<RHI::Handle<T>>
    {
        size_t operator()(RHI::Handle<T> const& s) const noexcept
        {
            auto h = static_cast<uint64_t>(s);
            return hash<uint64_t>{}(h);
        }
    };

} // namespace std