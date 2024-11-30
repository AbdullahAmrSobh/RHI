#include "Sampler.hpp"

#include "Common.hpp"
#include "Device.hpp"

namespace RHI::Vulkan
{
    VkFilter ConvertFilter(SamplerFilter samplerFilter)
    {
        switch (samplerFilter)
        {
        case SamplerFilter::Point:  return VK_FILTER_NEAREST;
        case SamplerFilter::Linear: return VK_FILTER_LINEAR;
        default:                    TL_UNREACHABLE(); return VK_FILTER_MAX_ENUM;
        }
    }

    VkSamplerAddressMode ConvertSamplerAddressMode(SamplerAddressMode addressMode)
    {
        switch (addressMode)
        {
        case SamplerAddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerAddressMode::Clamp:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        default:                         TL_UNREACHABLE(); return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
        }
    }

    VkCompareOp ConvertCompareOp(SamplerCompareOperation compareOperation)
    {
        switch (compareOperation)
        {
        case SamplerCompareOperation::Never:     return VK_COMPARE_OP_NEVER;
        case SamplerCompareOperation::Equal:     return VK_COMPARE_OP_EQUAL;
        case SamplerCompareOperation::NotEqual:  return VK_COMPARE_OP_NOT_EQUAL;
        case SamplerCompareOperation::Always:    return VK_COMPARE_OP_ALWAYS;
        case SamplerCompareOperation::Less:      return VK_COMPARE_OP_LESS;
        case SamplerCompareOperation::LessEq:    return VK_COMPARE_OP_LESS_OR_EQUAL;
        case SamplerCompareOperation::Greater:   return VK_COMPARE_OP_GREATER;
        case SamplerCompareOperation::GreaterEq: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        default:                                 TL_UNREACHABLE(); return VK_COMPARE_OP_MAX_ENUM;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Sampler
    ///////////////////////////////////////////////////////////////////////////

    ResultCode ISampler::Init(IDevice* device, const SamplerCreateInfo& createInfo)
    {
        VkSamplerCreateInfo samplerCI{
            .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext                   = nullptr,
            .flags                   = 0,
            .magFilter               = ConvertFilter(createInfo.filterMag),
            .minFilter               = ConvertFilter(createInfo.filterMin),
            .mipmapMode              = createInfo.filterMip == SamplerFilter::Linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .addressModeU            = ConvertSamplerAddressMode(createInfo.addressU),
            .addressModeV            = ConvertSamplerAddressMode(createInfo.addressV),
            .addressModeW            = ConvertSamplerAddressMode(createInfo.addressW),
            .mipLodBias              = createInfo.mipLodBias,
            .anisotropyEnable        = VK_TRUE,
            .maxAnisotropy           = 1.0f,
            .compareEnable           = VK_TRUE,
            .compareOp               = ConvertCompareOp(createInfo.compare),
            .minLod                  = createInfo.minLod,
            .maxLod                  = createInfo.maxLod,
            .borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
            .unnormalizedCoordinates = VK_FALSE,
        };
        auto result = vkCreateSampler(device->m_device, &samplerCI, nullptr, &handle);
        if (result == VK_SUCCESS && createInfo.name)
        {
            device->SetDebugName(handle, createInfo.name);
        }
        return ConvertResult(result);
    }

    void ISampler::Shutdown(IDevice* device)
    {
        vkDestroySampler(device->m_device, handle, nullptr);
    }

} // namespace RHI::Vulkan