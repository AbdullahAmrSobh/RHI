#include "RHI/Pch.hpp"

#include "Backend/Vulkan/Common.hpp"

#include "Backend/Vulkan/Buffer.hpp"
#include "Backend/Vulkan/Descriptor.hpp"
#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/Image.hpp"
#include "Backend/Vulkan/Resource.hpp"
#include "Backend/Vulkan/ShaderResourceGroup.hpp"
#include "Backend/Vulkan/PipelineState.hpp"

namespace RHI
{
namespace Vulkan
{

VkDescriptorType ConvertDescriptorType(ShaderInputResourceType resourceType, ShaderResourceAccessType access)
{
    if (access == ShaderResourceAccessType::Read)
    {
        switch (resourceType)
        {
            case ShaderInputResourceType::Image: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            case ShaderInputResourceType::TexelBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
            case ShaderInputResourceType::Buffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case ShaderInputResourceType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
            default: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        };
    }
    else
    {
        switch (resourceType)
        {
            case ShaderInputResourceType::Image: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            case ShaderInputResourceType::TexelBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
            case ShaderInputResourceType::Buffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            default: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        };
    }
}

// DescriptorSetLayout
DescriptorSetLayout::~DescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(m_device->GetHandle(), m_handle, nullptr);
}

VkResult DescriptorSetLayout::Init(const ShaderResourceGroupLayout& layout)
{
    uint32_t bindingLocation = 0;
    for (auto& binding : layout.GetShaderInputResourceBindings())
    {
        VkDescriptorSetLayoutBinding bindingInfo = {};
        bindingInfo.binding                      = bindingLocation++;
        bindingInfo.descriptorCount              = binding.count;
        bindingInfo.descriptorType               = ConvertDescriptorType(binding.type, binding.access);
        bindingInfo.stageFlags                   = CovnertShaderStages(binding.stages);
        bindingInfo.pImmutableSamplers           = nullptr;
        m_bindings.push_back(bindingInfo);

        VkDescriptorPoolSize poolSize;
        poolSize.type            = bindingInfo.descriptorType;
        poolSize.descriptorCount = bindingInfo.descriptorCount;
        m_size.push_back(poolSize);
    }

    VkDescriptorSetLayoutCreateInfo createInfo;
    createInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext        = nullptr;
    createInfo.flags        = 0;
    createInfo.bindingCount = CountElements(m_bindings);
    createInfo.pBindings    = m_bindings.data();

    return vkCreateDescriptorSetLayout(m_device->GetHandle(), &createInfo, nullptr, &m_handle);
}

// DescriptorSet

DescriptorSet::~DescriptorSet()
{
    vkFreeDescriptorSets(m_device->GetHandle(), m_pool->GetHandle(), 1, &m_handle);
}

VkResult DescriptorSet::Init(const DescriptorPool& pool, const DescriptorSetLayout& layout)
{
    m_pool   = &pool;
    m_layout = &layout;

    VkDescriptorSetLayout layoutHandle = layout.GetHandle();
    VkDescriptorSet       setHandle    = VK_NULL_HANDLE;

    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.pNext                       = nullptr;
    allocateInfo.descriptorSetCount          = 1;
    allocateInfo.pSetLayouts                 = &layoutHandle;
    allocateInfo.descriptorPool              = m_pool->GetHandle();

    return vkAllocateDescriptorSets(m_device->GetHandle(), &allocateInfo, &setHandle);
}

VkWriteDescriptorSet DescriptorSet::WriteImages(uint32_t bindingIndex, const std::vector<VkDescriptorImageInfo>& imageInfos) const
{
    VkWriteDescriptorSet write;
    write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext           = nullptr;
    write.dstSet          = m_handle;
    write.dstBinding      = bindingIndex;
    write.dstArrayElement = 0;
    write.descriptorCount = CountElements(imageInfos);
    write.descriptorType  = m_layout->GetBinding(bindingIndex).descriptorType;
    write.pImageInfo      = imageInfos.data();
    return write;
}

VkWriteDescriptorSet DescriptorSet::WriteBuffers(uint32_t bindingIndex, const std::vector<VkDescriptorBufferInfo>& bufferInfos) const
{
    VkWriteDescriptorSet write;
    write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext           = nullptr;
    write.dstSet          = m_handle;
    write.dstBinding      = bindingIndex;
    write.dstArrayElement = 0;
    write.descriptorCount = CountElements(bufferInfos);
    write.descriptorType  = m_layout->GetBinding(bindingIndex).descriptorType;
    write.pBufferInfo     = bufferInfos.data();
    return write;
}

VkWriteDescriptorSet DescriptorSet::WriteTexelBuffers(uint32_t bindingIndex, const std::vector<VkBufferView>& bufferViews) const
{
    VkWriteDescriptorSet write;
    write.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext            = nullptr;
    write.dstSet           = m_handle;
    write.dstBinding       = bindingIndex;
    write.dstArrayElement  = 0;
    write.descriptorCount  = CountElements(bufferViews);
    write.descriptorType   = m_layout->GetBinding(bindingIndex).descriptorType;
    write.pTexelBufferView = bufferViews.data();
    return write;
}

// DescriptorPool
DescriptorPool::~DescriptorPool()
{
    vkDestroyDescriptorPool(m_device->GetHandle(), m_handle, nullptr);
}

VkResult DescriptorPool::Init(const Capacity& capacity)
{
    VkDescriptorPoolCreateInfo createInfo = {};
    createInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext                      = nullptr;
    createInfo.flags                      = 0;
    createInfo.maxSets                    = capacity.maxSets;
    createInfo.poolSizeCount              = CountElements(capacity.sizes);
    createInfo.pPoolSizes                 = capacity.sizes.data();

    return vkCreateDescriptorPool(m_device->GetHandle(), &createInfo, nullptr, &m_handle);
}

}  // namespace Vulkan
}  // namespace RHI
