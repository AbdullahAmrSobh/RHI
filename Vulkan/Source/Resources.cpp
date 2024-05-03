#include <RHI/Common/Assert.hpp>

#include "Common.hpp"
#include "Context.hpp"
#include "FrameScheduler.hpp"
#include "Resources.hpp"

#define TIMEOUT_DURATION 9000000

namespace RHI::Vulkan
{
    ///////////////////////////////////////////////////////////////////////////
    /// BindGroupAllocator
    ///////////////////////////////////////////////////////////////////////////

    BindGroupAllocator::BindGroupAllocator(VkDevice device)
        : m_device(device)
    {
        // clang-format off
            VkDescriptorPoolSize poolSizes[] = {
                { VK_DESCRIPTOR_TYPE_SAMPLER,                1024 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1024 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1024 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1024 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1024 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1024 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1024 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1024 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1024 },
            };
        // clang-format on

        VkDescriptorPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        createInfo.maxSets = 8;
        createInfo.poolSizeCount = sizeof(poolSizes) / sizeof(VkDescriptorPoolSize);
        createInfo.pPoolSizes = poolSizes;

        auto result = vkCreateDescriptorPool(m_device, &createInfo, nullptr, &m_descriptorPool);
        VULKAN_ASSERT_SUCCESS(result);
    }

    void BindGroupAllocator::Shutdown()
    {
        vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
    }

    ResultCode BindGroupAllocator::InitBindGroup(IBindGroup* bindGroup, IBindGroupLayout* bindGroupLayout)
    {
        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.descriptorSetCount = 1;
        allocateInfo.pSetLayouts = &bindGroupLayout->handle;
        allocateInfo.descriptorPool = m_descriptorPool;
        auto result = vkAllocateDescriptorSets(m_device, &allocateInfo, &bindGroup->descriptorSet);
        if (result != VK_SUCCESS)
            return ResultCode::ErrorOutOfMemory;

        return ResultCode::Success;
    }

