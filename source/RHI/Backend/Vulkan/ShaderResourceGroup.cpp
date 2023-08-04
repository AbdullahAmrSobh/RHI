#pragma once

#include "RHI/Backend/Vulkan/ShaderResourceGroup.hpp"

#include "RHI/Backend/Vulkan/Context.hpp"
#include "RHI/Backend/Vulkan/Resources.hpp"
#include "RHI/Backend/Vulkan/Conversion.inl"

namespace Vulkan
{

ShaderResourceGroupAllocator::~ShaderResourceGroupAllocator()
{
}

RHI::ResultCode ShaderResourceGroupAllocator::Init()
{
    return {};
}

std::unique_ptr<RHI::ShaderResourceGroup> ShaderResourceGroupAllocator::Allocate(const RHI::ShaderResourceGroupLayout& layout)
{
    Context&   context   = static_cast<Context&>(*m_context);
    vk::Device device    = context.GetDevice();
    auto       setLayout = context.CreateDescriptorSetLayout(layout);

    vk::DescriptorSetAllocateInfo allocateInfo {};
    allocateInfo.setPSetLayouts(&setLayout->get());

    vk::DescriptorSet set = VK_NULL_HANDLE;

    for (vk::DescriptorPool pool : m_pools)
    {
        allocateInfo.setDescriptorPool(pool);
        vk::Result result = device.allocateDescriptorSets(&allocateInfo, &set);

        if (result == vk::Result::eSuccess)
        {
            return std::make_unique<ShaderResourceGroup>(context, pool, set);
        }
    }

    std::vector<vk::DescriptorPoolSize> poolSize {};

    for (auto& binding : layout.bindings)
    {
        vk::DescriptorPoolSize bindingSize {};
        bindingSize.setDescriptorCount(binding.arrayCount);
        bindingSize.setType(GetDescriptorType(binding.type));
        poolSize.push_back(bindingSize);
    }

    vk::DescriptorPoolCreateInfo createInfo {};
    createInfo.setMaxSets(10);
    createInfo.setPoolSizes(poolSize);

    vk::DescriptorPool pool = device.createDescriptorPool(createInfo).value;
    m_pools.push_back(pool);

    allocateInfo.setDescriptorPool(pool);
    vk::DescriptorSet descriptorSet = device.allocateDescriptorSets(allocateInfo).value.front();

    return std::make_unique<ShaderResourceGroup>(context, pool, descriptorSet);
}

ShaderResourceGroup::ShaderResourceGroup(RHI::Context& context, vk::DescriptorPool pool, vk::DescriptorSet handle)
    : DeviceObject(handle)
    , m_context(static_cast<Vulkan::Context*>(&context))
    , m_pool(pool)
{
}

ShaderResourceGroup::~ShaderResourceGroup()
{
    Context&   context = static_cast<Context&>(*m_context);
    vk::Device device  = context.GetDevice();

    device.freeDescriptorSets(m_pool, {m_handle});
}

void ShaderResourceGroup::Update()
{
    Context&   context = static_cast<Context&>(*m_context);
    vk::Device device  = context.GetDevice();

    std::vector<vk::WriteDescriptorSet> writeInfos {};

    for (auto binding : m_bindingMaps)
    {
        vk::WriteDescriptorSet writeInfo {};
        writeInfo.setDstBinding(binding.first);
        writeInfo.setDstSet(m_handle);
        writeInfo.setDescriptorCount(binding.second.arrayCount);
        writeInfo.setDescriptorType(GetDescriptorType(binding.second.type));

        writeInfos.push_back(writeInfo);
    }

    device.updateDescriptorSets(writeInfos, {});
}

}  // namespace Vulkan