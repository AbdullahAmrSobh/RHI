#include "Common.hpp"

#include "Device.hpp"

#include "BindGroup.hpp"

namespace RHI::Vulkan
{
    VkDescriptorType ConvertDescriptorType(BindingType bindingType)
    {
        switch (bindingType)
        {
        case BindingType::Sampler:              return VK_DESCRIPTOR_TYPE_SAMPLER;
        case BindingType::SampledImage:         return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case BindingType::StorageImage:         return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case BindingType::UniformBuffer:        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case BindingType::StorageBuffer:        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case BindingType::DynamicUniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        case BindingType::DynamicStorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        case BindingType::BufferView:           return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case BindingType::StorageBufferView:    return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        default:                                TL_UNREACHABLE(); return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        }
    }

    BindGroupAllocator::BindGroupAllocator(IDevice* device)
        : m_device(device)
    {
    }

    ResultCode BindGroupAllocator::Init()
    {
        VkDescriptorPoolSize poolSizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER,                 1024},
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1024},
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1024},
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1024},
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1024},
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1024},
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1024},
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024},
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1024},
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1024},
        };

        VkDescriptorPoolCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = 8,
            .poolSizeCount = sizeof(poolSizes) / sizeof(VkDescriptorPoolSize),
            .pPoolSizes = poolSizes,
        };
        Validate(vkCreateDescriptorPool(m_device->m_device, &createInfo, nullptr, &m_descriptorPool));
        return ResultCode::Success;
    }

    void BindGroupAllocator::Shutdown()
    {
        // vkDestroyDescriptorPool(m_device->m_device, m_descriptorPool, nullptr);
        m_device->m_deleteQueue.DestroyObject(m_descriptorPool);
    }

    ResultCode BindGroupAllocator::InitBindGroup(IBindGroup* bindGroup, IBindGroupLayout* bindGroupLayout)
    {
        bindGroup->bindlessCount = bindGroupLayout->bindlessCount;

        VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescriptorInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorSetCount = 1,
            .pDescriptorCounts = &bindGroup->bindlessCount,
        };
        VkDescriptorSetAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = bindGroupLayout->bindlessCount ? &variableDescriptorInfo : nullptr,
            .descriptorPool = m_descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &bindGroupLayout->handle,
        };

        auto result = vkAllocateDescriptorSets(m_device->m_device, &allocateInfo, &bindGroup->descriptorSet);
        if (result != VK_SUCCESS)
            return ResultCode::ErrorOutOfMemory;

        return ResultCode::Success;
    }

    void BindGroupAllocator::ShutdownBindGroup(IBindGroup* bindGroup)
    {
        vkFreeDescriptorSets(m_device->m_device, m_descriptorPool, 1, &bindGroup->descriptorSet);
        // m_device->m_deleteQueue.DestroyObject(bindGroup);
    }

    ResultCode IBindGroupLayout::Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo)
    {
        uint32_t bindingFlagsCount = 0;
        VkDescriptorBindingFlags bindingFlags[32] = {};

        uint32_t bindingLayoutsCount = 0;
        VkDescriptorSetLayoutBinding bindingLayouts[32] = {};

        uint32_t bindingCount = 0;
        for (size_t i = 0; i < createInfo.bindings.size(); ++i)
        {
            const auto& binding = createInfo.bindings[i];
            shaderBindings[bindingCount] = binding; // Store ShaderBinding in IBindGroupLayout

            // Check if this binding is bindless (array count is large or other criteria)
            bool isBindless = binding.arrayCount == BindlessArraySize;

            // Assign bindless descriptor binding flags if necessary
            if (isBindless)
            {
                bindingFlags[bindingFlagsCount++] =
                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                    VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

                bindlessCount = 1024;
            }
            else
            {
                bindingFlags[bindingFlagsCount++] = 0;
            }

            // Fill out VkDescriptorSetLayoutBinding
            bindingLayouts[bindingLayoutsCount++] = {
                .binding = bindingCount++,
                .descriptorType = ConvertDescriptorType(binding.type),
                .descriptorCount = binding.arrayCount, // Bindless types will have large counts or variable counts
                .stageFlags = ConvertShaderStage(binding.stages),
                .pImmutableSamplers = nullptr,
            };
        }

        // If bindless, set up VkDescriptorSetLayoutBindingFlagsCreateInfo
        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = bindingFlagsCount,
            .pBindingFlags = bindingFlags,
        };

        VkDescriptorSetLayoutCreateInfo setLayoutCI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = bindlessCount ? &bindingFlagsInfo : nullptr,
            .flags = static_cast<VkDescriptorSetLayoutCreateFlags>(bindlessCount ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT : 0),
            .bindingCount = bindingLayoutsCount,
            .pBindings = bindingLayouts,
        };

        // Create the descriptor set layout
        auto result = vkCreateDescriptorSetLayout(device->m_device, &setLayoutCI, nullptr, &handle);
        if (result == VK_SUCCESS && createInfo.name)
        {
            device->SetDebugName(handle, createInfo.name);
        }

        return ConvertResult(result);
    }

    void IBindGroupLayout::Shutdown(IDevice* device)
    {
        device->m_deleteQueue.DestroyObject(handle);
    }

    ResultCode IBindGroup::Init(IDevice* device, Handle<BindGroupLayout> layoutHandle)
    {
        auto allocator = device->m_bindGroupAllocator.get();
        auto layoutObject = device->m_bindGroupLayoutsOwner.Get(layoutHandle);

        std::copy(std::begin(layoutObject->shaderBindings), std::end(layoutObject->shaderBindings), std::begin(shaderBindings));

        return allocator->InitBindGroup(this, layoutObject);
    }

    void IBindGroup::Shutdown(IDevice* device)
    {
        auto allocator = device->m_bindGroupAllocator.get();
        allocator->ShutdownBindGroup(this);
    }

    void IBindGroup::Write(IDevice* device, const BindGroupUpdateInfo& updateInfo)
    {
        uint32_t writeInfosCount = 0;

        uint32_t imageInfosCount = 0;
        VkDescriptorImageInfo imageInfos[32] = {};

        uint32_t bufferInfosCount = 0;
        VkDescriptorBufferInfo bufferInfos[32] = {};

        uint32_t samplerInfosCount = 0;
        VkDescriptorImageInfo samplerInfos[32] = {};

        VkWriteDescriptorSet writeInfos[32 * 3]{};

        for (const auto& imageUpdate : updateInfo.images)
        {
            const auto& binding = shaderBindings[imageUpdate.dstBinding];
            VkDescriptorType descriptorType = ConvertDescriptorType(binding.type);

            for (size_t i = 0; i < imageUpdate.images.size(); ++i)
            {
                imageInfos[imageInfosCount] = {
                    .sampler = VK_NULL_HANDLE,
                    .imageView = device->m_imageOwner.Get(imageUpdate.images[i])->viewHandle,
                    .imageLayout = (descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
                                       ? VK_IMAGE_LAYOUT_GENERAL
                                       : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                };

                writeInfos[writeInfosCount++] = {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = descriptorSet,
                    .dstBinding = imageUpdate.dstBinding,
                    .dstArrayElement = imageUpdate.dstArrayElement + static_cast<uint32_t>(i),
                    .descriptorCount = 1,
                    .descriptorType = descriptorType,
                    .pImageInfo = &imageInfos[imageInfosCount++],
                    .pBufferInfo = nullptr,
                    .pTexelBufferView = nullptr,
                };
            }
        }

        for (const auto& bufferUpdate : updateInfo.buffers)
        {
            const auto& binding = shaderBindings[bufferUpdate.dstBinding];
            VkDescriptorType descriptorType = ConvertDescriptorType(binding.type);

            for (size_t i = 0; i < bufferUpdate.buffers.size(); ++i)
            {
                if (binding.type == BindingType::DynamicUniformBuffer || binding.type == BindingType::DynamicStorageBuffer)
                {
                    // For dynamic bindings, subregions are expected
                    auto subregion = bufferUpdate.subregions[i];
                    bufferInfos[bufferInfosCount] = {
                        .buffer = device->m_bufferOwner.Get(bufferUpdate.buffers[i])->handle,
                        .offset = subregion.offset,
                        .range = subregion.size,
                    };
                }
                else
                {
                    // Non-dynamic bindings don't use subregions
                    bufferInfos[bufferInfosCount] = {
                        .buffer = device->m_bufferOwner.Get(bufferUpdate.buffers[i])->handle,
                        .offset = 0,            // Use 0 if no specific subregion offset is required
                        .range = VK_WHOLE_SIZE, // Use VK_WHOLE_SIZE if no subregions are specified
                    };
                }

                writeInfos[writeInfosCount++] = {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = descriptorSet,
                    .dstBinding = bufferUpdate.dstBinding,
                    .dstArrayElement = bufferUpdate.dstArrayElement + static_cast<uint32_t>(i),
                    .descriptorCount = 1,
                    .descriptorType = descriptorType,
                    .pImageInfo = nullptr,
                    .pBufferInfo = &bufferInfos[bufferInfosCount++],
                    .pTexelBufferView = nullptr,
                };
            }
        }

        for (const auto& samplerUpdate : updateInfo.samplers)
        {
            [[maybe_unused]] const auto& binding = shaderBindings[samplerUpdate.dstBinding];
            VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

            for (size_t i = 0; i < samplerUpdate.samplers.size(); ++i)
            {
                samplerInfos[samplerInfosCount] = {
                    .sampler = device->m_samplerOwner.Get(samplerUpdate.samplers[i])->handle,
                    .imageView = VK_NULL_HANDLE,
                    .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                };

                writeInfos[writeInfosCount++] = {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = descriptorSet,
                    .dstBinding = samplerUpdate.dstBinding,
                    .dstArrayElement = samplerUpdate.dstArrayElement + static_cast<uint32_t>(i),
                    .descriptorCount = 1,
                    .descriptorType = descriptorType,
                    .pImageInfo = &samplerInfos[samplerInfosCount++],
                    .pBufferInfo = nullptr,
                    .pTexelBufferView = nullptr,
                };
            }
        }

        vkUpdateDescriptorSets(device->m_device, writeInfosCount, writeInfos, 0, nullptr);
    }

} // namespace RHI::Vulkan