#pragma once
#include "Backend/Vulkan/Resource.hpp"
#include "RHI/ShaderResourceGroup.hpp"

namespace RHI
{
namespace Vulkan
{
class DescriptorPool;

class DescriptorSetLayout final : public DeviceObject<VkDescriptorSetLayout>
{
public:
    DescriptorSetLayout(const Device& device)
        : DeviceObject(&device)
    {
    }
    ~DescriptorSetLayout();

    VkResult Init(const ShaderResourceGroupLayout& layout);

    inline std::span<const VkDescriptorPoolSize> GetSize() const
    {
        return {m_size.begin(), m_size.end()};
    }

    inline const VkDescriptorPoolSize& GetBindingSize(
        uint32_t bindingIndex) const
    {
        return m_size.at(bindingIndex);
    }

    inline VkDescriptorSetLayoutBinding GetBinding(uint32_t bindingIndex) const
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
    DescriptorSet(const Device& device)
        : DeviceObject(&device)
    {
    }
    ~DescriptorSet();

    VkResult Init(const DescriptorPool&      pool,
                  const DescriptorSetLayout& layout);

    VkWriteDescriptorSet WriteImages(
        uint32_t                                  bindingIndex,
        const std::vector<VkDescriptorImageInfo>& imageInfos) const;
    VkWriteDescriptorSet WriteBuffers(
        uint32_t                                   bindingIndex,
        const std::vector<VkDescriptorBufferInfo>& bufferInfos) const;
    VkWriteDescriptorSet WriteTexelBuffers(
        uint32_t                         bindingIndex,
        const std::vector<VkBufferView>& bufferViews) const;

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

    DescriptorPool(const Device& device)
        : DeviceObject(&device)
    {
    }

    ~DescriptorPool();

    inline Capacity GetCapacity() const
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
    ShaderResourceGroup(const Device&         device,
                        Unique<DescriptorSet> descriptorSet)
        : m_pDevice(&device)
        , m_descriptorSet(std::move(descriptorSet))
    {
    }
    ~ShaderResourceGroup() = default;

    inline const DescriptorSet& GetDescriptorSet() const
    {
        return *m_descriptorSet;
    }

    virtual EResultCode Update(const ShaderResourceGroupData& data) override;

private:
    const Device*         m_pDevice;
    Unique<DescriptorSet> m_descriptorSet;
};

class ShaderResourceGroupAllocator final : public IShaderResourceGroupAllocator
{
public:
    ShaderResourceGroupAllocator(const Device& device)
        : m_pDevice(&device)
    {
    }

    virtual Expected<Unique<IShaderResourceGroup>> Allocate(
        const ShaderResourceGroupLayout& layout) override;

private:
    const Device*                       m_pDevice;
    std::vector<Unique<DescriptorPool>> m_descriptorPools;
};

}  // namespace Vulkan
}  // namespace RHI