#pragma once
#include <algorithm>
#include <cstdint>

#include "RHI/Common.hpp"

namespace RHI
{

/// @brief A Handle for a resource stored in the HandlePool
template<typename Resource>
class Handle
{
public:
    Handle()
    {
        m_packedHandle = UINT64_MAX;
    }

    Handle(uint32_t id, uint16_t genId)
    {
        m_index        = id;
        m_generationId = genId;
    }

    RHI_FORCE_INLINE bool operator==(Handle other) const
    {
        return m_packedHandle == other.m_packedHandle;
    }

    RHI_FORCE_INLINE bool operator!=(Handle other) const
    {
        return m_packedHandle != other.m_packedHandle;
    }

    /// @brief Return true if this handle was initalized.
    /// NOTE: This does not mean that the handle is a valid handle.
    /// You must call HandlePool<Resource>::IsValid to check for this handle.
    RHI_FORCE_INLINE operator bool() const
    {
        return m_packedHandle != UINT32_MAX;
    }

private:
    union
    {
        struct
        {
            uint64_t m_index        : 48;
            uint64_t m_generationId : 16;
        };

        uint64_t m_packedHandle;
    };
};

template<typename Descriptor>
class HandlePoolBase
{
public:
    std::vector<Descriptor> m_descriptors;
};

/// @brief Represents a storage where RHI objects live.
template<typename Resource, typename Descriptor>
class HandlePool
{
public:
    HandlePool(const HandlePool& handles) = delete;

    RHI_FORCE_INLINE HandlePool(uint32_t capacity)
    {
        m_resources.resize(capacity);
        m_descriptors.resize(capacity);
        m_generetionID.resize(capacity, 0);
    }

    // Remove all stored handles, and resets the graph
    RHI_FORCE_INLINE void Reset()
    {
        m_resources.Clear();
        m_descriptors.Clear();
        m_generetionID.Clear();
        m_availableSlots.Clear();
    }

    RHI_FORCE_INLINE bool IsValid(Handle<Resource> handle) const
    {
        uint32_t index        = handle.m_index;
        uint16_t generationID = handle.m_generetionId;
        if (index < m_resources.size() && generationID == m_generetionID[index])
        {
            return true;
        }
        return false;
    }

    // Gets the resource associated with handle.
    RHI_FORCE_INLINE Resource GetResource(Handle<Resource> handle) const
    {
        uint32_t index        = handle.m_index;
        uint16_t generationID = handle.m_generetionId;

        if (!IsValid(Handle))
        {
            return {};
        }
        else
        {
            return m_resources[index];
        }
    }

    // Inserts a new resource and returns its handle.
    RHI_FORCE_INLINE Handle<Resource> Insert(Resource resource, Descriptor descriptor)
    {
        // Reuse a slot if available, otherwise allocate a new one
        uint32_t index;
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
            m_descriptors.push_back(descriptor);
        }
        else
        {
            m_resources[index]   = resource;
            m_descriptors[index] = descriptor;
        }

        return Handle(index, m_generetionID[index]);
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
        uint32_t index        = handle.m_index;
        uint16_t generationID = handle.m_generetionId;

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
    std::vector<Resource>   m_resources;
    std::vector<Descriptor> m_descriptors;
    std::vector<int32_t>    m_generetionID;

    // Maintain a list of available slots (indices) for reuse
    std::vector<uint32_t> m_availableSlots;
};

}  // namespace RHI