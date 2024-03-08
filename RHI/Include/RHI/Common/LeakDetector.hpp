#pragma once

#include "RHI/Common/Handle.hpp"
#include "RHI/Common/Callstack.hpp"

#include <algorithm>
#include <format>

namespace RHI
{
    class Context;

    template<typename HandleType>
    class ResourceLeakDetector
    {
    public:
        struct Resource
        {
            Handle<HandleType> handle;
            Callstack          callstack;
        };

        void        OnCreate(Handle<HandleType> handle);
        void        OnDestroy(Handle<HandleType> handle);

        std::string ReportLiveResources();

        uint32_t    LeakedResourcesCount();

    private:
        std::vector<Resource>::iterator FindResource(Handle<HandleType> handle);

    private:
        std::vector<Resource> m_liveResources;
    };

    template<typename T>
    inline void ResourceLeakDetector<T>::OnCreate(Handle<T> handle)
    {
        auto it = FindResource(handle);
        RHI_ASSERT(it == m_liveResources.end()); // Resource already exists in live pool

        Resource resource{};
        resource.callstack = CaptureCallstack(2);
        resource.handle    = handle;

        m_liveResources.push_back(resource);
    }

    template<typename T>
    inline void ResourceLeakDetector<T>::OnDestroy(Handle<T> handle)
    {
        auto it = FindResource(handle);
        RHI_ASSERT(it != m_liveResources.end()); // Resource not found or perhaps was deleted before

        m_liveResources.erase(it);
    }

    template<typename T>
    inline std::string ResourceLeakDetector<T>::ReportLiveResources()
    {
        auto breakline = "\n=============================================================================\n";
        auto message   = std::format("{}{} leak count {} \n", breakline, typeid(T).name(), m_liveResources.size());

        for (auto resource : m_liveResources)
        {
            auto stacktraceReport = ReportCallstack(resource.callstack);
            message.append(std::format("{}\n", stacktraceReport));
        }

        message.append(std::format("{}", breakline));
        return message;
    }

    template<typename T>
    std::vector<typename ResourceLeakDetector<T>::Resource>::iterator ResourceLeakDetector<T>::FindResource(Handle<T> handle)
    {
        return std::find_if(m_liveResources.begin(), m_liveResources.end(), [handle](auto resource)
                            {
                                return resource.handle == handle;
                            });
    }

    template<typename T>
    uint32_t ResourceLeakDetector<T>::LeakedResourcesCount()
    {
        return m_liveResources.size();
    }

}; // namespace RHI