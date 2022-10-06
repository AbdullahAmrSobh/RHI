#pragma once
#include "RHI/ShaderResourceGroup.hpp"

#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{
    class DescriptorPool;

    class DescriptorSetLayout final : public DeviceObject<VkDescriptorSetLayout>
    {
    public:
        DescriptorSetLayout(Device& device);
        ~DescriptorSetLayout();

        VkResult Init(const ShaderResourceGroupLayout& layout);

        static Result<Unique<DescriptorSetLayout>> Create(Device& device, const ShaderResourceGroupLayout& layout);
    };

    class DescriptorSet final : public DeviceObject<VkDescriptorSet>
    {
    public:
        struct ImageBindingData
        {
            uint32_t                           dstBinding;
            uint32_t                           dstArrayElement;
            VkDescriptorType                   descriptorType;
            std::vector<VkDescriptorImageInfo> imageInfos;
        };

        DescriptorSet(Device& device, VkDescriptorSet handle); // : m_pDevice(&device), m_handle(handle) {}
        ~DescriptorSet();

        VkResult Init(const ShaderResourceGroupLayout& layout);

    private:
        DescriptorPool*             m_pParantLoop;
        Unique<DescriptorSetLayout> m_layout;
        VkWriteDescriptorSet        m_writeInfo;
    };

    class DescriptorPool final : public DeviceObject<VkDescriptorPool>
    {
    public:
        struct Capacity
        {
            uint32_t                                maxSets;
            const std::vector<VkDescriptorPoolSize> sizes;
        };

        static Result<Unique<DescriptorPool>> Create(Device& device, const Capacity& capacity);

        DescriptorPool(Device& device);
        ~DescriptorPool();

        inline Capacity GetCapacity() const
        {
            return m_capacity;
        }

        VkResult Init(const Capacity& capacity);

        Result<Unique<DescriptorSet>> Allocate(const ShaderResourceGroupLayout& layout);

    private:
        Capacity                    m_capacity;
        std::vector<DescriptorSet*> m_pDescriptorSets;
    };

    class DescrpitorSetLayoutCache
    {
    public:
        DescriptorSetLayout* FindLayout(size_t hash);

        void Add(size_t hash, const DescriptorSetLayout& layout);
        void Remove(size_t hash);

    private:
        std::map<size_t, const DescriptorSetLayout*> m_cache;
    };

    class ShaderResourceGroup final : public IShaderResourceGroup
    {
    public:
        ShaderResourceGroup(const Device& device);
        ~ShaderResourceGroup();

        virtual EResultCode Update(const ShaderResourceGroupData& data) override;

    private:
        Unique<DescriptorSet> m_descriptorSet;
    };

    class ShaderResourceGroupAllocator final : public IShaderResourceGroupAllocator
    {
    public:
        ShaderResourceGroupAllocator(const Device& device);
        ~ShaderResourceGroupAllocator();

        virtual Expected<Unique<IShaderResourceGroup>> Allocate(const ShaderResourceGroupLayout& layout) const override;

    private:
        Device*                             m_pDevice;
        std::vector<Unique<DescriptorPool>> m_descriptorPools;
    };

} // namespace Vulkan
} // namespace RHI