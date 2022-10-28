#include "Backend/Vulkan/PipelineState.hpp"

#include "Backend/Vulkan/Common.hpp"
#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/RenderPass.hpp"

namespace RHI
{
namespace Vulkan
{


    Result<Unique<PipelineLayout>> PipelineLayout::Create(const Device& device, PipelineLayoutDesc& layoutDesc)
    {
        Unique<PipelineLayout> pipelineLayout = CreateUnique<PipelineLayout>(device);
        VkResult              result        = pipelineLayout->Init(layoutDesc);

        if (RHI_SUCCESS(result))
        {
            return ResultError(result);
        }

        return std::move(pipelineLayout);
    }

    Expected<Unique<IPipelineState>> Device::CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc)
    {
        Unique<PipelineState> pipelineState = CreateUnique<PipelineState>(*this);
        VkResult              result        = pipelineState->Init(desc);

        if (RHI_SUCCESS(result))
        {
            return Unexpected(ConvertResult(result));
        }

        return std::move(pipelineState);
    }

    PipelineLayout::~PipelineLayout()
    {
        vkDestroyPipelineLayout(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult PipelineLayout::Init(const PipelineLayoutDesc& layoutDesc)
    {
        
        size_t hash   = 0;
        
        size_t offset = 0;
        for (auto& layout : layoutDesc.shaderBindingGroupLayouts)
        {
            hash = hash_combine(hash, layout.GetHash());
            
            // For other resources

            for (auto& constants : layout.GetShaderConstantBufferBindings())
            {
                offset = constants.byteSize;
                VkPushConstantRange range;
                range.offset = offset;
                range.size   = constants.byteSize;
                m_pushConstantRanges.push_back(range);
            }

            
        }
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        std::transform(m_descriptorSetsLayouts.begin(), m_descriptorSetsLayouts.end(), std::back_insert_iterator(descriptorSetLayouts),
                       [](const auto& dsl) { return dsl->GetHandle(); });

        VkPipelineLayoutCreateInfo createInfo = {};
        createInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.pNext                      = nullptr;
        createInfo.flags                      = 0;
        createInfo.setLayoutCount             = CountElements(descriptorSetLayouts);
        createInfo.pSetLayouts                = descriptorSetLayouts.data();
        createInfo.pushConstantRangeCount     = CountElements(m_pushConstantRanges);
        createInfo.pPushConstantRanges        = m_pushConstantRanges.data();

        return vkCreatePipelineLayout(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    namespace PipelineStateInitalizers
    {
        struct ShaderStage
        {
            ShaderStage(const GraphicsPipelineShaderStages& shaderStages)
            {
                const ShaderModule* pVertexShader                 = static_cast<const ShaderModule*>(shaderStages.pVertexShader);
                const ShaderModule* pTessellationControlShader    = static_cast<const ShaderModule*>(shaderStages.pTessControlShader);
                const ShaderModule* pTessellationEvaluationShader = static_cast<const ShaderModule*>(shaderStages.pTessEvalShader);
                const ShaderModule* pGeometryShader               = static_cast<const ShaderModule*>(shaderStages.pGeometryShader);
                const ShaderModule* pFragmentShader               = static_cast<const ShaderModule*>(shaderStages.pPixelShader);

                VkPipelineShaderStageCreateInfo createInfo = {};
                createInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                createInfo.pNext                           = nullptr;
                createInfo.flags                           = 0;

                if (pVertexShader)
                {
                    createInfo.stage               = VK_SHADER_STAGE_VERTEX_BIT;
                    createInfo.module              = pVertexShader->GetHandle();
                    createInfo.pName               = pVertexShader->GetFunctionName().c_str();
                    createInfo.pSpecializationInfo = nullptr;
                    states.push_back(createInfo);
                }

                if (pTessellationControlShader)
                {
                    createInfo.stage               = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                    createInfo.module              = pTessellationControlShader->GetHandle();
                    createInfo.pName               = pTessellationControlShader->GetFunctionName().c_str();
                    createInfo.pSpecializationInfo = nullptr;
                    states.push_back(createInfo);
                }

                if (pTessellationEvaluationShader)
                {
                    createInfo.stage               = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                    createInfo.module              = pTessellationEvaluationShader->GetHandle();
                    createInfo.pName               = pTessellationEvaluationShader->GetFunctionName().c_str();
                    createInfo.pSpecializationInfo = nullptr;
                    states.push_back(createInfo);
                }

                if (pGeometryShader)
                {
                    createInfo.stage               = VK_SHADER_STAGE_GEOMETRY_BIT;
                    createInfo.module              = pGeometryShader->GetHandle();
                    createInfo.pName               = pGeometryShader->GetFunctionName().c_str();
                    createInfo.pSpecializationInfo = nullptr;
                    states.push_back(createInfo);
                }

                if (pFragmentShader)
                {

                    createInfo.stage               = VK_SHADER_STAGE_FRAGMENT_BIT;
                    createInfo.module              = pFragmentShader->GetHandle();
                    createInfo.pName               = pFragmentShader->GetFunctionName().c_str();
                    createInfo.pSpecializationInfo = nullptr;
                    states.push_back(createInfo);
                }
            }

            inline void Initalize(uint32_t& count, VkPipelineShaderStageCreateInfo const*& pState)
            {
                count  = CountElements(states);
                pState = states.data();
            }

            std::vector<VkPipelineShaderStageCreateInfo> states;
        };

        struct VertexInputState
        {
            VertexInputState(std::vector<GraphicsPipelineVertexAttributeState> vertexInputAttributes)
            {
                bindingDescription.stride = 0;

                uint32_t location = 0;

                for (auto inattribute : vertexInputAttributes)
                {
                    VkVertexInputAttributeDescription attribute = {};
                    attribute.binding                           = 0;
                    attribute.location                          = location;
                    attribute.format                            = ConvertFormat(inattribute.format);
                    attribute.offset                            = bindingDescription.stride;

                    bindingDescription.stride += FormatStrideSize(attribute.format);

                    location++;
                    attributes.push_back(attribute);
                }

                bindingDescription.binding   = 0;
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                state                                 = {};
                state.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                state.pNext                           = nullptr;
                state.flags                           = 0;
                state.vertexBindingDescriptionCount   = 1;
                state.pVertexBindingDescriptions      = &bindingDescription;
                state.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
                state.pVertexAttributeDescriptions    = attributes.data();
            }

            inline void Initalize(VkPipelineVertexInputStateCreateInfo const*& pState) const
            {
                pState = &state;
            }

            VkVertexInputBindingDescription                bindingDescription;
            std::vector<VkVertexInputAttributeDescription> attributes;
            VkPipelineVertexInputStateCreateInfo           state;
        };

        struct InputAssemblyState
        {
            InputAssemblyState()
            {
                state.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                state.pNext                  = nullptr;
                state.flags                  = 0;
                state.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                state.primitiveRestartEnable = VK_FALSE;
            }

            inline void Initalize(VkPipelineInputAssemblyStateCreateInfo const*& pState) const
            {
                pState = &state;
            }

            VkPipelineInputAssemblyStateCreateInfo state;
        };

        struct TessellationState
        {
            TessellationState()
            {

                state.sType              = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
                state.pNext              = nullptr;
                state.flags              = 0;
                state.patchControlPoints = 0;
            }

            inline void Initalize(VkPipelineTessellationStateCreateInfo const*& pState) const
            {
                pState = &state;
            }

            VkPipelineTessellationStateCreateInfo state;
        };

        struct ViewportState
        {
            ViewportState()
            {
                state.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                state.pNext         = nullptr;
                state.flags         = 0;
                state.viewportCount = 0;
                state.pViewports    = nullptr;
                state.scissorCount  = 0;
                state.pScissors     = nullptr;
            }

            inline void Initalize(VkPipelineViewportStateCreateInfo const*& pState) const
            {
                pState = &state;
            }

            VkPipelineViewportStateCreateInfo state;
        };

        struct RasterizationState
        {
            RasterizationState(const GraphicsPipelineRasterizationState& rasterStateDesc)
            {
                state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                state.pNext = nullptr;

                state.depthClampEnable = VK_FALSE;
                // discards all primitives before the rasterization stage if enabled which we don't want
                state.rasterizerDiscardEnable = VK_FALSE;

                state.polygonMode = VK_POLYGON_MODE_FILL;
                state.lineWidth   = 1.0f;
                // no backface cull
                state.cullMode  = VK_CULL_MODE_NONE;
                state.frontFace = VK_FRONT_FACE_CLOCKWISE;
                // no depth bias
                state.depthBiasEnable         = VK_FALSE;
                state.depthBiasConstantFactor = 0.0f;
                state.depthBiasClamp          = 0.0f;
                state.depthBiasSlopeFactor    = 0.0f;
            }

            inline void Initalize(VkPipelineRasterizationStateCreateInfo const*& pState) const
            {
                pState = &state;
            }

            VkPipelineRasterizationStateCreateInfo state;
        };

        struct MultisampleState
        {
            MultisampleState(ESampleCount sampleCount)
            {
                state.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                state.pNext                 = nullptr;
                state.flags                 = 0;
                state.rasterizationSamples  = ConvertSampleCount(sampleCount);
                state.sampleShadingEnable   = VK_FALSE;
                state.minSampleShading      = 1.0f;
                state.pSampleMask           = nullptr;
                state.alphaToCoverageEnable = VK_FALSE;
                state.alphaToOneEnable      = VK_FALSE;
            }

            inline void Initalize(VkPipelineMultisampleStateCreateInfo const*& pState) const
            {
                pState = &state;
            }

            VkPipelineMultisampleStateCreateInfo state;
        };

        struct DepthStencilState
        {
            DepthStencilState(const GraphicsPipelineDepthStencilState& depthStencil)
            {
                state.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                state.pNext                 = nullptr;
                state.flags                 = 0;
                state.depthTestEnable       = depthStencil.depthEnabled ? VK_TRUE : VK_FALSE;
                state.depthWriteEnable      = VK_TRUE;
                state.depthCompareOp        = VK_COMPARE_OP_LESS;
                state.depthBoundsTestEnable = VK_FALSE;
                state.stencilTestEnable     = depthStencil.stencilEnable ? VK_TRUE : VK_FALSE;
                state.front                 = {};
                state.back                  = {};
                state.minDepthBounds        = 0.0f;
                state.maxDepthBounds        = 1.0f;
            }

            inline void Initalize(VkPipelineDepthStencilStateCreateInfo const*& pState) const
            {
                pState = &state;
            }

            VkPipelineDepthStencilStateCreateInfo state;
        };

        struct ColorBlendState
        {
            ColorBlendState(const GraphicsPipelineColorBlendState& stateDesc)
            {
                state.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                state.pNext             = nullptr;
                state.flags             = 0;
                state.logicOpEnable     = VK_FALSE;
                state.logicOp           = VK_LOGIC_OP_SET;
                state.attachmentCount   = attachmentColorBlend.size();
                state.pAttachments      = attachmentColorBlend.data();
                state.blendConstants[0] = 0.0f;
                state.blendConstants[1] = 0.0f;
                state.blendConstants[2] = 0.0f;
                state.blendConstants[3] = 0.0f;
            }

            inline void Initalize(VkPipelineColorBlendStateCreateInfo const*& pState) const
            {
                pState = &state;
            }

            std::vector<VkPipelineColorBlendAttachmentState> attachmentColorBlend;
            VkPipelineColorBlendStateCreateInfo              state;
        };

        struct DynamicState
        {
            DynamicState()
            {
                VkDynamicState dynamicStates[] = {
                    VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
                    // VK_DYNAMIC_STATE_LINE_WIDTH,
                    // VK_DYNAMIC_STATE_DEPTH_BIAS,
                    // VK_DYNAMIC_STATE_BLEND_CONSTANTS,
                    // VK_DYNAMIC_STATE_DEPTH_BOUNDS,
                    // VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
                    // VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
                    // VK_DYNAMIC_STATE_STENCIL_REFERENCE,
                };
                state.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                state.pNext             = nullptr;
                state.flags             = 0;
                state.dynamicStateCount = 2;
                state.pDynamicStates    = dynamicStates;
            }

            inline void Initalize(VkPipelineDynamicStateCreateInfo const*& pState) const
            {
                pState = &state;
            }

            VkPipelineDynamicStateCreateInfo state;
        };

    } // namespace PipelineStateInitalizers

    PipelineState::~PipelineState()
    {
        vkDestroyPipeline(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult PipelineState::Init(const GraphicsPipelineStateDesc& desc)
    {
        VkGraphicsPipelineCreateInfo createInfo;

        PipelineStateInitalizers::ShaderStage        shaderStageStateInitalizer(desc.shaderStages);
        PipelineStateInitalizers::VertexInputState   vertexInputStateInitalizer(desc.vertexInputAttributes);
        PipelineStateInitalizers::InputAssemblyState inputAssemblyStateInitalizer;
        PipelineStateInitalizers::TessellationState  tessellationStateInitalizer;
        PipelineStateInitalizers::ViewportState      viewportStateInitalizer;
        PipelineStateInitalizers::RasterizationState rasterizationStateInitalizer(desc.rasterizationState);
        PipelineStateInitalizers::MultisampleState   multisampleStateInitalizer(ESampleCount::Count1);
        PipelineStateInitalizers::DepthStencilState  depthStencilStateInitalizer(desc.depthStencil);
        PipelineStateInitalizers::ColorBlendState    colorBlendStateInitalizer(desc.colorBlendState);
        PipelineStateInitalizers::DynamicState       dynamicStateInitalizer;

        shaderStageStateInitalizer.Initalize(createInfo.stageCount, createInfo.pStages);
        vertexInputStateInitalizer.Initalize(createInfo.pVertexInputState);
        inputAssemblyStateInitalizer.Initalize(createInfo.pInputAssemblyState);
        tessellationStateInitalizer.Initalize(createInfo.pTessellationState);
        viewportStateInitalizer.Initalize(createInfo.pViewportState);
        rasterizationStateInitalizer.Initalize(createInfo.pRasterizationState);
        multisampleStateInitalizer.Initalize(createInfo.pMultisampleState);
        depthStencilStateInitalizer.Initalize(createInfo.pDepthStencilState);
        colorBlendStateInitalizer.Initalize(createInfo.pColorBlendState);
        dynamicStateInitalizer.Initalize(createInfo.pDynamicState);

        auto subpass = static_cast<const Pass*>(desc.pPass)->GetRenderPass();
        assert(subpass.has_value());

        // Recreate the entire PipelineStateObject if the RenderPassChanges. 
        
        createInfo.sType              = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        createInfo.pNext              = nullptr;
        createInfo.flags              = 0;
        createInfo.layout             = m_layout->GetHandle();
        createInfo.renderPass         = subpass->first.GetHandle();
        createInfo.subpass            = subpass->second;
        createInfo.basePipelineHandle = VK_NULL_HANDLE;
        createInfo.basePipelineIndex  = 0;

        return vkCreateGraphicsPipelines(m_pDevice->GetHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_handle);
    }
    
} // namespace Vulkan
} // namespace RHI