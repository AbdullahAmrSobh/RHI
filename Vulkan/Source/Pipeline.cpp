#include "Pipeline.hpp"
#include "device.hpp"
#include "Common.hpp"

namespace RHI::Vulkan
{
    // TODO: remove or assert
    inline static constexpr uint32_t c_MaxRenderTargetAttachmentsCount = 16u;
    inline static constexpr uint32_t c_MaxPipelineVertexBindings = 32u;
    inline static constexpr uint32_t c_MaxPipelineVertexAttributes = 32u;
    inline static constexpr uint32_t c_MaxPipelineBindGroupsCount = 4u;

    inline static VkBool32 ConvertBool(bool value)
    {
        return value ? VK_TRUE : VK_FALSE;
    }

    VkShaderStageFlagBits ConvertShaderStage(ShaderStage shaderStage)
    {
        switch (shaderStage)
        {
        case ShaderStage::Vertex:  return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::Pixel:   return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
        default:                   TL_UNREACHABLE(); return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        }
    }

    VkShaderStageFlags ConvertShaderStage(TL::Flags<ShaderStage> shaderStageFlags)
    {
        VkShaderStageFlags result = 0;
        if (shaderStageFlags & ShaderStage::Vertex) result |= VK_SHADER_STAGE_VERTEX_BIT;
        if (shaderStageFlags & ShaderStage::Pixel) result |= VK_SHADER_STAGE_FRAGMENT_BIT;
        if (shaderStageFlags & ShaderStage::Compute) result |= VK_SHADER_STAGE_COMPUTE_BIT;
        return result;
    }

    VkVertexInputRate ConvertVertexInputRate(PipelineVertexInputRate inputRate)
    {
        switch (inputRate)
        {
        case PipelineVertexInputRate::PerInstance: return VK_VERTEX_INPUT_RATE_INSTANCE;
        case PipelineVertexInputRate::PerVertex:   return VK_VERTEX_INPUT_RATE_VERTEX;
        default:                                   TL_UNREACHABLE(); return VK_VERTEX_INPUT_RATE_MAX_ENUM;
        }
    }

    VkCullModeFlags ConvertCullModeFlags(PipelineRasterizerStateCullMode cullMode)
    {
        switch (cullMode)
        {
        case PipelineRasterizerStateCullMode::None:      return VK_CULL_MODE_NONE;
        case PipelineRasterizerStateCullMode::FrontFace: return VK_CULL_MODE_FRONT_BIT;
        case PipelineRasterizerStateCullMode::BackFace:  return VK_CULL_MODE_BACK_BIT;
        case PipelineRasterizerStateCullMode::Discard:   return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
        default:                                         TL_UNREACHABLE(); return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
        }
    }

    VkPolygonMode ConvertPolygonMode(PipelineRasterizerStateFillMode fillMode)
    {
        switch (fillMode)
        {
        case PipelineRasterizerStateFillMode::Point:    return VK_POLYGON_MODE_POINT;
        case PipelineRasterizerStateFillMode::Triangle: return VK_POLYGON_MODE_FILL;
        case PipelineRasterizerStateFillMode::Line:     return VK_POLYGON_MODE_LINE;
        default:                                        TL_UNREACHABLE(); return VK_POLYGON_MODE_MAX_ENUM;
        }
    }

    VkPrimitiveTopology ConvertPrimitiveTopology(PipelineTopologyMode topologyMode)
    {
        switch (topologyMode)
        {
        case PipelineTopologyMode::Points:    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PipelineTopologyMode::Lines:     return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PipelineTopologyMode::Triangles: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        default:                              TL_UNREACHABLE(); return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
        }
    }

    VkFrontFace ConvertFrontFace(PipelineRasterizerStateFrontFace frontFace)
    {
        switch (frontFace)
        {
        case PipelineRasterizerStateFrontFace::Clockwise:        return VK_FRONT_FACE_CLOCKWISE;
        case PipelineRasterizerStateFrontFace::CounterClockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        default:                                                 TL_UNREACHABLE(); return VK_FRONT_FACE_MAX_ENUM;
        }
    }

    VkCompareOp ConvertCompareOp(CompareOperator compareOperator)
    {
        switch (compareOperator)
        {
        case CompareOperator::Never:          return VK_COMPARE_OP_NEVER;
        case CompareOperator::Equal:          return VK_COMPARE_OP_EQUAL;
        case CompareOperator::NotEqual:       return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOperator::Greater:        return VK_COMPARE_OP_GREATER;
        case CompareOperator::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOperator::Less:           return VK_COMPARE_OP_LESS;
        case CompareOperator::LessOrEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOperator::Always:         return VK_COMPARE_OP_ALWAYS;
        default:                              TL_UNREACHABLE(); return VK_COMPARE_OP_MAX_ENUM;
        }
    }

