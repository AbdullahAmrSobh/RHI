#pragma once

#include "RHI/Common/Handle.hpp"
#include "RHI/Common/Callstack.hpp"

namespace RHI
{

    template<typename HandleType>
    class LeakDetector
    {
    public:
        struct Resource
        {
            Handle<HandleType> handle;
            Callstack          callstack;
        };

        // Adds a new resource to be tracked
        void Register(Handle<HandleType> handle);

        // Frees the given resources
        void Unregister(Handle<HandleType> handle);

        uint32_t Report(Context* context , bool printStacktrace);

    private:
        std::vector<Resource> m_liveObjects;

    };

}; // namespace RHI