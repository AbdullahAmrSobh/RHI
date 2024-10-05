#pragma once

#include <RHI/Semaphore.hpp>
#include <RHI/Result.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;

    struct ISemaphore : Semaphore
    {
        VkSemaphore handle;
        SemaphoreCreateInfo info;
        uint64_t timelineValue; // TODO: Might want to make this be atomic

        ResultCode Init(IContext* context, const SemaphoreCreateInfo& createInfo);
        void Shutdown(IContext* context);
    };
} // namespace RHI::Vulkan