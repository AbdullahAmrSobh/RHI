#include "RHI/Backend/Vulkan/DescriptorPool.hpp"

namespace RHI
{
namespace Vulkan
{

    DescriptorPool::~DescriptorPool() { vkDestroyDescriptorPool(m_pDevice->GetHandle(), m_handle, nullptr); }

    VkResult DescriptorPool::Init(const DescriptorPoolDesc& desc)
    {
        std::vector<VkDescriptorPoolSize> sizes;
        sizes.reserve(desc.descriptors.size());
        
        for (const auto& descriptor : desc.descriptors)
        {
            VkDescriptorPoolSize size = {};
            size.type                 = static_cast<VkDescriptorType>(static_cast<uint32_t>(descriptor.type));
            size.descriptorCount      = descriptor.count;
            sizes.push_back(size);
        }

        VkDescriptorPoolCreateInfo createInfo = {};
        createInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo.pNext                      = nullptr;
        createInfo.flags                      = 0;
        createInfo.maxSets                    = desc.maxSets;
        createInfo.poolSizeCount              = static_cast<uint32_t>(sizes.size());
        createInfo.pPoolSizes                 = sizes.data();

        return vkCreateDescriptorPool(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

} // namespace Vulkan
} // namespace RHI
