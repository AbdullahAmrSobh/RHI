#pragma once

#include <RHI/Sampler.hpp>
#include <RHI/Result.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;

    VkFilter ConvertFilter(SamplerFilter samplerFilter);
    VkSamplerAddressMode ConvertSamplerAddressMode(SamplerAddressMode addressMode);
    VkCompareOp ConvertCompareOp(SamplerCompareOperation compareOperation);


    struct ISampler : Sampler
    {
        VkSampler handle;

        ResultCode Init(IContext* context, const SamplerCreateInfo& createInfo);
        void Shutdown(IContext* context);
    };
} // namespace RHI::Vulkan
