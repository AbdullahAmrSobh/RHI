#pragma once

#include "RHI/Backend/Vulkan/Vulkan.hpp"
#include "RHI/ShaderResourceGroup.hpp"

namespace Vulkan
{

class Context;

class ShaderResourceGroupAllocator final : public RHI::ShaderResourceGroupAllocator
{
public:
    ShaderResourceGroupAllocator(RHI::Context& context)
        : RHI::ShaderResourceGroupAllocator(context)
    {
    }

    ~ShaderResourceGroupAllocator();

    RHI::ResultCode Init() override;

    std::unique_ptr<RHI::ShaderResourceGroup> Allocate(const RHI::ShaderResourceGroupLayout& layout) override;

private:
    Context*                        m_context;
    std::vector<vk::DescriptorPool> m_pools;
};

class ShaderResourceGroup final
    : public RHI::ShaderResourceGroup
    , public DeviceObject<vk::DescriptorSet>
{
public:
    ShaderResourceGroup(RHI::Context& context, vk::DescriptorPool pool, vk::DescriptorSet handle);
    ~ShaderResourceGroup();

    void Update();

private:
    Context*           m_context;
    vk::DescriptorPool m_pool;
};

}  // namespace Vulkan