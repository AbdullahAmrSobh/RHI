#include "RHI/Backend/Vulkan/Descriptor.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"

namespace RHI
{
namespace Vulkan
{
    DescriptorSet::~DescriptorSet() { vkFreeDescriptorSets(m_pDevice->GetHandle(), m_parantPool, 1, &m_handle); }

    VkResult DescriptorSet::Init(const DescriptorSetLayout& layout)
    {
        VkDescriptorSetAllocateInfo allocateInfo = {};
        return vkAllocateDescriptorSets(m_pDevice->GetHandle(), &allocateInfo, &m_handle);
    }

    void DescriptorSet::BeginUpdate() { m_isUpdateing = true; }

    void DescriptorSet::EndUpdate()
    {
        vkUpdateDescriptorSets(m_pDevice->GetHandle(), static_cast<uint32_t>(m_writeInfo.size()), m_writeInfo.data(), static_cast<uint32_t>(m_copyInfo.size()),
                               m_copyInfo.data());
    }

    void DescriptorSet::BindResource(uint32_t dstBinding, ITexture& texture) {}
    void DescriptorSet::BindResource(uint32_t dstBinding, ArrayView<ITexture*> textures) {}
    void DescriptorSet::BindResource(uint32_t dstBinding, IBuffer& buffer) {}
    void DescriptorSet::BindResource(uint32_t dstBinding, ArrayView<IBuffer*> buffers) {}

    Expected<DescriptorPoolPtr> Factory::CreateDescriptorPool(const DescriptorPoolDesc& desc)
    {
        auto     descriptorPool = CreateUnique<DescriptorPool>(*m_device);
        VkResult result         = descriptorPool->Init(desc);
        if (result != VK_SUCCESS)
            tl::unexpected(ToResultCode(result));

        return descriptorPool;
    }

    DescriptorPool::~DescriptorPool() { vkDestroyDescriptorPool(m_pDevice->GetHandle(), m_handle, nullptr); }

    VkResult DescriptorPool::Init(const DescriptorPoolDesc& desc)
    {
        VkDescriptorPoolCreateInfo createInfo = {};

        return vkCreateDescriptorPool(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    Expected<DescriptorSetPtr> DescriptorPool::AllocateDescriptorSet(const DescriptorSetLayout& layout)
    {
        auto     descriptorSet = CreateUnique<DescriptorSet>(*m_pDevice, m_handle);
        VkResult result        = descriptorSet->Init(layout);
        if (result != VK_SUCCESS)
            tl::unexpected(ToResultCode(result));

        return descriptorSet;
    }

} // namespace Vulkan
} // namespace RHI
