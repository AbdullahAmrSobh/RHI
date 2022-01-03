#include "RHI/Backend/Vulkan/PipelineState.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"
#include "RHI/Backend/Vulkan/PipelineLayout.hpp"
#include "RHI/Backend/Vulkan/RenderPass.hpp"

namespace RHI
{
namespace Vulkan
{
    Expected<PipelineStatePtr> Factory::CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc)
    {
        auto     pipelineState = CreateUnique<PipelineState>(*m_device);
        VkResult result        = pipelineState->Init(desc);
        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));

        return pipelineState;
    }

    Expected<PipelineStatePtr> Factory::CreateComputePipelineState(const ComputePipelineStateDesc& desc)
    {
        auto     pipelineState = CreateUnique<PipelineState>(*m_device);
        VkResult result        = pipelineState->Init(desc);
        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));

        return pipelineState;
    }

    PipelineState::~PipelineState() { vkDestroyPipeline(m_pDevice->GetHandle(), m_handle, nullptr); }

    VkResult PipelineState::Init(const GraphicsPipelineStateDesc& desc)
    {
        uint32_t stagesCount = (desc.pVertexShader != nullptr) + (desc.pPixelShader != nullptr) + (desc.pDomainShader != nullptr) +
                               (desc.pHullShader != nullptr) + (desc.pGeometryShader != nullptr);

        std::vector<VkPipelineShaderStageCreateInfo> stages(stagesCount);
        {
            for (auto& stage : stages)
            {
                stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                stage.pNext = nullptr;
                stage.flags = 0;
                stage.stage;
                stage.module;
                stage.pName;
                stage.pSpecializationInfo = nullptr;
            }
        }

        VkPipelineVertexInputStateCreateInfo vertexInputState;
        {
            VkVertexInputBindingDescription                bindingDesc;
            std::vector<VkVertexInputAttributeDescription> attributeDescs;

            vertexInputState.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputState.pNext                           = nullptr;
            vertexInputState.flags                           = 0;
            vertexInputState.vertexBindingDescriptionCount   = 1;
            vertexInputState.pVertexBindingDescriptions      = &bindingDesc;
            vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(desc.vertexBufferLayout.attributes.size());
            vertexInputState.pVertexAttributeDescriptions    = attributeDescs.data();
        }

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
        {
            inputAssemblyState.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssemblyState.pNext                  = nullptr;
            inputAssemblyState.flags                  = 0;
            inputAssemblyState.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssemblyState.primitiveRestartEnable = VK_FALSE;
        }

        VkPipelineTessellationStateCreateInfo tessellationState;
        {
            if (desc.pDomainShader == nullptr)
            {
                tessellationState.sType              = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
                tessellationState.pNext              = nullptr;
                tessellationState.flags              = 0;
                tessellationState.patchControlPoints = 0;
            }
            else
            {
                assert(false);
            }
        }

        VkPipelineViewportStateCreateInfo viewportState;
        {
            viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.pNext         = nullptr;
            viewportState.flags         = 0;
            viewportState.viewportCount = 0;
            viewportState.pViewports    = nullptr;
            viewportState.scissorCount  = 0;
            viewportState.pScissors     = nullptr;
        }

        VkPipelineRasterizationStateCreateInfo rasterizationState;
        {
            rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizationState.pNext = nullptr;
            rasterizationState.flags = 0;
            rasterizationState.depthClampEnable;
            rasterizationState.rasterizerDiscardEnable;
            rasterizationState.polygonMode;
            rasterizationState.cullMode;
            rasterizationState.frontFace;
            rasterizationState.depthBiasEnable;
            rasterizationState.depthBiasConstantFactor;
            rasterizationState.depthBiasClamp;
            rasterizationState.depthBiasSlopeFactor;
            rasterizationState.lineWidth;
        }

        VkPipelineMultisampleStateCreateInfo multisampleState;
        {
            multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampleState.pNext = nullptr;
            multisampleState.flags = 0;
            multisampleState.rasterizationSamples;
            multisampleState.sampleShadingEnable;
            multisampleState.minSampleShading;
            multisampleState.pSampleMask;
            multisampleState.alphaToCoverageEnable;
            multisampleState.alphaToOneEnable;
        }

        VkPipelineDepthStencilStateCreateInfo depthStencilState;
        {
            depthStencilState.sType           = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencilState.pNext           = nullptr;
            depthStencilState.flags           = 0;
            depthStencilState.depthTestEnable = desc.depthStencilState.depthEnabled ? VK_TRUE : VK_FALSE;
            depthStencilState.depthWriteEnable;
            depthStencilState.depthCompareOp;
            depthStencilState.depthBoundsTestEnable;
            depthStencilState.stencilTestEnable = desc.depthStencilState.stencilEnable ? VK_TRUE : VK_FALSE;
            depthStencilState.front;
            depthStencilState.back;
            depthStencilState.minDepthBounds;
            depthStencilState.maxDepthBounds;
        }

        VkPipelineColorBlendStateCreateInfo colorBlendState;
        {
            std::vector<VkPipelineColorBlendAttachmentState> attachmentColorBlend;
            colorBlendState.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlendState.pNext             = nullptr;
            colorBlendState.flags             = 0;
            colorBlendState.logicOpEnable     = VK_FALSE;
            colorBlendState.logicOp           = VK_LOGIC_OP_SET;
            colorBlendState.attachmentCount   = attachmentColorBlend.size();
            colorBlendState.pAttachments      = attachmentColorBlend.data();
            colorBlendState.blendConstants[0] = 0.0f;
            colorBlendState.blendConstants[1] = 0.0f;
            colorBlendState.blendConstants[2] = 0.0f;
            colorBlendState.blendConstants[3] = 0.0f;
        }

        VkPipelineDynamicStateCreateInfo dynamicState;
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
            dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.pNext             = nullptr;
            dynamicState.flags             = 0;
            dynamicState.dynamicStateCount = 2;
            dynamicState.pDynamicStates    = dynamicStates;
        }

        RenderPass* pRenderPass;

        VkGraphicsPipelineCreateInfo createInfo = {};
        createInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        createInfo.pNext                        = nullptr;
        createInfo.flags                        = 0;
        createInfo.stageCount                   = stagesCount;
        createInfo.pStages                      = stages.data();
        createInfo.pVertexInputState            = &vertexInputState;
        createInfo.pInputAssemblyState          = &inputAssemblyState;
        createInfo.pTessellationState           = &tessellationState;
        createInfo.pViewportState               = &viewportState;
        createInfo.pRasterizationState          = &rasterizationState;
        createInfo.pMultisampleState            = &multisampleState;
        createInfo.pDepthStencilState           = &depthStencilState;
        createInfo.pColorBlendState             = &colorBlendState;
        createInfo.pDynamicState                = &dynamicState;
        createInfo.layout                       = static_cast<PipelineLayout*>(desc.pPipelineLayout)->GetHandle();
        createInfo.renderPass                   = pRenderPass->GetHandle();
        createInfo.subpass                      = 0;
        createInfo.basePipelineHandle           = VK_NULL_HANDLE;
        createInfo.basePipelineIndex            = 0;

        return vkCreateGraphicsPipelines(m_pDevice->GetHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_handle);
    }

    VkResult PipelineState::Init(const ComputePipelineStateDesc& desc)
    {
        VkComputePipelineCreateInfo createInfo = {};

        VkPipelineShaderStageCreateInfo stage;

        createInfo.sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        createInfo.pNext              = nullptr;
        createInfo.flags              = 0;
        createInfo.stage              = stage;
        createInfo.layout             = static_cast<PipelineLayout*>(desc.pPipelineLayout)->GetHandle();
        createInfo.basePipelineHandle = VK_NULL_HANDLE;
        createInfo.basePipelineIndex  = 0;

        return vkCreateComputePipelines(m_pDevice->GetHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_handle);
    }

} // namespace Vulkan
} // namespace RHI
