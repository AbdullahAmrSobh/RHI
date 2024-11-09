#pragma once

#include <RHI/Sampler.hpp>
#include <RHI/Result.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;

    VkFilter ConvertFilter(SamplerFilter samplerFilter);

    VkSamplerAddressMode ConvertSamplerAddressMode(SamplerAddressMode addressMode);

    VkCompareOp ConvertCompareOp(SamplerCompareOperation compareOperation);

    struct ISampler : Sampler
    {
        VkSampler handle;

        ResultCode Init(IDevice* device, const SamplerCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };
} // namespace RHI::Vulkan
