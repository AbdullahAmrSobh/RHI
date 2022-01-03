#include "RHI/Backend/Vulkan/Sampler.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"

namespace RHI
{
namespace Vulkan
{

    Expected<SamplerPtr> Factory::CreateSampler(const SamplerDesc& desc)
    {
        auto sampler = CreateUnique<Sampler>(*m_device);

        VkResult result = sampler->Init(desc);
        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));
        
        return sampler;
    }

    Sampler::~Sampler() { vkDestroySampler(m_pDevice->GetHandle(), m_handle, nullptr); }

    VkResult Sampler::Init(const SamplerDesc& desc)
    {
        VkSamplerCreateInfo createInfo     = {};
        createInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        createInfo.pNext                   = nullptr;
        createInfo.flags                   = 0;
        createInfo.magFilter               = static_cast<VkFilter>(desc.filter);
        createInfo.minFilter               = static_cast<VkFilter>(desc.filter);
        createInfo.mipmapMode              = static_cast<VkSamplerMipmapMode>(desc.filter);
        createInfo.addressModeU            = static_cast<VkSamplerAddressMode>(desc.addressModeU);
        createInfo.addressModeV            = static_cast<VkSamplerAddressMode>(desc.addressModeV);
        createInfo.addressModeW            = static_cast<VkSamplerAddressMode>(desc.addressModeW);
        createInfo.mipLodBias              = desc.mipLodBias;
        createInfo.anisotropyEnable        = desc.maxAnisotropy > 0.0f ? VK_TRUE : VK_FALSE;
        createInfo.maxAnisotropy           = desc.maxAnisotropy;
        createInfo.compareEnable           = desc.compareOp != ECompareOp::Never ? VK_TRUE : VK_FALSE;
        createInfo.compareOp               = static_cast<VkCompareOp>(desc.compareOp);
        createInfo.minLod                  = desc.minLod;
        createInfo.maxLod                  = desc.maxLod;
        createInfo.borderColor             = static_cast<VkBorderColor>(desc.borderColor);
        createInfo.unnormalizedCoordinates = VK_FALSE;

        return vkCreateSampler(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

} // namespace Vulkan
} // namespace RHI
