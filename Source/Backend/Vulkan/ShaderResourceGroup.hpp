#pragma once
#include "RHI/ShaderResourceGroup.hpp"

#include "Backend/Vulkan/Descriptor.hpp"

namespace RHI
{
namespace Vulkan
{

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

    ResultCode Update(const ShaderResourceGroupData& data) override;

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

     Expected<Unique<IShaderResourceGroup>> Allocate(const ShaderResourceGroupLayout& layout) override;

private:
    Device*                             m_device;
    std::vector<Unique<DescriptorPool>> m_descriptorPools;
};

}  // namespace Vulkan
}  // namespace RHI