#pragma once

#include "RHI/Common/Assert.hpp"
#include "RHI/Common/Containers.h"

#include <cstdint>
#include <type_traits>

#define RHI_DECALRE_OPAQUE_RESOURCE(name) \
    struct name                           \
    {                                     \
    }

namespace RHI
{
    struct NullHandle_T final
    {
    };

    inline constexpr NullHandle_T NullHandle = {};

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

        Handle(NullHandle_T)
        {
            m_handle = UINT64_MAX;
        }

        template<typename BaseResource>
            requires(std::is_base_of_v<BaseResource, Resource> || std::is_base_of_v<Resource, BaseResource>)
        Handle(Handle<BaseResource> baseHandle)
        {
            m_rawHandle.index = baseHandle.m_rawHandle.index;
            m_rawHandle.genId = baseHandle.m_rawHandle.genId;
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

        inline bool operator==(const NullHandle_T&) const
        {
            return m_handle == UINT64_MAX;
        }

        inline operator bool() const
        {
            return *this != NullHandle;
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

        HandlePool()                   = default;
        HandlePool(const HandlePool&&) = delete;
        HandlePool(HandlePool&&)       = default;
        ~HandlePool()                  = default;

        // Clears all resources in the pool
        inline void Clear();

        // Gets the resource associated with handle
        inline const Resource* Get(Handle<Resource> handle) const;

        inline Resource* Get(Handle<Resource> handle);

        // Inserted a zerod resource and returns its handle
        inline Handle<Resource> Emplace(Resource&& resource);

        // Removes a resource from the owner
        inline void Release(Handle<Resource> handle);

    private:
        TL::Vector<Resource> m_resources;
        TL::Vector<uint16_t> m_genIds;
        TL::Vector<size_t>   m_freeSlots;
    };

} // namespace RHI

namespace RHI
{

    template<typename Resource>
    void HandlePool<Resource>::Clear()
    {
        m_resources.clear();
        m_genIds.clear();
        m_freeSlots.clear();
    }

    template<typename Resource>
    const Resource* HandlePool<Resource>::Get(Handle<Resource> handle) const
    {
        // Check if handle is valid
        RHI_ASSERT(handle != NullHandle);

        // Extract index and generation ID from handle
        size_t   index = handle.m_rawHandle.index;
        uint16_t genId = handle.m_rawHandle.genId;

        // Check if index is within bounds and generation ID matches
        if (index < m_resources.size() && m_genIds[index] == genId)
        {
            return &m_resources[index];
        }

        RHI_UNREACHABLE(); // Invalid handle

        return nullptr;
    }

    template<typename Resource>
    Resource* HandlePool<Resource>::Get(Handle<Resource> handle)
    {
        // Check if handle is valid
        RHI_ASSERT(handle != NullHandle);

        // Extract index and generation ID from handle
        size_t   index = handle.m_rawHandle.index;
        uint16_t genId = handle.m_rawHandle.genId;

        // Check if index is within bounds and generation ID matches
        if (index < m_resources.size() && m_genIds[index] == genId)
        {
            return &m_resources[index];
        }

        RHI_UNREACHABLE(); // Invalid handle

        return nullptr;
    }

    template<typename Resource>
    Handle<Resource> HandlePool<Resource>::Emplace(Resource&& resource)
    {
        // Check if there's a free slot
        if (!m_freeSlots.empty())
        {
            size_t index = m_freeSlots.back();
            m_freeSlots.pop_back();
            m_resources[index] = std::forward<Resource>(resource);
            // Generate a new generation ID
            uint16_t& genId = m_genIds[index];
            m_genIds[index] = genId;
            return Handle<Resource>(index, genId);
        }

        // If no free slot, expand the pool
        m_resources.emplace_back(std::forward<Resource>(resource));
        uint16_t genId = m_genIds.emplace_back((uint16_t)1);
        return Handle<Resource>(m_resources.size() - 1, genId);
    }

    template<typename Resource>
    void HandlePool<Resource>::Release(Handle<Resource> handle)
    {
        // Check if handle is valid
        RHI_ASSERT(handle != NullHandle);

        // Extract index and generation ID from handle
        size_t   index = handle.m_rawHandle.index;
        uint16_t genId = handle.m_rawHandle.genId;

        // Check if index is within bounds and generation ID matches
        if (index < m_resources.size() && m_genIds[index] == genId)
        {
            // Mark the slot as free
            m_freeSlots.push_back(index);
        }
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