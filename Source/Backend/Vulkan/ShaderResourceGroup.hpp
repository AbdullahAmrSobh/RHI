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
        static Result<Unique<DescriptorSetLayout>> Create(const Device& device, const ShaderResourceGroupLayout& layout);
        
        DescriptorSetLayout(const Device& device)
            : DeviceObject(&device)
        {
        }
        ~DescriptorSetLayout();
        
        VkResult Init(const ShaderResourceGroupLayout& layout);

        inline const std::vector<VkDescriptorPoolSize>& GetSize() const
        {
            return m_size;
        }

        inline const VkDescriptorPoolSize& GetBindingSize(uint32_t bindingIndex) const
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
        DescriptorSet(const Device& device, const DescriptorSetLayout& layout, VkDescriptorSet handle, const DescriptorPool& parant)
            : DeviceObject(&device, handle)
            , m_pLayout(&layout)
            , m_pParantPool(&parant)
        {
        }
        ~DescriptorSet();
        
        inline const DescriptorSetLayout& GetLayout() const
        {
            return *m_pLayout;
        }

        VkWriteDescriptorSet WriteImages(uint32_t bindingIndex, const std::vector<VkDescriptorImageInfo>& imageInfos) const;
        VkWriteDescriptorSet WriteBuffers(uint32_t bindingIndex, const std::vector<VkDescriptorBufferInfo>& bufferInfos) const;
        VkWriteDescriptorSet WriteTexelBuffers(uint32_t bindingIndex, const std::vector<VkBufferView>& bufferViews) const;
    
    private:
        const DescriptorPool*      m_pParantPool;
        const DescriptorSetLayout* m_pLayout;
    };

    class DescriptorPool final : public DeviceObject<VkDescriptorPool>
    {
    public:
        struct Capacity
        {
            uint32_t                          maxSets;
            std::vector<VkDescriptorPoolSize> sizes;
        };
        
        static Result<Unique<DescriptorPool>> Create(const Device& device, const Capacity& capacity);

        DescriptorPool(const Device& device)
            : DeviceObject(&device)
        {}
        
        ~DescriptorPool();
        
        inline Capacity GetCapacity() const
        {
            return m_capacity;
        }

        VkResult Init(const Capacity& capacity);

        Result<Unique<DescriptorSet>> Allocate(const DescriptorSetLayout& layout);

    private:
        Capacity                    m_capacity;
        std::vector<DescriptorSet*> m_pDescriptorSets;
    };

    class ShaderResourceGroup final : public IShaderResourceGroup
    {
    public:
        ShaderResourceGroup(const Device& device, Unique<DescriptorSet> descriptorSet)
            : m_pDevice(&device)
            , m_descriptorSet(std::move(descriptorSet))
        {
        }
        
        inline const DescriptorSet& GetDescriptorSet() const
        {
            return *m_descriptorSet;
        }
        
        virtual EResultCode Update(const ShaderResourceGroupData& data) override;
    
    private:
        const Device*               m_pDevice;
        Unique<DescriptorSet> m_descriptorSet;
    };
    
    class ShaderResourceGroupAllocator final : public IShaderResourceGroupAllocator
    {
    public:
        ShaderResourceGroupAllocator(const Device& device)
            : m_pDevice(&device)
        {
        }

        virtual Expected<Unique<IShaderResourceGroup>> Allocate(const ShaderResourceGroupLayout& layout) override;

    private:
        const Device*                       m_pDevice;
        std::vector<Unique<DescriptorPool>> m_descriptorPools;
    };

} // namespace Vulkan
} // namespace RHI