#include <RHI/Common/Assert.hpp>

#include "Conversion.hpp"
#include "Common.hpp"
#include "Context.hpp"
#include "Format.inl"
#include "FrameScheduler.hpp"
#include "Resources.hpp"

#include <Windows.h>
#include <algorithm>

#define TIMEOUT_DURATION 9000000

namespace Vulkan
{
    ///////////////////////////////////////////////////////////////////////////
    /// Image
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode Image::Init(Context* context, const VmaAllocationCreateInfo allocationInfo, const RHI::ImageCreateInfo& createInfo, ImagePool* parentPool, bool isTransientResource)
    {
        pool = parentPool;

        VkImageCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = {};
        vkCreateInfo.imageType = ConvertImageType(createInfo.type);
        vkCreateInfo.format = ConvertFormat(createInfo.format);
        vkCreateInfo.extent.width = createInfo.size.width;
        vkCreateInfo.extent.height = createInfo.size.height;
        vkCreateInfo.extent.depth = createInfo.size.depth;
        vkCreateInfo.mipLevels = createInfo.mipLevels;
        vkCreateInfo.arrayLayers = createInfo.arrayCount;
        vkCreateInfo.samples = ConvertSampleCount(createInfo.sampleCount);
        vkCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        vkCreateInfo.usage = ConvertImageUsageFlags(createInfo.usageFlags);
        vkCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkCreateInfo.queueFamilyIndexCount = 0;
        vkCreateInfo.pQueueFamilyIndices = nullptr;
        vkCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        this->format = vkCreateInfo.format;
        this->imageType = vkCreateInfo.imageType;
        VkResult result = VK_ERROR_UNKNOWN;

        if (isTransientResource)
        {
            result = vkCreateImage(context->m_device, &vkCreateInfo, nullptr, &handle);
        }
        else
        {
            result = vmaCreateImage(context->m_allocator, &vkCreateInfo, &allocationInfo, &handle, &allocation.handle, &allocation.info);
        }

        return ConvertResult(result);
    }

    void Image::Shutdown(Context* context)
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