    VkBlendFactor ConvertBlendFactor(BlendFactor blendFactor)
    {
        switch (blendFactor)
        {
        case BlendFactor::Zero:                  return VK_BLEND_FACTOR_ZERO;
        case BlendFactor::One:                   return VK_BLEND_FACTOR_ONE;
        case BlendFactor::SrcColor:              return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFactor::OneMinusSrcColor:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DstColor:              return VK_BLEND_FACTOR_DST_COLOR;
        case BlendFactor::OneMinusDstColor:      return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendFactor::SrcAlpha:              return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFactor::OneMinusSrcAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DstAlpha:              return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendFactor::OneMinusDstAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendFactor::ConstantColor:         return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case BlendFactor::OneMinusConstantColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case BlendFactor::ConstantAlpha:         return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case BlendFactor::OneMinusConstantAlpha: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        default:                                 TL_UNREACHABLE(); return VK_BLEND_FACTOR_MAX_ENUM;
        }
    }

    VkBlendOp ConvertBlendOp(BlendEquation blendEquation)
    {
        switch (blendEquation)
        {
        case BlendEquation::Add:             return VK_BLEND_OP_ADD;
        case BlendEquation::Subtract:        return VK_BLEND_OP_SUBTRACT;
        case BlendEquation::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendEquation::Min:             return VK_BLEND_OP_MIN;
        case BlendEquation::Max:             return VK_BLEND_OP_MAX;
        default:                             TL_UNREACHABLE(); return VK_BLEND_OP_MAX_ENUM;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    /// PipelineLayout
    ///////////////////////////////////////////////////////////////////////////

    ResultCode IPipelineLayout::Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo)
    {
        TL::Vector<VkDescriptorSetLayout> descriptorSetLayouts;
        for (auto bindGroupLayout : createInfo.layouts)
        {
            if (bindGroupLayout == RHI::NullHandle)
            {
                break;
            }

            auto layout = device->m_bindGroupLayoutsOwner.Get(bindGroupLayout);
            descriptorSetLayouts.push_back(layout->handle);
        }

        VkPipelineLayoutCreateInfo vkCreateInfo{

            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = uint32_t(descriptorSetLayouts.size()),
            .pSetLayouts = descriptorSetLayouts.data(),
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
        };
        auto result = vkCreatePipelineLayout(device->m_device, &vkCreateInfo, nullptr, &handle);
        if (result == VK_SUCCESS && createInfo.name)
        {
            device->SetDebugName(handle, createInfo.name);
        }
        return ConvertResult(result);
    }

    void IPipelineLayout::Shutdown(IDevice* device)
    {
        vkDestroyPipelineLayout(device->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// GraphicsPipeline
    ///////////////////////////////////////////////////////////////////////////

    ResultCode IGraphicsPipeline::Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo)
    {
        uint32_t stagesCreateInfoCount = 2;
        VkPipelineShaderStageCreateInfo stagesCreateInfos[4];
        {
            VkPipelineShaderStageCreateInfo vertexStageCI{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = static_cast<IShaderModule*>(createInfo.vertexShaderModule)->m_shaderModule,
                .pName = createInfo.vertexShaderName,
                .pSpecializationInfo = nullptr,
            };
            stagesCreateInfos[0] = vertexStageCI;

            VkPipelineShaderStageCreateInfo pixelStageCI{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = static_cast<IShaderModule*>(createInfo.pixelShaderModule)->m_shaderModule,
                .pName = createInfo.pixelShaderName,
                .pSpecializationInfo = nullptr,
            };
            stagesCreateInfos[1] = pixelStageCI;
        }

        uint32_t vertexBindingsCount = 0;
        uint32_t vertexAttributesCount = 0;

        VkVertexInputBindingDescription vertexBindings[c_MaxPipelineVertexBindings] = {};
        VkVertexInputAttributeDescription vertexAttributes[c_MaxPipelineVertexAttributes] = {}; // Adjusted size for vertexAttributes

        for (const auto& bindingDesc : createInfo.vertexBufferBindings)
        {
            // Set up vertex binding
            auto& binding = vertexBindings[vertexBindingsCount];
            binding.binding = bindingDesc.binding;
            binding.stride = bindingDesc.stride;
            binding.inputRate =
                bindingDesc.stepRate == PipelineVertexInputRate::PerVertex ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;

            // Iterate through vertex attributes for this binding
            for (const auto& attributeDesc : bindingDesc.attributes)
            {
                // Set up vertex attribute
                auto& attribute = vertexAttributes[vertexAttributesCount];
                attribute.location = vertexAttributesCount;
                attribute.binding = bindingDesc.binding;
                attribute.format = ConvertFormat(attributeDesc.format);
                attribute.offset = attributeDesc.offset;

                vertexAttributesCount++;
            }

            vertexBindingsCount++;
        }

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = vertexBindingsCount,
            .pVertexBindingDescriptions = vertexBindings,
            .vertexAttributeDescriptionCount = vertexAttributesCount,
            .pVertexAttributeDescriptions = vertexAttributes,
        };

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .topology = ConvertPrimitiveTopology(createInfo.topologyMode),
            .primitiveRestartEnable = VK_FALSE,
        };

        VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .patchControlPoints = 0,
        };

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .viewportCount = 1,
            .pViewports = nullptr,
            .scissorCount = 1,
            .pScissors = nullptr,
        };

        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = ConvertPolygonMode(createInfo.rasterizationState.fillMode),
            .cullMode = ConvertCullModeFlags(createInfo.rasterizationState.cullMode),
            .frontFace = ConvertFrontFace(createInfo.rasterizationState.frontFace),
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = createInfo.rasterizationState.lineWidth,
        };

        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .rasterizationSamples = ConvertSampleCount(createInfo.multisampleState.sampleCount),
            .sampleShadingEnable = createInfo.multisampleState.sampleShading ? VK_TRUE : VK_FALSE,
            .minSampleShading = float(uint32_t(multisampleStateCreateInfo.rasterizationSamples)) / 2.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE,
        };

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthTestEnable = createInfo.depthStencilState.depthTestEnable ? VK_TRUE : VK_FALSE,
            .depthWriteEnable = createInfo.depthStencilState.depthWriteEnable ? VK_TRUE : VK_FALSE,
            .depthCompareOp = ConvertCompareOp(createInfo.depthStencilState.compareOperator),
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = createInfo.depthStencilState.stencilTestEnable ? VK_TRUE : VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = 0.0,
            .maxDepthBounds = 1.0,
        };

        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState),
            .pDynamicStates = dynamicStates,
        };

        uint32_t colorAttachmentFormatCount = 0;
        VkFormat colorAttachmentFormats[c_MaxRenderTargetAttachmentsCount] = {};

        for (uint32_t formatIndex = 0; formatIndex < createInfo.renderTargetLayout.colorAttachmentsFormats.size(); formatIndex++)
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

        layout = device->m_pipelineLayoutOwner.Get(createInfo.layout)->handle;

        VkGraphicsPipelineCreateInfo graphicsPipelineCI{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &renderTargetLayout,
            .flags = 0,
            .stageCount = stagesCreateInfoCount,
            .pStages = stagesCreateInfos,
            .pVertexInputState = &vertexInputStateCreateInfo,
            .pInputAssemblyState = &inputAssemblyStateCreateInfo,
            .pTessellationState = &tessellationStateCreateInfo,
            .pViewportState = &viewportStateCreateInfo,
            .pRasterizationState = &rasterizationStateCreateInfo,
            .pMultisampleState = &multisampleStateCreateInfo,
            .pDepthStencilState = &depthStencilStateCreateInfo,
            .pColorBlendState = &colorBlendStateCreateInfo,
            .pDynamicState = &dynamicStateCreateInfo,
            .layout = layout,
            .renderPass = VK_NULL_HANDLE,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0,
        };
        auto result = vkCreateGraphicsPipelines(device->m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCI, nullptr, &handle);
        if (result == VK_SUCCESS && createInfo.name)
        {
            device->SetDebugName(handle, createInfo.name);
        }
        return ConvertResult(result);
    }

    void IGraphicsPipeline::Shutdown(IDevice* device)
    {
        vkDestroyPipeline(device->m_device, handle, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// ComputePipeline
    ///////////////////////////////////////////////////////////////////////////

    ResultCode IComputePipeline::Init(IDevice* device, const ComputePipelineCreateInfo& createInfo)
    {
        auto shaderModule = static_cast<IShaderModule*>(createInfo.shaderModule);

        layout = device->m_pipelineLayoutOwner.Get(createInfo.layout)->handle;

        VkPipelineShaderStageCreateInfo shaderStageCI{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = shaderModule->m_shaderModule,
            .pName = createInfo.shaderName,
            .pSpecializationInfo = nullptr,
        };

        VkComputePipelineCreateInfo computePipelineCI{
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .stage = shaderStageCI,
            .layout = layout,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0,
        };

        auto result = vkCreateComputePipelines(device->m_device, VK_NULL_HANDLE, 1, &computePipelineCI, nullptr, &handle);
        return ConvertResult(result);
    }

    void IComputePipeline::Shutdown(IDevice* device)
    {
        vkDestroyPipeline(device->m_device, handle, nullptr);
    }

} // namespace RHI::Vulkan