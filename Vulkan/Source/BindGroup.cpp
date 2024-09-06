#include "Common.hpp"

#include "Context.hpp"

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

    BindGroupAllocator::BindGroupAllocator(IContext* context)
        : m_context(context)
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
        Validate(vkCreateDescriptorPool(m_context->m_device, &createInfo, nullptr, &m_descriptorPool));
        return ResultCode::Success;
    }

    void BindGroupAllocator::Shutdown()
    {
        vkDestroyDescriptorPool(m_context->m_device, m_descriptorPool, nullptr);
    }

    ResultCode BindGroupAllocator::InitBindGroup(IBindGroup* bindGroup, IBindGroupLayout* bindGroupLayout, uint32_t bindlessElementsCount)
    {
        VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescriptorInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorSetCount = 1,
            .pDescriptorCounts = &bindlessElementsCount,
        };
        VkDescriptorSetAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = bindlessElementsCount != UINT32_MAX ? &variableDescriptorInfo : nullptr,
            .descriptorPool = m_descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &bindGroupLayout->handle,
        };

        auto result = vkAllocateDescriptorSets(m_context->m_device, &allocateInfo, &bindGroup->descriptorSet);
        if (result != VK_SUCCESS)
            return ResultCode::ErrorOutOfMemory;

        return ResultCode::Success;
    }

    void BindGroupAllocator::ShutdownBindGroup(IBindGroup* bindGroup)
    {
        vkFreeDescriptorSets(m_context->m_device, m_descriptorPool, 1, &bindGroup->descriptorSet);
    }

    ResultCode IBindGroupLayout::Init(IContext* context, const BindGroupLayoutCreateInfo& createInfo)
    {
        layoutInfo = createInfo;

        TL::Vector<VkDescriptorBindingFlags> bindingFlags;
        TL::Vector<VkDescriptorSetLayoutBinding> bindingInfos;

        for (uint32_t index = 0; index < c_MaxBindGroupElementsCount; index++)
        {
            auto binding = createInfo.bindings[index];
            bool useBindless = binding.arrayCount == ShaderBinding::VariableArraySize;

            if (binding.type == BindingType::None)
                break;

            VkDescriptorBindingFlags flags{};
            if (useBindless)
            {
                flags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
                flags |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
            }
            bindingFlags.push_back(flags);

            VkDescriptorSetLayoutBinding descriptorBinding{};
            descriptorBinding.binding = index;
            descriptorBinding.descriptorType = ConvertDescriptorType(binding.type);
            // TODO: revist this count
            descriptorBinding.descriptorCount = useBindless ? 1024 : binding.arrayCount;
            descriptorBinding.stageFlags = ConvertShaderStage(binding.stages);
            descriptorBinding.pImmutableSamplers = nullptr;
            bindingInfos.push_back(descriptorBinding);
        }

        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
        bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        bindingFlagsInfo.pNext = nullptr;
        bindingFlagsInfo.bindingCount = (uint32_t)bindingFlags.size();
        bindingFlagsInfo.pBindingFlags = bindingFlags.data();
        VkDescriptorSetLayoutCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        vkCreateInfo.pNext = &bindingFlagsInfo;
        vkCreateInfo.flags = 0;
        vkCreateInfo.bindingCount = (uint32_t)bindingInfos.size();
        vkCreateInfo.pBindings = bindingInfos.data();
        auto result = vkCreateDescriptorSetLayout(context->m_device, &vkCreateInfo, nullptr, &handle);
        return ConvertResult(result);
    }

    void IBindGroupLayout::Shutdown(IContext* context)
    {
        vkDestroyDescriptorSetLayout(context->m_device, handle, nullptr);
    }

    ResultCode IBindGroup::Init(IContext* context, Handle<BindGroupLayout> layoutHandle, uint32_t bindlessElementsCount)
    {
        layout = layoutHandle;

        auto allocator = context->m_bindGroupAllocator.get();
        auto layoutObject = context->m_bindGroupLayoutsOwner.Get(layoutHandle);

        return allocator->InitBindGroup(this, layoutObject, bindlessElementsCount);
    }

    void IBindGroup::Shutdown(IContext* context)
    {
        auto allocator = context->m_bindGroupAllocator.get();
        allocator->ShutdownBindGroup(this);
    }

    void IBindGroup::Write(IContext* context, TL::Span<const BindGroupUpdateInfo> bindingResources)
    {
        struct DescriptorWriteData
        {
            TL::Vector<VkDescriptorImageInfo> imageInfos;
            TL::Vector<VkDescriptorBufferInfo> bufferInfos;
            TL::Vector<VkBufferView> bufferViews;
        };

        TL::Vector<DescriptorWriteData> writeData;
        TL::Vector<VkWriteDescriptorSet> writeInfos;

        auto info = context->m_bindGroupLayoutsOwner.Get(layout)->layoutInfo;

        for (size_t i = 0; i < bindingResources.size(); ++i)
        {
            const BindGroupUpdateInfo& binding = bindingResources[i];

            DescriptorWriteData& data = writeData.emplace_back();

            VkWriteDescriptorSet writeInfo{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = descriptorSet,
                .dstBinding = binding.binding,
                .dstArrayElement = binding.dstArrayElement,
                .descriptorCount = 1,
                .descriptorType = ConvertDescriptorType(info.bindings[binding.binding].type),
                .pImageInfo = nullptr,
                .pBufferInfo = nullptr,
                .pTexelBufferView = nullptr,
            };

            switch (binding.type)
            {
            case BindGroupUpdateInfo::Type::Image:
                for (size_t j = 0; j < binding.data.images.size(); ++j)
                {
                    auto& imageInfo = data.imageInfos.emplace_back();

                    imageInfo.imageView = context->m_imageViewOwner.Get(binding.data.images[j])->handle;
                    switch (info.bindings[binding.binding].type)
                    {
                    case BindingType::SampledImage: imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; break;
                    case BindingType::StorageImage: imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; break;
                    default:                        TL_UNREACHABLE(); break;
                    }
                }
                writeInfo.pImageInfo = data.imageInfos.data();
                writeInfo.descriptorCount = uint32_t(binding.data.images.size());
                break;

            case BindGroupUpdateInfo::Type::Buffer:
                for (size_t j = 0; j < binding.data.buffers.size(); ++j)
                {
                    auto& bufferInfo = data.bufferInfos.emplace_back();
                    bufferInfo.buffer = context->m_bufferOwner.Get(binding.data.buffers[j])->handle;
                    bufferInfo.offset = 0; // Assuming full buffer range
                    bufferInfo.range = VK_WHOLE_SIZE;
                }
                writeInfo.pBufferInfo = data.bufferInfos.data();
                writeInfo.descriptorCount = uint32_t(binding.data.buffers.size());
                break;

            case BindGroupUpdateInfo::Type::DynamicBuffer:
                for (size_t j = 0; j < binding.data.dynamicBuffers.size(); ++j)
                {
                    auto& bufferInfo = data.bufferInfos.emplace_back();
                    bufferInfo.buffer = context->m_bufferOwner.Get(binding.data.dynamicBuffers[j].buffer)->handle;
                    bufferInfo.offset = binding.data.dynamicBuffers[j].offset;
                    bufferInfo.range = binding.data.dynamicBuffers[j].range;
                }
                writeInfo.pBufferInfo = data.bufferInfos.data();
                writeInfo.descriptorCount = uint32_t(binding.data.buffers.size());
                break;

            case BindGroupUpdateInfo::Type::Sampler:
                for (size_t j = 0; j < binding.data.samplers.size(); ++j)
                {
                    auto& imageInfo = data.imageInfos.emplace_back();
                    imageInfo.sampler = context->m_samplerOwner.Get(binding.data.samplers[j])->handle;
                }
                writeInfo.pImageInfo = data.imageInfos.data();
                writeInfo.descriptorCount = uint32_t(binding.data.samplers.size());
                break;

            default:
                TL_UNREACHABLE();
                break;
            }

            writeInfos.push_back(writeInfo);
        }

        vkUpdateDescriptorSets(context->m_device, uint32_t(writeInfos.size()), writeInfos.data(), 0, nullptr);
    }

} // namespace RHI::Vulkan