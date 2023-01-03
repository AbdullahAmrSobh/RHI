#pragma once
#include "RHI/ShaderResourceGroup.hpp"

#include "Backend/Vulkan/DeviceObject.hpp"

namespace RHI
{
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

class ShaderResourceGroup final : public IShaderResourceGroup
{
public:
    ShaderResourceGroup(Device& device, Unique<DescriptorSet> descriptorSet)
        : m_device(&device)
        , m_descriptorSet(std::move(descriptorSet))
    {
    }
    ~ShaderResourceGroup() = default;

    const DescriptorSet& GetDescriptorSet() const
    {
        return *m_descriptorSet;
    }

    virtual ResultCode Update(const ShaderResourceGroupData& data) override;

private:
    Device*               m_device;
    Unique<DescriptorSet> m_descriptorSet;
};

class ShaderResourceGroupAllocator final : public IShaderResourceGroupAllocator
{
public:
    ShaderResourceGroupAllocator(Device& device)
        : m_device(&device)
    {
    }

    virtual Expected<Unique<IShaderResourceGroup>> Allocate(const ShaderResourceGroupLayout& layout) override;

private:
    Device*                             m_device;
    std::vector<Unique<DescriptorPool>> m_descriptorPools;
};

}  // namespace Vulkan
}  // namespace RHI