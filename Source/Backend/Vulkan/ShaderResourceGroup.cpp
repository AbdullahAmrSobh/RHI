#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/ShaderResourceGroup.hpp"

namespace RHI
{
namespace Vulkan
{
    VkDescriptorType ConvertDescriptorType(EShaderInputResourceType resourceType, EAccess access)
    {
        if (access == EAccess::Read)
        {
            switch (resourceType)
            {
            case EShaderInputResourceType::Image: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            case EShaderInputResourceType::TexelBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
            case EShaderInputResourceType::Buffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case EShaderInputResourceType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
            default: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
            };
        }
        else
        {
            switch (resourceType)
            {
            case EShaderInputResourceType::Image: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            case EShaderInputResourceType::TexelBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
            case EShaderInputResourceType::Buffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            default: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
            };
        }

        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

    Result<Unique<DescriptorSetLayout>> DescriptorSetLayout::Create(Device& device, const ShaderResourceGroupLayout& layout)
    {
        Unique<DescriptorSetLayout> descriptorSetLayout = CreateUnique<DescriptorSetLayout>(device);
        VkResult                    result              = descriptorSetLayout->Init(layout);
        return std::move(descriptorSetLayout);
    }

    Result<Unique<DescriptorPool>> DescriptorPool::Create(Device& device, const Capacity& capacity)
    {
        Unique<DescriptorPool> descriptorPool = CreateUnique<DescriptorPool>(device);
        VkResult               result         = descriptorPool->Init(capacity);
        return std::move(descriptorPool);
    }

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult DescriptorSetLayout::Init(const ShaderResourceGroupLayout& layout)
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        uint32_t bindingLocation = 0;
        for (auto& binding : layout.GetShaderInputResourceBindings())
        {
            VkDescriptorSetLayoutBinding bindingInfo = {};
            bindingInfo.binding                      = bindingLocation++;
            bindingInfo.descriptorCount              = binding.count;
            bindingInfo.descriptorType               = ConvertDescriptorType(binding.type, binding.access);
            bindingInfo.stageFlags                   = CovnertShaderStages(binding.stages);
            bindingInfo.pImmutableSamplers           = nullptr;
        }

        VkDescriptorSetLayoutCreateInfo createInfo;
        createInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.pNext        = nullptr;
        createInfo.flags        = 0;
        createInfo.bindingCount = CountElements(bindings);
        createInfo.pBindings    = bindings.data();

        return vkCreateDescriptorSetLayout(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    DescriptorSet::~DescriptorSet()
    {
        vkFreeDescriptorSets(m_pDevice->GetHandle(), m_pParantLoop->GetHandle(), 1, &m_handle);
    }

    VkResult DescriptorSet::Init(const ShaderResourceGroupLayout& layout)
    {
        // m_layout                           = Create;
        VkDescriptorSetLayout layoutHandle = m_layout->GetHandle();

        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.pNext              = nullptr;
        allocateInfo.descriptorPool     = m_pParantLoop->GetHandle();
        allocateInfo.descriptorSetCount = 1;
        allocateInfo.pSetLayouts        = &layoutHandle;

        return vkAllocateDescriptorSets(m_pDevice->GetHandle(), &allocateInfo, &m_handle);
    }

    DescriptorPool::~DescriptorPool()
    {
        vkDestroyDescriptorPool(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    Result<Unique<DescriptorSet>> DescriptorPool::Allocate(const ShaderResourceGroupLayout& layout)
    {
        Unique<DescriptorSet>       descriptorSet = CreateUnique<DescriptorSet>(*m_pDevice);
        VkResult                    result = descriptorSet->Init(layout);
        if (result != VK_SUCCESS)
        {
            return ResultError(result);
        }
        return std::move(descriptorSet);
    }

    ShaderResourceGroup::ShaderResourceGroup(const Device& device) {}

    ShaderResourceGroup::~ShaderResourceGroup() {}

    EResultCode ShaderResourceGroup::Update(const ShaderResourceGroupData& data)
    {
        return EResultCode::Fail;
    }

    ShaderResourceGroupAllocator::ShaderResourceGroupAllocator(const Device& device) {}

    ShaderResourceGroupAllocator::~ShaderResourceGroupAllocator() {}

    Expected<Unique<IShaderResourceGroup>> ShaderResourceGroupAllocator::Allocate(const ShaderResourceGroupLayout& layout) const
    {
        Unique<ShaderResourceGroup> group = CreateUnique<ShaderResourceGroup>(*m_pDevice);
        return std::move(group);
    }

} // namespace Vulkan
} // namespace RHI