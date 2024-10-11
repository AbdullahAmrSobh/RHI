#pragma once

#include <RHI/Semaphore.hpp>
#include <RHI/Result.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;

    struct ISemaphore : Semaphore
    {
        VkSemaphore handle;
        SemaphoreCreateInfo info;
        uint64_t timelineValue; // TODO: Might want to make this be atomic

        ResultCode Init(IDevice* device, const SemaphoreCreateInfo& createInfo);
        void Shutdown(IDevice* device);
    };
} // namespace RHI::Vulkan