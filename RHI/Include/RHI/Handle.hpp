#pragma once
#include <algorithm>
#include <cstdint>

namespace RHI
{

enum class ObjectType : uint16_t
{
    None,
    // TODO
};

template<typename Resource>
class Handle
{
public:
    inline bool operator==(Handle lhs) const
    {
        return lhs.m_generetionId == rhs.m_generetionId && lhs.m_index == rhs.m_index && lhs.m_resourceType == rhs.m_resourceType;
    }

    inline bool operator!=(Handle lhs) const
    {
        return !(lhs.m_generetionId == rhs.m_generetionId && lhs.m_index == rhs.m_index && lhs.m_resourceType == rhs.m_resourceType);
    }

private:
    using ResourceDescriptor = typename Resource::Descriptor;
    friend class HandlePool<Resource, ResourceDescriptor>;

    Handle(uint32_t index, uint32_t generationId, ObjectType type);

    uint32_t         m_index;
    uint16_t         m_generetionId;
    const ObjectType m_resourceType;
};

static_assert(sizeof(Handle) == sizeof(uint64_t));
static_assert(alignof(Handle) == alignof(uint32_t));

inline static constexpr Handle NullHandle = Handle {UINT32_MAX, UINT16_MAX, ObjectType::None};

/// @brief Represents a storage where RHI objects live.
template<typename Resource>
class HandlePool
{
public:
    using HandleType = Handle<Resource>;
    using Descriptor = typename Resource::Descriptor;

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

    // Gets the resource associated with handle.
    RHI_FORCE_INLINE Resource GetResource(Handle<Resource> handle) const
    {
        uint32_t index        = handle.m_index;
        uint16_t generationID = handle.m_generetionId;
        if (index < m_resources.size() && generationID == m_generetionID[index])
        {
            return m_resources[index];
        }
        return NullHandle;
    }

    // Gets the descriptor of the resource associated with the handle.
    RHI_FORCE_INLINE Descriptor GetDescriptor(Handle<Resource> handle) const
    {
        uint32_t index        = handle.m_index;
        uint16_t generationID = handle.m_generetionId;
        if (index < m_resources.size() && generationID == m_generetionID[index])
        {
            return m_descriptors[index];
        }
        return NullHandle;
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

        return Handle(index, m_generetionID[index], ObjectType::Unknown);
    }

    // Gets all objects that are currently alive.
    RHI_FORCE_INLINE std::vector<Handle> GetLiveHandles() const
    {
        std::vector<Handle<Resource>> liveHandles;
        for (uint32_t i = 0; i < m_resources.size(); i++)
        {
            if (m_resources[i] != NullHandle)
            {
                liveHandles.push_back(Handle(i, m_generetionID[i], ObjectType::Unknown));
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
            m_resources[index] = NullHandle;
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