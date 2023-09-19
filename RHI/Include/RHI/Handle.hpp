#pragma once
#include <cstdint>
#include <vector>

#include "RHI/Common.hpp"

namespace RHI
{

template<typename Resource>
class Handle
{
public:
    Handle()
    {
        uint64_t val = UINT64_MAX;
        memcpy(this, &val, sizeof(Handle<Resource>));
    }

    Handle(uint64_t id, uint16_t genId)
    {
        m_index        = id;
        m_generationId = genId;
    }

    template<typename T>
    Handle(Handle<T> _handle)
    {
        static_assert(std::is_base_of<Resource, T>::value, "Invalid type provided");

        auto h = reinterpret_cast<Handle<Resource>>(_handle);

        m_index        = h.m_index;
        m_generationId = h.m_generationId;
    }

    RHI_FORCE_INLINE bool operator==(Handle other) const
    {
        return m_index == other.m_index && m_generationId == other.m_generationId;
    }

    RHI_FORCE_INLINE bool operator!=(Handle other) const
    {
        return !(m_index == other.m_index && m_generationId == other.m_generationId);
    }

    RHI_FORCE_INLINE operator bool() const
    {
        return std::bit_cast<uint64_t>(*this) != UINT32_MAX;
    }

private:
    uint64_t m_index        : 48;
    uint64_t m_generationId : 16;
};

/// @brief Represents a storage where RHI objects live.
template<typename Resource, typename Auxiliary>
class HandlePool
{
public:
    HandlePool(const HandlePool& handles) = delete;

    RHI_FORCE_INLINE HandlePool(uint32_t capacity)
    {
        m_resources.resize(capacity);
        m_auxilirayData.resize(capacity);
        m_generetionID.resize(capacity, 0);
    }

    // Remove all stored handles, and resets the graph
    RHI_FORCE_INLINE void Reset()
    {
        m_resources.clear();
        m_auxilirayData.clear();
        m_generetionID.clear();
        m_availableSlots.clear();
    }

    RHI_FORCE_INLINE bool IsValid(Handle<Resource> handle) const
    {
        uint32_t index        = handle.m_data.index;
        uint16_t generationID = handle.m_data.generetionId;

        if (index < m_resources.size() && generationID == m_generetionID[index])
            return true;

        return false;
    }

    // Gets the resource associated with handle.
    template<typename T>
    RHI_FORCE_INLINE Resource Get(Handle<T> handle) const
    {
        uint64_t index        = handle.m_data.index;
        uint16_t generationID = handle.m_data.generationId;

        static_assert(std::is_base_of<Resource, T>::value, "Invalid handle type");

        if (IsValid(handle))
            return m_resources[index];

        RHI_ASSERT(false);

        return {};
    }

    template<typename T>
    RHI_FORCE_INLINE Resource GetInfo(Handle<T> handle) const
    {
        uint64_t index        = handle.m_data.index;
        uint16_t generationID = handle.m_data.generationId;

        static_assert(std::is_base_of<Resource, T>::value, "Invalid handle type");

        if (IsValid(handle))
            return m_auxilirayData[index];

        RHI_ASSERT(false);

        return {};
    }

    // Inserts a new resource and returns its handle.
    RHI_FORCE_INLINE Handle<Resource> Insert(Resource resource, Auxiliary descriptor)
    {
        // Reuse a slot if available, otherwise allocate a new one
        uint64_t index;

        if (!m_availableSlots.empty())
        {
            index = m_availableSlots.back();
            m_availableSlots.pop_back();
        }
        else
        {
            index = m_resources.size();
            m_generetionID.push_back(1);
        }

        // Update the resources and descriptors vectors
        if (index >= m_resources.size())
        {
            m_resources.push_back(resource);
            m_auxilirayData.push_back(descriptor);
        }
        else
        {
            m_resources[index]     = resource;
            m_auxilirayData[index] = descriptor;
        }

        return Handle<Resource>(index, m_generetionID[index]);
    }

    // Gets all objects that are currently alive.
    RHI_FORCE_INLINE std::vector<Handle<Resource>> GetLiveHandles() const
    {
        std::vector<Handle<Resource>> liveHandles;
        for (uint32_t i = 0; i < m_resources.size(); i++)
        {
            if (m_resources[i])
            {
                liveHandles.push_back(Handle(i, m_generetionID[i]));
            }
        }
        return liveHandles;
    }

    // Removes a resource from the owner.
    RHI_FORCE_INLINE void Remove(Handle<Resource> handle)
    {
        uint32_t index        = handle.m_data.index;
        uint16_t generationID = handle.m_data.generetionId;

        if (index < m_resources.size() && generationID == m_generetionID[index])
        {
            m_resources[index] = Handle<Resource>();
            m_generetionID[index]++;
            m_availableSlots.push_back(index);  // Add the slot to available slots
        }
    }

private:
    // TODO rewrite it such that it does not depend on std::vector.
    // as this is a very in-effiecent implementation.
    std::vector<Resource>  m_resources;
    std::vector<Auxiliary> m_auxilirayData;
    std::vector<int32_t>   m_generetionID;

    // Maintain a list of available slots (indices) for reusSe
    std::vector<uint32_t> m_availableSlots;
};

}  // namespace RHI