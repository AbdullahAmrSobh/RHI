#include <RHI/Common/Assert.hpp>

#include "Common.hpp"
#include "Context.hpp"
#include "Resources.hpp"

namespace RHI::Vulkan
{
    ///////////////////////////////////////////////////////////////////////////
    /// BindGroupAllocator
    ///////////////////////////////////////////////////////////////////////////

    BindGroupAllocator::BindGroupAllocator(IContext* context)
        : m_context(context)
    {
    }

    ResultCode BindGroupAllocator::Init()
    {
        VkDescriptorPoolSize poolSizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1024 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1024 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1024 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1024 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1024 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1024 },
        };

        VkDescriptorPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        createInfo.maxSets = 8;
        createInfo.poolSizeCount = sizeof(poolSizes) / sizeof(VkDescriptorPoolSize);
        createInfo.pPoolSizes = poolSizes;

        Validate(vkCreateDescriptorPool(m_context->m_device, &createInfo, nullptr, &m_descriptorPool));

        return ResultCode::Success;
    }

    void BindGroupAllocator::Shutdown()
    {
        vkDestroyDescriptorPool(m_context->m_device, m_descriptorPool, nullptr);
    }

    ResultCode BindGroupAllocator::InitBindGroup(IBindGroup* bindGroup, IBindGroupLayout* bindGroupLayout, uint32_t bindlessElementsCount)
    {
        VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescriptorInfo{};
        variableDescriptorInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
        variableDescriptorInfo.pNext = nullptr;
        variableDescriptorInfo.descriptorSetCount = 1;
        variableDescriptorInfo.pDescriptorCounts = &bindlessElementsCount;
        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.pNext = bindGroupLayout->bindlessElementIndex != UINT32_MAX ? &variableDescriptorInfo : nullptr;
        allocateInfo.descriptorSetCount = 1;
        allocateInfo.pSetLayouts = &bindGroupLayout->handle;
        allocateInfo.descriptorPool = m_descriptorPool;
        auto result = vkAllocateDescriptorSets(m_context->m_device, &allocateInfo, &bindGroup->descriptorSet);
        if (result != VK_SUCCESS)
            return ResultCode::ErrorOutOfMemory;

        return ResultCode::Success;
    }

    void BindGroupAllocator::ShutdownBindGroup(IBindGroup* bindGroup)
    {
        vkFreeDescriptorSets(m_context->m_device, m_descriptorPool, 1, &bindGroup->descriptorSet);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Image
    ///////////////////////////////////////////////////////////////////////////

    ResultCode IImage::Init(IContext* context, const ImageCreateInfo& _createInfo)
    {
        auto formatInfo = GetFormatInfo(_createInfo.format);

        this->flags = {};
        this->imageType = ConvertImageType(_createInfo.type);
        this->format = ConvertFormat(_createInfo.format);
        this->extent = ConvertExtent3D(_createInfo.size);
        this->mipLevels = _createInfo.mipLevels;
        this->arrayLayers = _createInfo.arrayCount;
        this->samples = ConvertSampleCount(_createInfo.sampleCount);
        this->usage = ConvertImageUsageFlags(_createInfo.usageFlags);
        this->availableAspects = formatInfo.hasRed ? ImageAspect::Color : ImageAspect::Depth; // TODO: do this correctly

        this->initialState.pipelineStage  = { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, ACCESS_FLAGS_SHADER_READ };
        ;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.flags = 0u;
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        allocInfo.preferredFlags = 0u;
        allocInfo.memoryTypeBits = 0u;
        allocInfo.pool = VK_NULL_HANDLE;
        allocInfo.pUserData = nullptr;

        VkImageCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = this->flags;
        createInfo.imageType = this->imageType;
        createInfo.format = this->format;
        createInfo.extent = this->extent;
        createInfo.mipLevels = this->mipLevels;
        createInfo.arrayLayers = this->arrayLayers;
        createInfo.samples = this->samples;
        createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        createInfo.usage = this->usage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        auto result = vmaCreateImage(
            context->m_allocator,
            &createInfo,
            &allocInfo,
            &handle,
            &allocation.handle,
            &allocation.info);

        if (_createInfo.name)
        {
            context->SetDebugName(handle, _createInfo.name);
        }

        return ConvertResult(result);
    }

    ResultCode IImage::Init(IContext* context, VkImage image, const VkSwapchainCreateInfoKHR& swapchainCreateInfo)
    {
        (void)context;

        this->handle = image;

        this->flags = {};
        this->imageType = VK_IMAGE_TYPE_2D;
        this->format = swapchainCreateInfo.imageFormat;
        this->extent.width = swapchainCreateInfo.imageExtent.width;
        this->extent.height = swapchainCreateInfo.imageExtent.height;
        this->extent.depth = 1;
        this->mipLevels = 1;
        this->arrayLayers = swapchainCreateInfo.imageArrayLayers;
        this->samples = VK_SAMPLE_COUNT_1_BIT;
        this->usage = swapchainCreateInfo.imageUsage;

        return ResultCode::Success;
    }

    void IImage::Shutdown(IContext* context)
    {
        vmaDestroyImage(context->m_allocator, handle, allocation.handle);
    }

    VkMemoryRequirements IImage::GetMemoryRequirements(IContext* context) const
    {
        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(context->m_device, handle, &requirements);
        return requirements;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Buffer
    ///////////////////////////////////////////////////////////////////////////

    ResultCode IBuffer::Init(IContext* context, const BufferCreateInfo& _createInfo)
    {
        this->flags = {};
        this->size = _createInfo.byteSize;
        this->usage = ConvertBufferUsageFlags(_createInfo.usageFlags);

        VmaAllocationCreateInfo allocInfo{};
        // allocInfo.flags = 0u;
        // allocInfo.usage = _createInfo.heapType == MemoryType::GPUShared ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        allocInfo.usage = _createInfo.heapType == MemoryType::GPUShared ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        // allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        allocInfo.preferredFlags = 0u;
        allocInfo.memoryTypeBits = 0u;
        allocInfo.pool = VK_NULL_HANDLE;
        allocInfo.pUserData = nullptr;

        VkBufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = this->flags;
        createInfo.size = this->size;
        createInfo.usage = this->usage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        auto result = vmaCreateBuffer(
            context->m_allocator,
            &createInfo,
            &allocInfo,
            &handle,
            &allocation.handle,
            &allocation.info);

        if (result == VK_SUCCESS)
        {
            context->SetDebugName(handle, _createInfo.name);
        }

        return ConvertResult(result);
    }

    void IBuffer::Shutdown(IContext* context)
    {
        vmaDestroyBuffer(context->m_allocator, handle, allocation.handle);
    }

    VkMemoryRequirements IBuffer::GetMemoryRequirements(IContext* context) const
    {
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(context->m_device, handle, &requirements);
        return requirements;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// ImageView
    ///////////////////////////////////////////////////////////////////////////

    ResultCode IImageView::Init(IContext* context, const ImageViewCreateInfo& _createInfo)
    {
        auto image = context->m_imageOwner.Get(_createInfo.image);
        RHI_ASSERT(image);

        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.image = image->handle;

        switch (image->imageType)
        {
        case VK_IMAGE_TYPE_1D: createInfo.viewType = _createInfo.subresource.arrayCount == 1 ? VK_IMAGE_VIEW_TYPE_1D : VK_IMAGE_VIEW_TYPE_1D_ARRAY; break;
        case VK_IMAGE_TYPE_2D: createInfo.viewType = _createInfo.subresource.arrayCount == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY; break;
        case VK_IMAGE_TYPE_3D: createInfo.viewType = VK_IMAGE_VIEW_TYPE_3D; break;
        default:               RHI_UNREACHABLE(); break;
        }

        createInfo.format = image->format;
        createInfo.components = ConvertComponentMapping(_createInfo.components);
        createInfo.subresourceRange = ConvertSubresourceRange(_createInfo.subresource);

        auto result = vkCreateImageView(context->m_device, &createInfo, nullptr, &handle);
        return ConvertResult(result);
    }

    void IImageView::Shutdown(IContext* context)
    {
        vkDestroyImageView(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// BufferView
    ///////////////////////////////////////////////////////////////////////////

    ResultCode IBufferView::Init(IContext* context, const BufferViewCreateInfo& createInfo)
    {
        auto buffer = context->m_bufferOwner.Get(createInfo.buffer);
        RHI_ASSERT(buffer);

        VkBufferViewCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = 0;
        vkCreateInfo.buffer = buffer->handle;
        vkCreateInfo.format = ConvertFormat(createInfo.format);
        vkCreateInfo.offset = createInfo.subregion.offset;
        vkCreateInfo.range = createInfo.subregion.size;

        auto result = vkCreateBufferView(context->m_device, &vkCreateInfo, nullptr, &handle);
        return ConvertResult(result);
    }

    void IBufferView::Shutdown(IContext* context)
    {
        vkDestroyBufferView(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// BindGroupLayout
    ///////////////////////////////////////////////////////////////////////////

    ResultCode IBindGroupLayout::Init(IContext* context, const BindGroupLayoutCreateInfo& createInfo)
    {
        layoutInfo = createInfo;
        bindlessElementIndex = UINT32_MAX;

        TL::Vector<VkDescriptorBindingFlags> bindingFlags;
        TL::Vector<VkDescriptorSetLayoutBinding> bindingInfos;

        for (uint32_t index = 0; index < c_MaxBindGroupElementsCount; index++)
        {
            auto binding = createInfo.bindings[index];

            if (binding.type == ShaderBindingType::None)
                break;

            VkDescriptorBindingFlags flags{};
            if (binding.arrayCount == ShaderBinding::VariableArraySize)
            {
                flags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
                flags |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                bindlessElementIndex = index;
            }
            bindingFlags.push_back(flags);

            VkDescriptorSetLayoutBinding descriptorBinding{};
            descriptorBinding.binding = index;
            descriptorBinding.descriptorType = ConvertDescriptorType(binding.type);
            descriptorBinding.descriptorCount = binding.arrayCount;
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

    ///////////////////////////////////////////////////////////////////////////
    /// BindGroup
    ///////////////////////////////////////////////////////////////////////////

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

    void IBindGroup::Write(IContext* context, TL::Span<const ResourceBinding> bindingResources)
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
            const ResourceBinding& binding = bindingResources[i];

            DescriptorWriteData& data = writeData.emplace_back();

            VkWriteDescriptorSet writeInfo = {};
            writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.dstSet = descriptorSet;
            writeInfo.dstBinding = binding.binding;
            writeInfo.dstArrayElement = binding.dstArrayElement;
            writeInfo.descriptorType = ConvertDescriptorType(info.bindings[binding.binding].type);
            switch (binding.type)
            {
            case ResourceBinding::Type::Image:
                for (size_t j = 0; j < binding.data.images.size(); ++j)
                {
                    auto& imageInfo = data.imageInfos.emplace_back();

                    imageInfo.imageView = context->m_imageViewOwner.Get(binding.data.images[j])->handle;
                    switch (info.bindings[binding.binding].type)
                    {
                    case ShaderBindingType::SampledImage: imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; break;
                    case ShaderBindingType::StorageImage: imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; break;
                    default:                              RHI_UNREACHABLE(); break;
                    }
                }
                writeInfo.pImageInfo = data.imageInfos.data();
                writeInfo.descriptorCount = uint32_t(binding.data.images.size());
                break;

            case ResourceBinding::Type::Buffer:
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

            case ResourceBinding::Type::DynamicBuffer:
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

            case ResourceBinding::Type::Sampler:
                for (size_t j = 0; j < binding.data.samplers.size(); ++j)
                {
                    auto& imageInfo = data.imageInfos.emplace_back();
                    imageInfo.sampler = context->m_samplerOwner.Get(binding.data.samplers[j])->handle;
                }
                writeInfo.pImageInfo = data.imageInfos.data();
                writeInfo.descriptorCount = uint32_t(binding.data.samplers.size());
                break;

            default:
                RHI_UNREACHABLE();
                break;
            }

            writeInfos.push_back(writeInfo);
        }

        vkUpdateDescriptorSets(context->m_device, uint32_t(writeInfos.size()), writeInfos.data(), 0, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// PipelineLayout
    ///////////////////////////////////////////////////////////////////////////

    ResultCode IPipelineLayout::Init(IContext* context, const PipelineLayoutCreateInfo& createInfo)
    {
        TL::Vector<VkDescriptorSetLayout> descriptorSetLayouts;
        for (auto bindGroupLayout : createInfo.layouts)
        {
            if (bindGroupLayout == RHI::NullHandle)
            {
                break;
            }

            auto layout = context->m_bindGroupLayoutsOwner.Get(bindGroupLayout);
            descriptorSetLayouts.push_back(layout->handle);
        }

        VkPipelineLayoutCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = 0;
        vkCreateInfo.setLayoutCount = uint32_t(descriptorSetLayouts.size());
        vkCreateInfo.pSetLayouts = descriptorSetLayouts.data();
        vkCreateInfo.pushConstantRangeCount = 0;
        vkCreateInfo.pPushConstantRanges = nullptr;

        auto result = vkCreatePipelineLayout(context->m_device, &vkCreateInfo, nullptr, &handle);
        return ConvertResult(result);
    }

    void IPipelineLayout::Shutdown(IContext* context)
    {
        vkDestroyPipelineLayout(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// GraphicsPipeline
    ///////////////////////////////////////////////////////////////////////////

    ResultCode IGraphicsPipeline::Init(IContext* context, const GraphicsPipelineCreateInfo& createInfo)
    {
        uint32_t stagesCreateInfoCount = 2;
        VkPipelineShaderStageCreateInfo stagesCreateInfos[4];
        {
            VkPipelineShaderStageCreateInfo stageInfo{};
            stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageInfo.pNext = nullptr;
            stageInfo.flags = 0;
            stageInfo.pSpecializationInfo = nullptr;
            stageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            stageInfo.module = static_cast<IShaderModule*>(createInfo.vertexShaderModule)->m_shaderModule;
            stageInfo.pName = createInfo.vertexShaderName;
            stagesCreateInfos[0] = stageInfo;

            stageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            stageInfo.module = static_cast<IShaderModule*>(createInfo.pixelShaderModule)->m_shaderModule;
            stageInfo.pName = createInfo.pixelShaderName;
            stagesCreateInfos[1] = stageInfo;
        }

        uint32_t vertexInputBindingDescriptionsCount = 0;
        VkVertexInputBindingDescription vertexInputBindingDescriptions[c_MaxPipelineVertexBindings] = {};
        for (auto bindingDesc : createInfo.inputAssemblerState.bindings)
        {
            if (bindingDesc.stepRate == PipelineVertexInputRate::None)
            {
                break;
            }
            auto& binding = vertexInputBindingDescriptions[vertexInputBindingDescriptionsCount++];
            binding.binding = bindingDesc.binding;
            binding.stride = bindingDesc.stride;
            binding.inputRate = bindingDesc.stepRate == PipelineVertexInputRate::PerVertex ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
        }

        uint32_t inputAttributeDescriptionsCount = 0;
        VkVertexInputAttributeDescription inputAttributeDescriptions[c_MaxPipelineVertexBindings] = {};
        for (auto attributeDesc : createInfo.inputAssemblerState.attributes)
        {
            if (attributeDesc.format == Format::Unknown)
            {
                break;
            }
            auto& attribute = inputAttributeDescriptions[inputAttributeDescriptionsCount++];
            attribute.location = attributeDesc.location;
            attribute.binding = attributeDesc.binding;
            attribute.format = ConvertFormat(attributeDesc.format);
            attribute.offset = attributeDesc.offset;
        }

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.pNext = nullptr;
        vertexInputStateCreateInfo.flags = 0;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = vertexInputBindingDescriptionsCount;
        vertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions;
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = inputAttributeDescriptionsCount;
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = inputAttributeDescriptions;

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
        inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreateInfo.pNext = nullptr;
        inputAssemblyStateCreateInfo.flags = 0;
        inputAssemblyStateCreateInfo.topology = ConvertPrimitiveTopology(createInfo.topologyMode);
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo{};
        tessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tessellationStateCreateInfo.pNext = nullptr;
        tessellationStateCreateInfo.flags = 0;
        // tessellationStateCreateInfo.patchControlPoints;

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
        viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.pNext = nullptr;
        viewportStateCreateInfo.flags = 0;
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.scissorCount = 1;
        viewportStateCreateInfo.pScissors = nullptr;
        viewportStateCreateInfo.pViewports = nullptr;

        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
        rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateCreateInfo.pNext = nullptr;
        rasterizationStateCreateInfo.flags = 0;
        rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
        rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateCreateInfo.polygonMode = ConvertPolygonMode(createInfo.rasterizationState.fillMode);
        rasterizationStateCreateInfo.cullMode = ConvertCullModeFlags(createInfo.rasterizationState.cullMode);
        rasterizationStateCreateInfo.frontFace = ConvertFrontFace(createInfo.rasterizationState.frontFace);
        rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
        rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
        rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
        rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
        rasterizationStateCreateInfo.lineWidth = createInfo.rasterizationState.lineWidth;

        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
        multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCreateInfo.pNext = nullptr;
        multisampleStateCreateInfo.flags = 0;
        multisampleStateCreateInfo.rasterizationSamples = ConvertSampleCount(createInfo.multisampleState.sampleCount);
        multisampleStateCreateInfo.sampleShadingEnable = createInfo.multisampleState.sampleShading ? VK_TRUE : VK_FALSE;
        multisampleStateCreateInfo.minSampleShading = float(uint32_t(multisampleStateCreateInfo.rasterizationSamples)) / 2.0f;
        multisampleStateCreateInfo.pSampleMask = nullptr;
        multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
        depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCreateInfo.pNext = nullptr;
        depthStencilStateCreateInfo.flags = 0;
        depthStencilStateCreateInfo.depthTestEnable = createInfo.depthStencilState.depthTestEnable ? VK_TRUE : VK_FALSE;
        depthStencilStateCreateInfo.depthWriteEnable = createInfo.depthStencilState.depthWriteEnable ? VK_TRUE : VK_FALSE;
        depthStencilStateCreateInfo.depthCompareOp = ConvertCompareOp(createInfo.depthStencilState.compareOperator);
        depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCreateInfo.stencilTestEnable = createInfo.depthStencilState.stencilTestEnable ? VK_TRUE : VK_FALSE;
        // depthStencilStateCreateInfo.front;
        // depthStencilStateCreateInfo.back;
        depthStencilStateCreateInfo.minDepthBounds = 0.0;
        depthStencilStateCreateInfo.maxDepthBounds = 1.0;

        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.pNext = nullptr;
        dynamicStateCreateInfo.flags = 0;
        dynamicStateCreateInfo.dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState);
        dynamicStateCreateInfo.pDynamicStates = dynamicStates;

        uint32_t colorAttachmentFormatCount = 0;
        VkFormat colorAttachmentFormats[c_MaxRenderTargetAttachmentsCount] = {};

        for (uint32_t formatIndex = 0; formatIndex < c_MaxRenderTargetAttachmentsCount; formatIndex++)
        {
            auto format = createInfo.renderTargetLayout.colorAttachmentsFormats[formatIndex];
            if (format == Format::Unknown)
            {
                break;
            }
            colorAttachmentFormatCount++;
            colorAttachmentFormats[formatIndex] = ConvertFormat(format);
        }

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
        VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentStates[c_MaxRenderTargetAttachmentsCount] = {};

        uint32_t location = 0;
        for (auto blendState : createInfo.colorBlendState.blendStates)
        {
            if (location >= colorAttachmentFormatCount)
            {
                break;
            }

            auto& state = pipelineColorBlendAttachmentStates[location++];
            state.blendEnable = blendState.blendEnable ? VK_TRUE : VK_FALSE;
            state.srcColorBlendFactor = ConvertBlendFactor(blendState.srcColor);
            state.dstColorBlendFactor = ConvertBlendFactor(blendState.dstColor);
            state.colorBlendOp = ConvertBlendOp(blendState.colorBlendOp);
            state.srcAlphaBlendFactor = ConvertBlendFactor(blendState.srcAlpha);
            state.dstAlphaBlendFactor = ConvertBlendFactor(blendState.dstAlpha);
            state.alphaBlendOp = ConvertBlendOp(blendState.alphaBlendOp);
            state.colorWriteMask = 0;
            if (blendState.writeMask & ColorWriteMask::Red)
            {
                state.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
            }

            if (blendState.writeMask & ColorWriteMask::Green)
            {
                state.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
            }

            if (blendState.writeMask & ColorWriteMask::Blue)
            {
                state.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
            }

            if (blendState.writeMask & ColorWriteMask::Alpha)
            {
                state.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
            }
        }

        colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.pNext = nullptr;
        colorBlendStateCreateInfo.flags = 0;
        colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_SET;
        colorBlendStateCreateInfo.attachmentCount = colorAttachmentFormatCount;
        colorBlendStateCreateInfo.pAttachments = pipelineColorBlendAttachmentStates;
        colorBlendStateCreateInfo.blendConstants[0] = createInfo.colorBlendState.blendConstants[0];
        colorBlendStateCreateInfo.blendConstants[1] = createInfo.colorBlendState.blendConstants[1];
        colorBlendStateCreateInfo.blendConstants[2] = createInfo.colorBlendState.blendConstants[2];
        colorBlendStateCreateInfo.blendConstants[3] = createInfo.colorBlendState.blendConstants[3];

        VkPipelineRenderingCreateInfo renderTargetLayout{};
        renderTargetLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        renderTargetLayout.pNext = nullptr;
        // renderTargetLayout.viewMask;
        renderTargetLayout.colorAttachmentCount = colorAttachmentFormatCount;
        renderTargetLayout.pColorAttachmentFormats = colorAttachmentFormats;
        renderTargetLayout.depthAttachmentFormat = ConvertFormat(createInfo.renderTargetLayout.depthAttachmentFormat);
        renderTargetLayout.stencilAttachmentFormat = ConvertFormat(createInfo.renderTargetLayout.stencilAttachmentFormat);

        layout = context->m_pipelineLayoutOwner.Get(createInfo.layout)->handle;

        VkGraphicsPipelineCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        vkCreateInfo.pNext = &renderTargetLayout;
        vkCreateInfo.flags = 0;
        vkCreateInfo.stageCount = stagesCreateInfoCount;
        vkCreateInfo.pStages = stagesCreateInfos;
        vkCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
        vkCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
        vkCreateInfo.pTessellationState = &tessellationStateCreateInfo;
        vkCreateInfo.pViewportState = &viewportStateCreateInfo;
        vkCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
        vkCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
        vkCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
        vkCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
        vkCreateInfo.pDynamicState = &dynamicStateCreateInfo;
        vkCreateInfo.layout = layout;
        vkCreateInfo.renderPass = VK_NULL_HANDLE;
        vkCreateInfo.subpass = 0;
        vkCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        vkCreateInfo.basePipelineIndex = 0;
        auto result = vkCreateGraphicsPipelines(context->m_device, VK_NULL_HANDLE, 1, &vkCreateInfo, nullptr, &handle);
        if (result == VK_SUCCESS)
        {
            context->SetDebugName(handle, createInfo.name);
        }
        return ConvertResult(result);
    }

    void IGraphicsPipeline::Shutdown(IContext* context)
    {
        vkDestroyPipeline(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// ComputePipeline
    ///////////////////////////////////////////////////////////////////////////

    ResultCode IComputePipeline::Init(IContext* context, const ComputePipelineCreateInfo& createInfo)
    {
        auto shaderModule = static_cast<IShaderModule*>(createInfo.shaderModule);

        layout = context->m_pipelineLayoutOwner.Get(createInfo.layout)->handle;

        VkPipelineShaderStageCreateInfo shaderStage{};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.pNext = nullptr;
        shaderStage.flags = 0;
        shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStage.module = shaderModule->m_shaderModule;
        shaderStage.pName = createInfo.shaderName;
        shaderStage.pSpecializationInfo = nullptr;

        VkComputePipelineCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = {};
        vkCreateInfo.stage = shaderStage;
        vkCreateInfo.layout = layout;
        vkCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        vkCreateInfo.basePipelineIndex = 0;

        auto result = vkCreateComputePipelines(context->m_device, VK_NULL_HANDLE, 1, &vkCreateInfo, nullptr, &handle);
        return ConvertResult(result);
    }

    void IComputePipeline::Shutdown(IContext* context)
    {
        vkDestroyPipeline(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Sampler
    ///////////////////////////////////////////////////////////////////////////

    ResultCode ISampler::Init(IContext* context, const SamplerCreateInfo& createInfo)
    {
        VkSamplerCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = 0;
        vkCreateInfo.magFilter = ConvertFilter(createInfo.filterMag);
        vkCreateInfo.minFilter = ConvertFilter(createInfo.filterMin);
        vkCreateInfo.mipmapMode = createInfo.filterMip == SamplerFilter::Linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
        vkCreateInfo.addressModeU = ConvertSamplerAddressMode(createInfo.addressU);
        vkCreateInfo.addressModeV = ConvertSamplerAddressMode(createInfo.addressV);
        vkCreateInfo.addressModeW = ConvertSamplerAddressMode(createInfo.addressW);
        vkCreateInfo.mipLodBias = createInfo.mipLodBias;
        vkCreateInfo.anisotropyEnable = VK_TRUE;
        vkCreateInfo.maxAnisotropy = 1.0f;
        vkCreateInfo.compareEnable = VK_TRUE;
        vkCreateInfo.compareOp = ConvertCompareOp(createInfo.compare);
        vkCreateInfo.minLod = createInfo.minLod;
        vkCreateInfo.maxLod = createInfo.maxLod;
        vkCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        vkCreateInfo.unnormalizedCoordinates = VK_FALSE;
        auto result = vkCreateSampler(context->m_device, &vkCreateInfo, nullptr, &handle);
        if (result == VK_SUCCESS)
        {
            context->SetDebugName(handle, createInfo.name);
        }
        return ConvertResult(result);
    }

    void ISampler::Shutdown(IContext* context)
    {
        vkDestroySampler(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// ShaderModule
    ///////////////////////////////////////////////////////////////////////////

    IShaderModule::~IShaderModule()
    {
        vkDestroyShaderModule(((IContext*)m_context)->m_device, m_shaderModule, nullptr);
    }

    ResultCode IShaderModule::Init(TL::Span<const uint32_t> shaderBlob)
    {
        auto context = static_cast<IContext*>(m_context);

        m_spirv.resize(shaderBlob.size());
        std::copy(shaderBlob.begin(), shaderBlob.end(), m_spirv.begin());

        VkShaderModuleCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .codeSize = shaderBlob.size_bytes(),
            .pCode = m_spirv.data()
        };

        return ConvertResult(vkCreateShaderModule(context->m_device, &createInfo, nullptr, &m_shaderModule));
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Fence
    ///////////////////////////////////////////////////////////////////////////

    IFence::~IFence()
    {
        vkDestroyFence(m_context->m_device, m_fence, nullptr);
    }

    ResultCode IFence::Init()
    {
        m_state = FenceState::NotSubmitted;

        VkFenceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        TryValidateVk(vkCreateFence(m_context->m_device, &createInfo, nullptr, &m_fence));
        return ResultCode::Success;
    }

    void IFence::Reset()
    {
        m_state = FenceState::NotSubmitted;
        Validate(vkResetFences(m_context->m_device, 1, &m_fence));
    }

    bool IFence::WaitInternal(uint64_t timeout)
    {
        if (m_state == FenceState::NotSubmitted)
            return VK_SUCCESS;

        return Validate(vkWaitForFences(m_context->m_device, 1, &m_fence, VK_TRUE, timeout));
    }

    FenceState IFence::GetState()
    {
        if (m_state == FenceState::Pending)
        {
            auto result = vkGetFenceStatus(m_context->m_device, m_fence);
            return result == VK_SUCCESS ? FenceState::Signaled : FenceState::Pending;
        }

        return FenceState::NotSubmitted;
    }

    VkFence IFence::UseFence()
    {
        m_state = FenceState::Pending;
        return m_fence;
    }
} // namespace RHI::Vulkan