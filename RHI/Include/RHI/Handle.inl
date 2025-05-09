#pragma once

#include <format>

namespace RHI
{
    template<typename Resource>
    void HandlePool<Resource>::Clear()
    {
        m_resources.clear();
        m_genIds.clear();
        m_freeSlots.clear();
        m_liveResources.clear();
    }

    template<typename Resource>
    const Resource* HandlePool<Resource>::Get(Handle<Resource> handle) const
    {
        // Check if handle is valid
        TL_ASSERT(handle != NullHandle);

        // Extract index and generation ID from handle
        size_t   index = handle.m_rawHandle.index;
        uint16_t genId = handle.m_rawHandle.genId;

        // Check if index is within bounds and generation ID matches
        if (index < m_resources.size() && m_genIds[index] == genId)
        {
            return &m_resources[index];
        }

        TL_UNREACHABLE(); // Invalid handle

        return nullptr;
    }

    template<typename Resource>
    Resource* HandlePool<Resource>::Get(Handle<Resource> handle)
    {
        // Check if handle is valid
        TL_ASSERT(handle != NullHandle);

        // Extract index and generation ID from handle
        size_t   index = handle.m_rawHandle.index;
        uint16_t genId = handle.m_rawHandle.genId;

        // Check if index is within bounds and generation ID matches
        if (index < m_resources.size() && m_genIds[index] == genId)
        {
            return &m_resources[index];
        }

        TL_UNREACHABLE(); // Invalid handle

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
            m_resources[index] = std::move(resource);
            // Generate a new generation ID
            uint16_t& genId    = m_genIds[index];
            m_genIds[index]    = genId;

            return Handle<Resource>(index, genId);
        }

        // If no free slot, expand the pool
        m_resources.emplace_back(std::move(resource));
        m_genIds.emplace_back(uint16_t(1));
        auto genId              = m_genIds.back();
        auto handle             = Handle<Resource>(m_resources.size() - 1, genId);
        m_liveResources[handle] = TL::CaptureStacktrace(3);
        return handle;
    }

    template<typename Resource>
    void HandlePool<Resource>::Release(Handle<Resource> handle)
    {
        // Check if handle is valid
        TL_ASSERT(handle != NullHandle);

        // Extract index and generation ID from handle
        size_t   index = handle.m_rawHandle.index;
        uint16_t genId = handle.m_rawHandle.genId;

        // Always call the destructor
        if (index < m_resources.size())
        {
            m_resources[index].~Resource();
        }

        // Check if index is within bounds and generation ID matches
        if (index < m_resources.size() && m_genIds[index] == genId)
        {
            // Mark the slot as free
            m_freeSlots.push_back(index);
        }

        m_liveResources.erase(handle);
    }

    template<typename T>
    TL::String HandlePool<T>::ReportLiveResources() const
    {
        auto breakline = "\n=============================================================================\n";
        auto message   = std::format("{}{} leak count {} \n", breakline, typeid(T).name(), m_liveResources.size());

        for (auto [handle, stacktrace] : m_liveResources)
        {
            auto stacktraceReport = TL::ReportStacktrace(stacktrace);
            message.append(std::format("{}\n", stacktraceReport));
        }

        message.append(std::format("{}", breakline));
        return TL::String{message};
    }

    template<typename T>
    uint32_t HandlePool<T>::ReportLiveResourcesCount() const
    {
        return (uint32_t)m_liveResources.size();
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