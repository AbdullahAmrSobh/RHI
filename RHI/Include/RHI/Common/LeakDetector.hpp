#pragma once

#include "RHI/Common/Handle.hpp"
#include "RHI/Common/Callstack.hpp"
#include "RHI/Common/Containers.h"

#include <format>

namespace RHI
{
    class Context;

    template<typename HandleType>
    class ResourceLeakDetector
    {
    public:
        void        OnCreate(Handle<HandleType> handle);
        void        OnDestroy(Handle<HandleType> handle);

        std::string ReportLiveResources();

        uint32_t    LeakedResourcesCount();

    private:
        TL::UnorderedMap<Handle<HandleType>, Callstack> m_liveResources;
    };

    template<typename T>
    inline void ResourceLeakDetector<T>::OnCreate(Handle<T> handle)
    {
        RHI_ASSERT(m_liveResources.find(handle) == m_liveResources.end()); // Resource already exists in live pool
        m_liveResources[handle] = CaptureCallstack(2);
    }

    template<typename T>
    inline void ResourceLeakDetector<T>::OnDestroy(Handle<T> handle)
    {
        RHI_ASSERT(m_liveResources.find(handle) != m_liveResources.end()); // Resource not found or perhaps was deleted before
        m_liveResources.erase(handle);
    }

    template<typename T>
    inline std::string ResourceLeakDetector<T>::ReportLiveResources()
    {
        auto breakline = "\n=============================================================================\n";
        auto message   = std::format("{}{} leak count {} \n", breakline, typeid(T).name(), m_liveResources.size());

        for (auto [handle, stacktrace] : m_liveResources)
        {
            auto stacktraceReport = ReportCallstack(stacktrace);
            message.append(std::format("{}\n", stacktraceReport));
        }

        message.append(std::format("{}", breakline));
        return message;
    }

    template<typename T>
    uint32_t ResourceLeakDetector<T>::LeakedResourcesCount()
    {
        return m_liveResources.size();
    }

}; // namespace RHI