    VkMemoryRequirements Image::GetMemoryRequirements(VkDevice device) const
    {
        VkMemoryRequirements requirements{};
        vkGetImageMemoryRequirements(device, handle, &requirements);
        return requirements;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Buffer
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode Buffer::Init(Context* context, const VmaAllocationCreateInfo allocationInfo, const RHI::BufferCreateInfo& createInfo, BufferPool* parentPool, bool isTransientResource)
    {
        pool = parentPool;

        VkBufferCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = {};
        vkCreateInfo.size = createInfo.byteSize;
        vkCreateInfo.usage = ConvertBufferUsageFlags(createInfo.usageFlags);
        vkCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkCreateInfo.queueFamilyIndexCount = 0;
        vkCreateInfo.pQueueFamilyIndices = nullptr;

        VkResult result = VK_ERROR_UNKNOWN;
        if (isTransientResource)
        {
            result = vkCreateBuffer(context->m_device, &vkCreateInfo, nullptr, &handle);
        }
        else
        {
            result = vmaCreateBuffer(context->m_allocator, &vkCreateInfo, &allocationInfo, &handle, &allocation.handle, &allocation.info);
        }

        return ConvertResult(result);
    }

    void Buffer::Shutdown(Context* context)
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

    VkMemoryRequirements Buffer::GetMemoryRequirements(VkDevice device) const
    {
        VkMemoryRequirements requirements{};
        vkGetBufferMemoryRequirements(device, handle, &requirements);
        return requirements;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// ImageView
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode ImageView::Init(Context* context, RHI::Handle<Image> imageHandle, const RHI::ImageViewCreateInfo& createInfo)
    {
        auto image = context->m_imageOwner.Get(imageHandle);
        RHI_ASSERT(image);

        VkImageViewCreateInfo vkCreateInfo{};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vkCreateInfo.pNext = nullptr;
        vkCreateInfo.flags = 0;
        vkCreateInfo.image = image->handle;

        switch (image->imageType)
        {
        case VK_IMAGE_TYPE_1D: vkCreateInfo.viewType = createInfo.subresource.arrayCount == 1 ? VK_IMAGE_VIEW_TYPE_1D : VK_IMAGE_VIEW_TYPE_1D_ARRAY; break;
        case VK_IMAGE_TYPE_2D: vkCreateInfo.viewType = createInfo.subresource.arrayCount == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY; break;
        case VK_IMAGE_TYPE_3D: vkCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_3D; break;
        default:               RHI_UNREACHABLE(); break;
        }

        vkCreateInfo.format = image->format;
        vkCreateInfo.components.r = ConvertComponentSwizzle(createInfo.components.r);
        vkCreateInfo.components.g = ConvertComponentSwizzle(createInfo.components.g);
        vkCreateInfo.components.b = ConvertComponentSwizzle(createInfo.components.b);
        vkCreateInfo.components.a = ConvertComponentSwizzle(createInfo.components.a);
        vkCreateInfo.subresourceRange = ConvertSubresourceRange(createInfo.subresource);

        auto result = vkCreateImageView(context->m_device, &vkCreateInfo, nullptr, &handle);
        return ConvertResult(result);
    }

    void ImageView::Shutdown(Context* context)
    {
        vkDestroyImageView(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// BufferView
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode BufferView::Init(Context* context, RHI::Handle<Buffer> bufferHandle, const RHI::BufferViewCreateInfo& createInfo)
    {
        auto buffer = context->m_bufferOwner.Get(bufferHandle);
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

    void BufferView::Shutdown(Context* context)
    {
        vkDestroyBufferView(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// BindGroupLayout
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode BindGroupLayout::Init(Context* context, const RHI::BindGroupLayoutCreateInfo& createInfo)
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (auto shaderBinding : createInfo.bindings)
        {
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

    void BindGroupLayout::Shutdown(Context* context)
    {
        vkDestroyDescriptorSetLayout(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// BindGroup
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode BindGroup::Init(Context* context, VkDescriptorSetLayout layout, VkDescriptorPool descriptorPool)
    {
        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.descriptorPool = descriptorPool;
        allocateInfo.descriptorSetCount = 1;
        allocateInfo.pSetLayouts = &layout;

        auto result = vkAllocateDescriptorSets(context->m_device, &allocateInfo, &handle);
        return ConvertResult(result);
    }

    void BindGroup::Shutdown(Context* context)
    {
        (void)context;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// PipelineLayout
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode PipelineLayout::Init(Context* context, const RHI::PipelineLayoutCreateInfo& createInfo)
    {
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        for (auto bindGroupLayout : createInfo.layouts)
        {
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

    void PipelineLayout::Shutdown(Context* context)
    {
        vkDestroyPipelineLayout(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// GraphicsPipeline
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode GraphicsPipeline::Init(Context* context, const RHI::GraphicsPipelineCreateInfo& createInfo)
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
            stageInfo.module = static_cast<ShaderModule*>(createInfo.vertexShaderModule)->m_shaderModule;
            stageInfo.pName = createInfo.vertexShaderName;
            stagesCreateInfos[0] = stageInfo;

            stageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            stageInfo.module = static_cast<ShaderModule*>(createInfo.pixelShaderModule)->m_shaderModule;
            stageInfo.pName = createInfo.pixelShaderName;
            stagesCreateInfos[1] = stageInfo;
        }

        std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions;

        for (auto attributeDesc : createInfo.inputAssemblerState.attributes)
        {
            VkVertexInputAttributeDescription attribute{};
            attribute.location = attributeDesc.location;
            attribute.binding = attributeDesc.binding;
            attribute.format = ConvertFormat(attributeDesc.format);
            attribute.offset = attributeDesc.offset;
            inputAttributeDescriptions.push_back(attribute);
        }

        for (auto bindingDesc : createInfo.inputAssemblerState.bindings)
        {
            VkVertexInputBindingDescription binding{};
            binding.binding = bindingDesc.binding;
            binding.stride = bindingDesc.stride;
            binding.inputRate = bindingDesc.stepRate == RHI::PipelineVertexInputRate::PerVertex ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
            vertexInputBindingDescriptions.push_back(binding);
        }

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.pNext = nullptr;
        vertexInputStateCreateInfo.flags = 0;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindingDescriptions.size());
        vertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(inputAttributeDescriptions.size());
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = inputAttributeDescriptions.data();

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
        multisampleStateCreateInfo.minSampleShading = float(multisampleStateCreateInfo.rasterizationSamples / 2);
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

        uint32_t pipelineColorBlendAttachmentStateCount = 0;
        VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentStates[8];

        for (auto blendState : createInfo.colorBlendState.blendStates)
        {
            auto& state = pipelineColorBlendAttachmentStates[pipelineColorBlendAttachmentStateCount++];
            state.blendEnable = blendState.blendEnable ? VK_TRUE : VK_FALSE;
            state.srcColorBlendFactor = ConvertBlendFactor(blendState.srcColor);
            state.dstColorBlendFactor = ConvertBlendFactor(blendState.dstColor);
            state.colorBlendOp = ConvertBlendOp(blendState.colorBlendOp);
            state.srcAlphaBlendFactor = ConvertBlendFactor(blendState.srcAlpha);
            state.dstAlphaBlendFactor = ConvertBlendFactor(blendState.dstAlpha);
            state.alphaBlendOp = ConvertBlendOp(blendState.alphaBlendOp);
            state.colorWriteMask = 0;
            if (blendState.writeMask & RHI::ColorWriteMask::Red)
            {
                state.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
            }

            if (blendState.writeMask & RHI::ColorWriteMask::Green)
            {
                state.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
            }

            if (blendState.writeMask & RHI::ColorWriteMask::Blue)
            {
                state.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
            }

            if (blendState.writeMask & RHI::ColorWriteMask::Alpha)
            {
                state.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
            }
        }

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
        colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.pNext = nullptr;
        colorBlendStateCreateInfo.flags = 0;
        colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_SET;
        colorBlendStateCreateInfo.attachmentCount = pipelineColorBlendAttachmentStateCount;
        colorBlendStateCreateInfo.pAttachments = pipelineColorBlendAttachmentStates;
        colorBlendStateCreateInfo.blendConstants[0] = createInfo.colorBlendState.blendConstants[0];
        colorBlendStateCreateInfo.blendConstants[1] = createInfo.colorBlendState.blendConstants[1];
        colorBlendStateCreateInfo.blendConstants[2] = createInfo.colorBlendState.blendConstants[2];
        colorBlendStateCreateInfo.blendConstants[3] = createInfo.colorBlendState.blendConstants[3];

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.pNext = nullptr;
        dynamicStateCreateInfo.flags = 0;
        dynamicStateCreateInfo.dynamicStateCount = uint32_t(dynamicStates.size());
        dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

        uint32_t colorAttachmentFormatCount = static_cast<uint32_t>(createInfo.renderTargetLayout.colorAttachmentsFormats.size());
        VkFormat colorAttachmentFormats[8] = {};

        uint32_t index = 0;
        for (auto format : createInfo.renderTargetLayout.colorAttachmentsFormats)
            colorAttachmentFormats[index++] = ConvertFormat(format);

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

    void GraphicsPipeline::Shutdown(Context* context)
    {
        vkDestroyPipeline(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// ComputePipeline
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode ComputePipeline::Init(Context* context, const RHI::ComputePipelineCreateInfo& createInfo)
    {
        auto shaderModule = static_cast<ShaderModule*>(createInfo.shaderModule);

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

        ComputePipeline pipeline{};

        auto result = vkCreateComputePipelines(context->m_device, VK_NULL_HANDLE, 1, &vkCreateInfo, nullptr, &pipeline.handle);
        return ConvertResult(result);
    }

    void ComputePipeline::Shutdown(Context* context)
    {
        vkDestroyPipeline(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Sampler
    ///////////////////////////////////////////////////////////////////////////

    RHI::ResultCode Sampler::Init(Context* context, const RHI::SamplerCreateInfo& createInfo)
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

    void Sampler::Shutdown(Context* context)
    {
        vkDestroySampler(context->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// ShaderModule
    ///////////////////////////////////////////////////////////////////////////

    ShaderModule::~ShaderModule()
    {
        vkDestroyShaderModule(m_context->m_device, m_shaderModule, nullptr);
    }

    VkResult ShaderModule::Init(const RHI::ShaderModuleCreateInfo& createInfo)
    {
        auto context = static_cast<Context*>(m_context);

        VkShaderModuleCreateInfo moduleCreateInfo{};
        moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCreateInfo.pNext = nullptr;
        moduleCreateInfo.flags = {};
        moduleCreateInfo.codeSize = createInfo.size * 4;
        moduleCreateInfo.pCode = (uint32_t*)createInfo.code;
        RHI_ASSERT(moduleCreateInfo.codeSize % 4 == 0);

        return vkCreateShaderModule(context->m_device, &moduleCreateInfo, nullptr, &m_shaderModule);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// BindGroupAllocator
    ///////////////////////////////////////////////////////////////////////////

    BindGroupAllocator::~BindGroupAllocator()
    {
        auto context = static_cast<Context*>(m_context);

        for (auto pool : m_descriptorPools)
            vkDestroyDescriptorPool(context->m_device, pool, nullptr);
    }

    VkResult BindGroupAllocator::Init()
    {
        return VK_SUCCESS;
    }

    std::vector<RHI::Handle<RHI::BindGroup>> BindGroupAllocator::AllocateBindGroups(TL::Span<RHI::Handle<RHI::BindGroupLayout>> bindGroupLayouts)
    {
        auto context = static_cast<Context*>(m_context);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        for (auto bindGroupLayoutHandle : bindGroupLayouts)
        {
            auto bindGroupLayout = m_context->m_bindGroupLayoutsOwner.Get(bindGroupLayoutHandle);
            descriptorSetLayouts.push_back(bindGroupLayout->handle);
        }

        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.descriptorSetCount = uint32_t(descriptorSetLayouts.size());
        allocateInfo.pSetLayouts = descriptorSetLayouts.data();

        bool success = false;
        std::vector<VkDescriptorSet> descriptorSets;
        descriptorSets.resize(descriptorSetLayouts.size());
        for (auto descriptorPool : m_descriptorPools)
        {
            allocateInfo.descriptorPool = descriptorPool;

            auto result = vkAllocateDescriptorSets(context->m_device, &allocateInfo, descriptorSets.data());

            if (result == VK_SUCCESS)
            {
                success = true;
                break;
            }
        }

        if (success == false)
        {
            // clang-format off
            VkDescriptorPoolSize poolSizes[] = {
                { VK_DESCRIPTOR_TYPE_SAMPLER,                16 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          16 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          16 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         16 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         16 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   16 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   16 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 16 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 16 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       16 },
            };
            // clang-format on

            VkDescriptorPoolCreateInfo poolCreateInfo{};
            poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolCreateInfo.pNext = nullptr;
            poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            poolCreateInfo.maxSets = 8;
            poolCreateInfo.poolSizeCount = sizeof(poolSizes) / sizeof(VkDescriptorPoolSize); // todo recheck this
            poolCreateInfo.pPoolSizes = poolSizes;

            VkDescriptorPool newDescriptorPool{};
            auto result = vkCreateDescriptorPool(context->m_device, &poolCreateInfo, nullptr, &newDescriptorPool);
            VULKAN_ASSERT_SUCCESS(result);
            m_descriptorPools.push_back(newDescriptorPool);

            allocateInfo.descriptorPool = newDescriptorPool;

            result = vkAllocateDescriptorSets(context->m_device, &allocateInfo, descriptorSets.data());
            VULKAN_ASSERT_SUCCESS(result);
        }

        std::vector<RHI::Handle<RHI::BindGroup>> bindGroups;
        for (auto descriptorSet : descriptorSets)
        {
            auto [handle, bindGroup] = m_context->m_bindGroupOwner.InsertZerod();
            bindGroup.handle = descriptorSet;
            bindGroup.pool = allocateInfo.descriptorPool;
            bindGroups.push_back(handle);
        }

        return bindGroups;
    }

    void BindGroupAllocator::Free(TL::Span<RHI::Handle<RHI::BindGroup>> bindGroups)
    {
        for (auto group : bindGroups)
        {
            auto set = m_context->m_bindGroupOwner.Get(group);
            vkFreeDescriptorSets(m_context->m_device, set->pool, 1, &set->handle);
        }
    }

    void BindGroupAllocator::Update(RHI::Handle<RHI::BindGroup> _bindGroup, const RHI::BindGroupData& data)
    {
        auto context = static_cast<Context*>(m_context);
        auto bindGroup = m_context->m_bindGroupOwner.Get(_bindGroup);

        std::vector<std::vector<VkDescriptorImageInfo>> descriptorImageInfos;
        std::vector<std::vector<VkDescriptorBufferInfo>> descriptorBufferInfos;
        std::vector<std::vector<VkBufferView>> descriptorBufferViews;

        std::vector<VkWriteDescriptorSet> writeInfos;

        for (auto [binding, resourceVarient] : data.m_bindings)
        {
            VkWriteDescriptorSet writeInfo{};
            writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.pNext = nullptr;
            writeInfo.dstSet = bindGroup->handle;
            writeInfo.dstBinding = binding;
            if (auto resources = std::get_if<0>(&resourceVarient))
            {
                auto& imageInfos = descriptorImageInfos.emplace_back();

                for (auto viewHandle : resources->views)
                {
                    auto view = context->m_imageViewOwner.Get(viewHandle);

                    VkDescriptorImageInfo imageInfo{};
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView = view->handle;
                    imageInfos.push_back(imageInfo);
                }

                writeInfo.dstArrayElement = resources->arrayOffset;
                writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
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
                    bufferInfo.range = VK_WHOLE_SIZE;
                    bufferInfos.push_back(bufferInfo);
                }

                writeInfo.dstArrayElement = buffer->arrayOffset;
                writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // todo support storage buffers, and more complex types
                writeInfo.descriptorCount = uint32_t(bufferInfos.size());
                writeInfo.pBufferInfo = bufferInfos.data();
            }
            else if (auto sampler = std::get_if<2>(&resourceVarient))
            {
                auto& imageInfos = descriptorImageInfos.emplace_back();

                for (auto samplerHandle : sampler->samplers)
                {
                    auto sampler2 = m_context->m_samplerOwner.Get(samplerHandle);

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

        vkUpdateDescriptorSets(m_context->m_device, uint32_t(writeInfos.size()), writeInfos.data(), 0, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// BufferPool
    ///////////////////////////////////////////////////////////////////////////

    BufferPool::~BufferPool()
    {
        vmaDestroyPool(m_context->m_allocator, m_pool);
    }

    VkResult BufferPool::Init(const RHI::PoolCreateInfo& createInfo)
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
        poolCreateInfo.memoryTypeIndex = m_context->GetMemoryTypeIndex(createInfo.heapType);

        return vmaCreatePool(m_context->m_allocator, &poolCreateInfo, &m_pool);
    }

    RHI::Result<RHI::Handle<RHI::Buffer>> BufferPool::Allocate(const RHI::BufferCreateInfo& createInfo)
    {
        VmaAllocationCreateInfo allocationInfo{};
        allocationInfo.pool = m_pool;

        auto [handle, buffer] = m_context->m_bufferOwner.InsertZerod();
        if (auto result = buffer.Init(m_context, allocationInfo, createInfo, this, false); RHI::IsError(result))
        {
            return result;
        }

        return RHI::Handle<RHI::Buffer>(handle);
    }

    void BufferPool::FreeBuffer(RHI::Handle<RHI::Buffer> buffer)
    {
        auto resource = m_context->m_bufferOwner.Get(buffer);
        RHI_ASSERT(resource);

        vmaDestroyBuffer(m_context->m_allocator, resource->handle, resource->allocation.handle);
    }

    size_t BufferPool::GetSize(RHI::Handle<RHI::Buffer> buffer) const
    {
        auto& owner = m_context->m_bufferOwner;
        return owner.Get(buffer)->GetMemoryRequirements(m_context->m_device).size;
    }

    RHI::DeviceMemoryPtr BufferPool::MapBuffer(RHI::Handle<RHI::Buffer> handle)
    {
        auto resource = m_context->m_bufferOwner.Get(handle);
        auto allocation = resource->allocation.handle;

        RHI::DeviceMemoryPtr memoryPtr = nullptr;
        VkResult result = vmaMapMemory(m_context->m_allocator, allocation, &memoryPtr);
        VULKAN_ASSERT_SUCCESS(result);
        return memoryPtr;
    }

    void BufferPool::UnmapBuffer(RHI::Handle<RHI::Buffer> handle)
    {
        auto resource = m_context->m_bufferOwner.Get(handle)->allocation.handle;
        vmaUnmapMemory(m_context->m_allocator, resource);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// ImagePool
    ///////////////////////////////////////////////////////////////////////////

    ImagePool::~ImagePool()
    {
        vmaDestroyPool(m_context->m_allocator, m_pool);
    }

    VkResult ImagePool::Init(const RHI::PoolCreateInfo& createInfo)
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
        poolCreateInfo.memoryTypeIndex = m_context->GetMemoryTypeIndex(RHI::MemoryType::GPULocal);

        return vmaCreatePool(m_context->m_allocator, &poolCreateInfo, &m_pool);
    }

    RHI::Result<RHI::Handle<RHI::Image>> ImagePool::Allocate(const RHI::ImageCreateInfo& createInfo)
    {
        VmaAllocationCreateInfo allocationInfo{};
        allocationInfo.pool = m_pool;

        auto [handle, image] = m_context->m_imageOwner.InsertZerod();
        if (auto result = image.Init(m_context, allocationInfo, createInfo, this, false); RHI::IsError(result))
        {
            return result;
        }

        return RHI::Handle<RHI::Image>(handle);
    }

    void ImagePool::FreeImage(RHI::Handle<RHI::Image> handle)
    {
        auto resource = m_context->m_imageOwner.Get(handle);
        RHI_ASSERT(resource);

        vmaDestroyImage(m_context->m_allocator, resource->handle, resource->allocation.handle);
    }

    size_t ImagePool::GetSize(RHI::Handle<RHI::Image> handle) const
    {
        auto& owner = m_context->m_imageOwner;
        return owner.Get(handle)->GetMemoryRequirements(m_context->m_device).size;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Fence
    ///////////////////////////////////////////////////////////////////////////

    Fence::~Fence()
    {
        vkDestroyFence(m_context->m_device, m_fence, nullptr);
    }

    VkResult Fence::Init()
    {
        m_state = State::NotSubmitted;

        VkFenceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        auto result = vkCreateFence(m_context->m_device, &createInfo, nullptr, &m_fence);
        VULKAN_ASSERT_SUCCESS(result);
        return result;
    }

    void Fence::Reset()
    {
        m_state = State::NotSubmitted;
        auto result = vkResetFences(m_context->m_device, 1, &m_fence);
        VULKAN_ASSERT_SUCCESS(result);
    }

    bool Fence::WaitInternal(uint64_t timeout)
    {
        if (m_state == State::NotSubmitted)
            return VK_SUCCESS;

        auto result = vkWaitForFences(m_context->m_device, 1, &m_fence, VK_TRUE, timeout);
        return result == VK_SUCCESS;
    }

    Fence::State Fence::GetState()
    {
        if (m_state == State::Pending)
        {
            auto result = vkGetFenceStatus(m_context->m_device, m_fence);
            return result == VK_SUCCESS ? State::Signaled : State::Pending;
        }

        return State::NotSubmitted;
    }

    VkFence Fence::UseFence()
    {
        m_state = State::Pending;
        return m_fence;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Swapchain
    ///////////////////////////////////////////////////////////////////////////
    Swapchain::Swapchain(Context* context)
        : m_context(context)
        , m_imageReady{ VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE }
        , m_swapchain(VK_NULL_HANDLE)
        , m_surface(VK_NULL_HANDLE)
        , m_lastPresentResult(VK_ERROR_UNKNOWN)
        , m_swapchainInfo()
    {
    }

    Swapchain::~Swapchain()
    {
        auto context = static_cast<Context*>(m_context);

        vkDestroySwapchainKHR(context->m_device, m_swapchain, nullptr);
        vkDestroySurfaceKHR(context->m_instance, m_surface, nullptr);

        for (auto semaphore : m_imageReady)
        {
            if (semaphore != VK_NULL_HANDLE)
                vkDestroySemaphore(context->m_device, semaphore, nullptr);
        }
    }

    VkResult Swapchain::Init(const RHI::SwapchainCreateInfo& createInfo)
    {
        auto context = static_cast<Context*>(m_context);

        m_swapchainInfo = createInfo;
        m_swapchainImagesCount = createInfo.imageCount;

        for (uint32_t i = 0; i < createInfo.imageCount; i++)
        {
            m_imageReady[i] = context->CreateVulkanSemaphore();
        }

        VkResult result;
        result = InitSurface();
        VULKAN_ASSERT_SUCCESS(result);

        result = InitSwapchain();
        VULKAN_ASSERT_SUCCESS(result);

        return result;
    }

    VkSemaphore Swapchain::GetCurrentImageSemaphore()
    {
        return m_imageReady[m_currentImageIndex];
    }

    uint32_t Swapchain::AcquireNextImage(Fence& fence)
    {
        uint32_t acquiredImageIndex;
        auto result = vkAcquireNextImageKHR(m_context->m_device, m_swapchain, UINT64_MAX, VK_NULL_HANDLE, fence.UseFence(), &acquiredImageIndex);
        VULKAN_ASSERT_SUCCESS(result);
        return acquiredImageIndex;
    }

    uint32_t Swapchain::AcquireNextImage(VkSemaphore semaphore)
    {
        uint32_t acquiredImageIndex;
        auto result = vkAcquireNextImageKHR(m_context->m_device, m_swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &acquiredImageIndex);
        VULKAN_ASSERT_SUCCESS(result);
        return acquiredImageIndex;
    }

    RHI::ResultCode Swapchain::Resize(RHI::ImageSize2D newSize)
    {
        // auto context = static_cast<Context*>(m_context);
        // TODO Wait idle

        m_swapchainInfo.imageSize = { newSize.width, newSize.height, 1 };

        auto result = InitSwapchain();
        VULKAN_ASSERT_SUCCESS(result);
        return ConvertResult(result);
    }

    RHI::ResultCode Swapchain::Present(RHI::ImageAttachment& attachemnt)
    {
        auto context = (Context*)m_context;
        auto queue = context->GetQueue(RHI::QueueType::Graphics);
        auto swapchain = (Swapchain*)attachemnt.swapchain;
        auto waitPass = (Pass*)attachemnt.lastUse->pass;
        auto waitSemaphore = waitPass->m_signalSemaphore;
        auto signalSemaphore = swapchain->GetCurrentImageSemaphore();

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &waitSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain->m_swapchain;
        presentInfo.pImageIndices = &swapchain->m_currentImageIndex;
        presentInfo.pResults = &swapchain->m_lastPresentResult;
        auto result = vkQueuePresentKHR(queue, &presentInfo);
        VULKAN_ASSERT_SUCCESS(result);

        swapchain->AcquireNextImage(signalSemaphore);

        return RHI::ResultCode::Success;
    }

    VkResult Swapchain::InitSwapchain()
    {
        VkSurfaceCapabilitiesKHR surfaceCapabilities{};
        VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context->m_physicalDevice, m_surface, &surfaceCapabilities);
        VULKAN_RETURN_VKERR_CODE(result);

        auto surfaceFormat = GetSurfaceFormat(ConvertFormat(m_swapchainInfo.imageFormat));
        auto minImageCount = std::clamp(m_swapchainInfo.imageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
        auto minImageWidth = std::clamp(m_swapchainInfo.imageSize.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        auto minImageHeight = std::clamp(m_swapchainInfo.imageSize.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.surface = m_surface;
        createInfo.minImageCount = minImageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent.width = minImageWidth;
        createInfo.imageExtent.height = minImageHeight;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = ConvertImageUsageFlags(m_swapchainInfo.imageUsage);
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        createInfo.compositeAlpha = GetCompositeAlpha(surfaceCapabilities);
        createInfo.presentMode = GetPresentMode();
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = m_swapchain;

        result = vkCreateSwapchainKHR(m_context->m_device, &createInfo, nullptr, &m_swapchain);
        VULKAN_RETURN_VKERR_CODE(result);

        uint32_t imagesCount;
        result = vkGetSwapchainImagesKHR(m_context->m_device, m_swapchain, &imagesCount, nullptr);
        std::vector<VkImage> images;
        images.resize(imagesCount);
        result = vkGetSwapchainImagesKHR(m_context->m_device, m_swapchain, &imagesCount, images.data());
        VULKAN_RETURN_VKERR_CODE(result);

        for (auto imageHandles : images)
        {
            Vulkan::Image image{};
            image.handle = imageHandles;
            image.format = surfaceFormat.format;
            image.imageType = VK_IMAGE_TYPE_2D;
            image.swapchain = this;

            auto handle = m_context->m_imageOwner.Insert(image);
            m_images.push_back(handle);
        }

        m_currentImageIndex = AcquireNextImage(GetCurrentImageSemaphore());

        return result;
    }

    VkSurfaceFormatKHR Swapchain::GetSurfaceFormat(VkFormat format)
    {
        uint32_t formatsCount;

        VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->m_physicalDevice, m_surface, &formatsCount, nullptr);
        VULKAN_ASSERT_SUCCESS(result);

        std::vector<VkSurfaceFormatKHR> formats{};
        formats.resize(formatsCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->m_physicalDevice, m_surface, &formatsCount, formats.data());

        for (auto surfaceFormat : formats)
        {
            if (surfaceFormat.format == format)
                return surfaceFormat;
        }

        RHI_UNREACHABLE();

        return formats.front();
    }

    VkCompositeAlphaFlagBitsKHR Swapchain::GetCompositeAlpha(VkSurfaceCapabilitiesKHR surfaceCapabilities)
    {
        VkCompositeAlphaFlagBitsKHR preferedModes[] = { VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                                                        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
                                                        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                                                        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR };

        for (VkCompositeAlphaFlagBitsKHR mode : preferedModes)
        {
            if (surfaceCapabilities.supportedCompositeAlpha & mode)
            {
                return mode;
            }
        }

        RHI_UNREACHABLE();

        return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }

    VkPresentModeKHR Swapchain::GetPresentMode()
    {
        auto context = static_cast<Context*>(m_context);

        uint32_t presentModesCount;
        auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(context->m_physicalDevice, m_surface, &presentModesCount, nullptr);
        VULKAN_ASSERT_SUCCESS(result);
        std::vector<VkPresentModeKHR> presentModes{};
        presentModes.resize(presentModesCount);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(context->m_physicalDevice, m_surface, &presentModesCount, presentModes.data());

        VkPresentModeKHR preferredModes[] = { VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR };

        for (VkPresentModeKHR preferredMode : preferredModes)
        {
            for (VkPresentModeKHR supportedMode : presentModes)
            {
                if (supportedMode == preferredMode)
                {
                    return supportedMode;
                }
            }
        }

        // context->GetDebugMessenger().LogWarnning("Could not find preferred presentation mode");

        return presentModes[0];
    }

} // namespace Vulkan