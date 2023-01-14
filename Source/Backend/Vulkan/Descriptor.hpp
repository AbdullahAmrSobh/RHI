#pragma once
#include "Backend/Vulkan/DeviceObject.hpp"

namespace RHI
{

class ShaderResourceGroupLayout;

namespace Vulkan
{

class DescriptorPool;

class DescriptorSetLayout final : public DeviceObject<VkDescriptorSetLayout>
{
public:
    DescriptorSetLayout(Device& device)
        : DeviceObject(device)
    {
    }
    ~DescriptorSetLayout();

    VkResult Init(const ShaderResourceGroupLayout& layout);

    std::span<const VkDescriptorPoolSize> GetSize() const
    {
        return {m_size.begin(), m_size.end()};
    }

    const VkDescriptorPoolSize& GetBindingSize(uint32_t bindingIndex) const
    {
        return m_size.at(bindingIndex);
    }

    VkDescriptorSetLayoutBinding GetBinding(uint32_t bindingIndex) const
    {
        return m_bindings.at(bindingIndex);
    }

private:
    std::vector<VkDescriptorSetLayoutBinding> m_bindings;
    std::vector<VkDescriptorPoolSize>         m_size;
};

class DescriptorSet final : public DeviceObject<VkDescriptorSet>
{
public:
    DescriptorSet(Device& device)
        : DeviceObject(device)
    {
    }
    ~DescriptorSet();

    VkResult Init(const DescriptorPool& pool, const DescriptorSetLayout& layout);

    VkWriteDescriptorSet WriteImages(uint32_t bindingIndex, const std::vector<VkDescriptorImageInfo>& imageInfos) const;
    VkWriteDescriptorSet WriteBuffers(uint32_t bindingIndex, const std::vector<VkDescriptorBufferInfo>& bufferInfos) const;
    VkWriteDescriptorSet WriteTexelBuffers(uint32_t bindingIndex, const std::vector<VkBufferView>& bufferViews) const;

private:
    const DescriptorPool*      m_pool;
    const DescriptorSetLayout* m_layout;
};

class DescriptorPool final : public DeviceObject<VkDescriptorPool>
{
    friend class DescriptorPool;

public:
    struct Capacity
    {
        uint32_t                          maxSets;
        std::vector<VkDescriptorPoolSize> sizes;
    };

    DescriptorPool(Device& device)
        : DeviceObject(device)
    {
    }

    ~DescriptorPool();

    Capacity GetCapacity() const
    {
        return m_capacity;
    }

    VkResult Init(const Capacity& capacity);

private:
    Capacity                    m_capacity;
    std::vector<DescriptorSet*> m_pDescriptorSets;
};

}  // namespace Vulkan
}  // namespace RHI