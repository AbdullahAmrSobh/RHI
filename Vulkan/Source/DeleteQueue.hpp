#pragma once

#include <RHI/Common/Containers.h>
#include <RHI/Resources.hpp>

#include <vulkan/vulkan.h>

#include <functional>

namespace RHI::Vulkan
{
    class IContext;

    class DeleteQueue
    {
    public:
        inline static constexpr uint32_t MAX_FRAMES_IN_FLIGHT_COUNT = 3;

        DeleteQueue(IContext* context)
            : m_context(context)
        {
        }

        ~DeleteQueue();

        void Destroy(uint64_t frameIndex, std::function<void()> callback);

        void Flush(uint64_t frameIndex);

    private:
        IContext* m_context;

        TL::Vector<std::function<void()>> m_deleteQueue[MAX_FRAMES_IN_FLIGHT_COUNT]; // todo define count as constant
    };
} // namespace RHI::Vulkan