#include "RHI/Backend/Vulkan/DescriptorSetLayout.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"

#include "RHI/Backend/Vulkan/Sampler.hpp"
#include "RHI/Backend/Vulkan/Utils.hpp"

namespace RHI
{
namespace Vulkan
{
    Expected<DescriptorSetLayoutPtr> Factory::CreateDescriptorSetLayout(const DescriptorSeLayoutDesc& desc)
    {
        auto     descriptorSetLayout = CreateUnique<DescriptorSetLayout>(*m_device);
        VkResult result              = descriptorSetLayout->Init(desc);
        if (result != VK_SUCCESS)
            return descriptorSetLayout;
        else
            return tl::unexpected(ToResultCode(result));
    }

    DescriptorSetLayout::~DescriptorSetLayout() { vkDestroyDescriptorSetLayout(m_pDevice->GetHandle(), m_handle, nullptr); }
    
    VkResult DescriptorSetLayout::Init(const DescriptorSeLayoutDesc& desc)
    {
        std::vector<VkSampler>                    samplersHandles = {};
        std::vector<VkDescriptorSetLayoutBinding> bindingInfos;
        bindingInfos.reserve(desc.m_descriptors.size());

        uint32_t bindingLocation = 0;
        for (auto& descriptor : desc.m_descriptors)
        {

            VkDescriptorSetLayoutBinding bindingInfo = {};
            bindingInfo.binding                      = bindingLocation++;
            bindingInfo.descriptorType               = Utils::GetDescriptorType(descriptor.type, descriptor.accessType);
            bindingInfo.descriptorCount              = descriptor.count;
            bindingInfo.stageFlags                   = Utils::ConvertShaderStage(descriptor.stages);

            if (descriptor.pImmutableSampler)
            {
                for (uint32_t i = 0; i < descriptor.count; ++i)
                {
                    m_samplers.push_back(static_cast<const Sampler*>(descriptor.pImmutableSampler));
                    samplersHandles.push_back(m_samplers.back()->GetHandle());
                }
                bindingInfo.pImmutableSamplers = samplersHandles.data();
            }
            else
            {
                bindingInfo.pImmutableSamplers = nullptr;
            }
            bindingInfos.push_back(bindingInfo);
        }

        VkDescriptorSetLayoutCreateInfo createInfo = {};
        createInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.pNext                           = nullptr;
        createInfo.flags                           = 0;
        createInfo.bindingCount                    = static_cast<uint32_t>(bindingInfos.size());
        createInfo.pBindings                       = bindingInfos.data();
        return vkCreateDescriptorSetLayout(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    };

} // namespace Vulkan
} // namespace RHI