    void BindGroupAllocator::ShutdownBindGroup(IBindGroup* bindGroup)
    {
        vkFreeDescriptorSets(m_device, m_descriptorPool, 1, &bindGroup->descriptorSet);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Image
    ///////////////////////////////////////////////////////////////////////////

    ResultCode IImage::Init(IContext* context, const ImageCreateInfo& _createInfo, bool isTransient)
    {
        signalSemaphore = VK_NULL_HANDLE;
        waitSemaphore   = VK_NULL_HANDLE;

        VkImageCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = {};
        createInfo.imageType = ConvertImageType(_createInfo.type);
        createInfo.format = ConvertFormat(_createInfo.format);
        createInfo.extent = ConvertExtent3D(_createInfo.size);
        createInfo.mipLevels = _createInfo.mipLevels;
        createInfo.arrayLayers = _createInfo.arrayCount;
        createInfo.samples = ConvertSampleCount(_createInfo.sampleCount);
        createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        createInfo.usage = ConvertImageUsageFlags(_createInfo.usageFlags);
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        this->format = createInfo.format;
        this->imageType = createInfo.imageType;
        VkResult result = VK_ERROR_UNKNOWN;

        if (isTransient)
        {
            result = vkCreateImage(context->m_device, &createInfo, nullptr, &handle);
        }
        else
        {
            VmaAllocationCreateInfo allocationInfo{};

            if (_createInfo.allocationInfo.pool)
            {
                auto resourcePool = (IResourcePool*)_createInfo.allocationInfo.pool;
                allocationInfo.pool = resourcePool->m_pool;
            }
            else
            {
            }

            result = vmaCreateImage(context->m_allocator, &createInfo, &allocationInfo, &handle, &allocation.handle, &allocation.info);
        }

        context->SetDebugName(VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, uint64_t(handle), _createInfo.name);

        return ConvertResult(result);
    }

    void IImage::Shutdown(IContext* context)
    {
        if (pool)
        {
            vmaDestroyImage(context->m_allocator, handle, allocation.handle);
        }
        else
        {
            vkDestroyImage(context->m_device, handle, nullptr);
        }
    }

    VkMemoryRequirements IImage::GetMemoryRequirements(VkDevice device) const
    {
        VkMemoryRequirements requirements{};
        vkGetImageMemoryRequirements(device, handle, &requirements);
        return requirements;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Buffer
    ///////////////////////////////////////////////////////////////////////////

    ResultCode IBuffer::Init(IContext* context, const BufferCreateInfo& _createInfo, bool isTransient)
    {
        VkBufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = {};
        createInfo.size = _createInfo.byteSize;
        createInfo.usage = ConvertBufferUsageFlags(_createInfo.usageFlags);
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;

        VkResult result = VK_ERROR_UNKNOWN;
        if (isTransient)
        {
            result = vkCreateBuffer(context->m_device, &createInfo, nullptr, &handle);
        }
        else
        {
            VmaAllocationCreateInfo allocationInfo{};

            if (_createInfo.allocationInfo.pool)
            {
                auto resourcePool = (IResourcePool*)_createInfo.allocationInfo.pool;
                allocationInfo.pool = resourcePool->m_pool;
            }
            else
            {
                allocationInfo.usage = _createInfo.allocationInfo.heapType == MemoryType::GPUShared ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
                allocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
            }

            result = vmaCreateBuffer(context->m_allocator, &createInfo, &allocationInfo, &handle, &allocation.handle, &allocation.info);
        }

        return ConvertResult(result);
    }

    void IBuffer::Shutdown(IContext* context)
    {
        if (pool)
        {
            vmaDestroyBuffer(context->m_allocator, handle, allocation.handle);
        }
        else
        {
            vkDestroyBuffer(context->m_device, handle, nullptr);
        }
    }

    VkMemoryRequirements IBuffer::GetMemoryRequirements(VkDevice device) const
    {
        VkMemoryRequirements requirements{};
        vkGetBufferMemoryRequirements(device, handle, &requirements);
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
        vkCreateInfo.offset = createInfo.byteOffset;
        vkCreateInfo.range = createInfo.byteSize;

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

        TL::Vector<VkDescriptorSetLayoutBinding> bindings;
        for (auto shaderBinding : createInfo.bindings)
        {
            if (shaderBinding.type == ShaderBindingType::None)
            {
                break;
            }

            auto& binding = bindings.emplace_back<VkDescriptorSetLayoutBinding>({});
            binding.binding = uint32_t(bindings.size() - 1);
            binding.descriptorType = ConvertDescriptorType(shaderBinding.type);
            binding.descriptorCount = shaderBinding.arrayCount;
            binding.stageFlags = ConvertShaderStage(shaderBinding.stages);
            binding.pImmutableSamplers = nullptr;
        }

        VkDescriptorSetLayoutCreateInfo vkCreateInfo;
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = 0;
        vkCreateInfo.bindingCount = uint32_t(bindings.size());
        vkCreateInfo.pBindings = bindings.data();

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

    ResultCode IBindGroup::Init(IContext* context, Handle<BindGroupLayout> layoutHandle)
    {
        layout = layoutHandle;

        auto allocator = context->m_bindGroupAllocator.get();
        auto layoutObject = context->m_bindGroupLayoutsOwner.Get(layoutHandle);

        return allocator->InitBindGroup(this, layoutObject);
    }

    void IBindGroup::Shutdown(IContext* context)
    {
        auto allocator = context->m_bindGroupAllocator.get();
        allocator->ShutdownBindGroup(this);
    }

    void IBindGroup::Write(IContext* context, BindGroupData data)
    {
        TL::Vector<TL::Vector<VkDescriptorImageInfo>> descriptorImageInfos;
        TL::Vector<TL::Vector<VkDescriptorBufferInfo>> descriptorBufferInfos;
        TL::Vector<TL::Vector<VkBufferView>> descriptorBufferViews;

        TL::Vector<VkWriteDescriptorSet> writeInfos;

        auto bindGroupLayout = context->m_bindGroupLayoutsOwner.Get(layout);

        for (uint32_t binding = 0; binding < c_MaxShaderBindGroupElementsCount; binding++)
        {
            auto resourceVarient = data.m_bindings[binding];

            if (bindGroupLayout->layoutInfo.bindings[binding].type == ShaderBindingType::None)
                break;

            VkWriteDescriptorSet writeInfo{};
            writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.pNext = nullptr;
            writeInfo.dstSet = descriptorSet;
            writeInfo.dstBinding = binding;
            if (auto resources = std::get_if<0>(&resourceVarient))
            {
                auto& imageInfos = descriptorImageInfos.emplace_back();

                for (auto viewHandle : resources->views)
                {
                    auto view = context->m_imageViewOwner.Get(viewHandle);

                    auto type = ConvertDescriptorType(bindGroupLayout->layoutInfo.bindings[binding].type);

                    VkDescriptorImageInfo imageInfo{};
                    imageInfo.imageLayout = type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView = view->handle;
                    imageInfos.push_back(imageInfo);
                }

                writeInfo.dstArrayElement = resources->arrayOffset;
                writeInfo.descriptorType = ConvertDescriptorType(bindGroupLayout->layoutInfo.bindings[binding].type);
                writeInfo.descriptorCount = uint32_t(imageInfos.size());
                writeInfo.pImageInfo = imageInfos.data();
            }
            else if (auto buffer = std::get_if<1>(&resourceVarient))
            {
                auto& bufferInfos = descriptorBufferInfos.emplace_back();

                for (auto viewHandle : buffer->views)
                {
                    VkDescriptorBufferInfo bufferInfo{};
                    bufferInfo.buffer = context->m_bufferOwner.Get(viewHandle)->handle;
                    bufferInfo.offset = 0;
                    bufferInfo.range = buffer->elementSize == SIZE_MAX ? VK_WHOLE_SIZE : buffer->elementSize;
                    bufferInfos.push_back(bufferInfo);
                }

                writeInfo.dstArrayElement = buffer->arrayOffset;
                writeInfo.descriptorType = buffer->dynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // todo support storage buffers, and more complex types
                writeInfo.descriptorCount = uint32_t(bufferInfos.size());
                writeInfo.pBufferInfo = bufferInfos.data();
            }
            else if (auto sampler = std::get_if<2>(&resourceVarient))
            {
                auto& imageInfos = descriptorImageInfos.emplace_back();

                for (auto samplerHandle : sampler->samplers)
                {
                    auto sampler2 = context->m_samplerOwner.Get(samplerHandle);

                    VkDescriptorImageInfo imageInfo{};
                    imageInfo.sampler = sampler2->handle;
                    imageInfos.push_back(imageInfo);
                }

                writeInfo.dstArrayElement = sampler->arrayOffset;
                writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                writeInfo.descriptorCount = uint32_t(imageInfos.size());
                writeInfo.pImageInfo = imageInfos.data();
            }
            else
            {
                RHI_UNREACHABLE();
            }

            // writeInfo.pTexelBufferView; todo

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
        vkCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // ConvertFilter(createInfo.filterMip);
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
        vkDestroyShaderModule(m_context->m_device, m_shaderModule, nullptr);
    }

    VkResult IShaderModule::Init(TL::Span<const uint8_t> shaderBlob)
    {
        auto context = static_cast<IContext*>(m_context);

        // Check alignment and log warning if necessary
        if (reinterpret_cast<uintptr_t>(shaderBlob.data()) % 4 != 0)
        {
            context->DebugLogWarn("Shader blob requires 4-byte alignment. Reallocating and copying for Vulkan compatibility.");

            // Calculate aligned size and allocate memory
            size_t alignedSize = shaderBlob.size() + 4 - reinterpret_cast<uintptr_t>(shaderBlob.data()) % 4;
            uint8_t* alignedData = new uint8_t[alignedSize];

            // Copy data and set alignedBlob
            std::memcpy(alignedData, shaderBlob.data(), shaderBlob.size());
            TL::Span<uint8_t> alignedBlob = { alignedData, alignedSize };

            // Use alignedBlob for shader module creation
            VkShaderModuleCreateInfo moduleCreateInfo{
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .codeSize = alignedBlob.size(),
                .pCode = reinterpret_cast<const uint32_t*>(alignedBlob.data()),
            };

            VkResult res = vkCreateShaderModule(context->m_device, &moduleCreateInfo, nullptr, &m_shaderModule);

            // Deallocate temporary buffer
            delete[] alignedData;

            return res;
        }
        else
        {
            // No alignment needed, use original blob
            VkShaderModuleCreateInfo moduleCreateInfo{
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .codeSize = shaderBlob.size(),
                .pCode = (const uint32_t*)shaderBlob.data()
            };

            return vkCreateShaderModule(context->m_device, &moduleCreateInfo, nullptr, &m_shaderModule);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    /// ResourcePool
    ///////////////////////////////////////////////////////////////////////////

    IResourcePool::~IResourcePool()
    {
        vmaDestroyPool(m_context->m_allocator, m_pool);
    }

    VkResult IResourcePool::Init(const ResourcePoolCreateInfo& createInfo)
    {
        m_poolInfo = createInfo;

        VmaPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.flags = VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT;
        poolCreateInfo.blockSize = createInfo.blockSize;
        poolCreateInfo.minBlockCount = createInfo.minBlockCount;
        poolCreateInfo.maxBlockCount = createInfo.maxBlockCount;
        poolCreateInfo.priority = 1.0f;
        poolCreateInfo.minAllocationAlignment = createInfo.minBlockAlignment;
        poolCreateInfo.pMemoryAllocateNext = nullptr;
        poolCreateInfo.memoryTypeIndex = m_context->GetMemoryTypeIndex(MemoryType::GPULocal);
        return vmaCreatePool(m_context->m_allocator, &poolCreateInfo, &m_pool);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Fence
    ///////////////////////////////////////////////////////////////////////////

    IFence::~IFence()
    {
        vkDestroyFence(m_context->m_device, m_fence, nullptr);
    }

    VkResult IFence::Init()
    {
        m_state = FenceState::NotSubmitted;

        VkFenceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        auto result = vkCreateFence(m_context->m_device, &createInfo, nullptr, &m_fence);
        VULKAN_ASSERT_SUCCESS(result);
        return result;
    }

    void IFence::Reset()
    {
        m_state = FenceState::NotSubmitted;
        auto result = vkResetFences(m_context->m_device, 1, &m_fence);
        VULKAN_ASSERT_SUCCESS(result);
    }

    bool IFence::WaitInternal(uint64_t timeout)
    {
        if (m_state == FenceState::NotSubmitted)
            return VK_SUCCESS;

        auto result = vkWaitForFences(m_context->m_device, 1, &m_fence, VK_TRUE, timeout);
        return result == VK_SUCCESS;
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