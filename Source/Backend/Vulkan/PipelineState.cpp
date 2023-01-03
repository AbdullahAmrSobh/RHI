#include "RHI/Pch.hpp"

#include "Backend/Vulkan/Common.hpp"

#include "Backend/Vulkan/PipelineState.hpp"

#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/RenderPass.hpp"
#include "Backend/Vulkan/Resource.hpp"
#include "Backend/Vulkan/ShaderResourceGroup.hpp"

namespace RHI
{
namespace Vulkan
{
PipelineLayout::~PipelineLayout()
{
    vkDestroyPipelineLayout(m_device->GetHandle(), m_handle, nullptr);
}

VkResult PipelineLayout::Init(const PipelineLayoutDesc& layoutDesc)
{
    size_t hash = 0;

    uint32_t offset = 0;
    for (auto& layout : layoutDesc.shaderBindingGroupLayouts)
    {
        hash = hash_combine(hash, layout.GetHash());

        // For other resources

        for (auto& constants : layout.GetShaderConstantBufferBindings())
        {
            offset = static_cast<uint32_t>(constants.byteSize);
            VkPushConstantRange range;
            range.offset = offset;
            range.size   = static_cast<uint32_t>(constants.byteSize);
            m_pushConstantRanges.push_back(range);
        }
    }
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    std::transform(m_descriptorSetsLayouts.begin(),
                   m_descriptorSetsLayouts.end(),
                   std::back_insert_iterator(descriptorSetLayouts),
                   [](const auto& dsl) { return dsl->GetHandle(); });

    VkPipelineLayoutCreateInfo createInfo = {};
    createInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.pNext                      = nullptr;
    createInfo.flags                      = 0;
    createInfo.setLayoutCount             = CountElements(descriptorSetLayouts);
    createInfo.pSetLayouts                = descriptorSetLayouts.data();
    createInfo.pushConstantRangeCount     = CountElements(m_pushConstantRanges);
    createInfo.pPushConstantRanges        = m_pushConstantRanges.data();

    return vkCreatePipelineLayout(m_device->GetHandle(), &createInfo, nullptr, &m_handle);
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
        createInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        createInfo.pNext                           = nullptr;
        createInfo.flags                           = 0;

        if (pVertexShader)
        {
            createInfo.stage               = VK_SHADER_STAGE_VERTEX_BIT;
            createInfo.module              = pVertexShader->GetHandle();
            createInfo.pName               = pVertexShader->m_functionName.c_str();
            createInfo.pSpecializationInfo = nullptr;
            states.push_back(createInfo);
        }

        if (pTessellationControlShader)
        {
            createInfo.stage               = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            createInfo.module              = pTessellationControlShader->GetHandle();
            createInfo.pName               = pTessellationControlShader->m_functionName.c_str();
            createInfo.pSpecializationInfo = nullptr;
            states.push_back(createInfo);
        }

        if (pTessellationEvaluationShader)
        {
            createInfo.stage               = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            createInfo.module              = pTessellationEvaluationShader->GetHandle();
            createInfo.pName               = pTessellationEvaluationShader->m_functionName.c_str();
            createInfo.pSpecializationInfo = nullptr;
            states.push_back(createInfo);
        }

        if (pGeometryShader)
        {
            createInfo.stage               = VK_SHADER_STAGE_GEOMETRY_BIT;
            createInfo.module              = pGeometryShader->GetHandle();
            createInfo.pName               = pGeometryShader->m_functionName.c_str();
            createInfo.pSpecializationInfo = nullptr;
            states.push_back(createInfo);
        }

        if (pFragmentShader)
        {
            createInfo.stage               = VK_SHADER_STAGE_FRAGMENT_BIT;
            createInfo.module              = pFragmentShader->GetHandle();
            createInfo.pName               = pFragmentShader->m_functionName.c_str();
            createInfo.pSpecializationInfo = nullptr;
            states.push_back(createInfo);
        }
    }

    void Initalize(uint32_t& count, VkPipelineShaderStageCreateInfo const*& pState)
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

            bindingDescription.stride += GetFormatByteSize(inattribute.format);

            location++;
            attributes.push_back(attribute);
        }

        bindingDescription.binding   = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        state       = {};
        state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        state.pNext = nullptr;
        state.flags = 0;
        state.vertexBindingDescriptionCount   = 1;
        state.pVertexBindingDescriptions      = &bindingDescription;
        state.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
        state.pVertexAttributeDescriptions    = attributes.data();
    }

    void Initalize(VkPipelineVertexInputStateCreateInfo const*& pState) const
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

    void Initalize(VkPipelineInputAssemblyStateCreateInfo const*& pState) const
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

    void Initalize(VkPipelineTessellationStateCreateInfo const*& pState) const
    {
        pState = &state;
    }

    VkPipelineTessellationStateCreateInfo state;
};

struct ViewportState
{
    ViewportState()
    {
        viewport.x        = 0;
        viewport.y        = 0;
        viewport.width    = 640;
        viewport.height   = 480;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        scissor           = {{0, 0}, {640, 480}};

        state.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        state.pNext         = nullptr;
        state.flags         = 0;
        state.viewportCount = 1;
        state.pViewports    = &viewport;
        state.scissorCount  = 1;
        state.pScissors     = &scissor;
    }

    void Initalize(VkPipelineViewportStateCreateInfo const*& pState) const
    {
        pState = &state;
    }

    VkViewport                        viewport;
    VkRect2D                          scissor;
    VkPipelineViewportStateCreateInfo state;
};

struct RasterizationState
{
    RasterizationState(const GraphicsPipelineRasterizationState& rasterStateDesc)
    {
        state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        state.pNext = nullptr;
        state.flags = 0;

        state.depthClampEnable = VK_FALSE;
        // discards all primitives before the rasterization stage if enabled
        // which we don't want
        state.rasterizerDiscardEnable = VK_FALSE;

        state.polygonMode = VK_POLYGON_MODE_FILL;
        switch (rasterStateDesc.fillMode)
        {
            case RasterizationFillMode::Point: state.polygonMode = VK_POLYGON_MODE_POINT; break;
            case RasterizationFillMode::Triangle: state.polygonMode = VK_POLYGON_MODE_FILL; break;
            case RasterizationFillMode::Line: state.polygonMode = VK_POLYGON_MODE_LINE; break;
        };

        state.lineWidth = 1.0f;
        // no backface cull
        switch (rasterStateDesc.cullMode)
        {
            case RasterizationCullMode::None: state.cullMode = VK_CULL_MODE_NONE; break;
            case RasterizationCullMode::FrontFace: state.cullMode = VK_CULL_MODE_FRONT_BIT; break;
            case RasterizationCullMode::BackFace: state.cullMode = VK_CULL_MODE_BACK_BIT; break;
        };
        state.frontFace = VK_FRONT_FACE_CLOCKWISE;
        // no depth bias
        state.depthBiasEnable         = VK_FALSE;
        state.depthBiasConstantFactor = 0.0f;
        state.depthBiasClamp          = 0.0f;
        state.depthBiasSlopeFactor    = 0.0f;
    }

    void Initalize(VkPipelineRasterizationStateCreateInfo const*& pState) const
    {
        pState = &state;
    }

    VkPipelineRasterizationStateCreateInfo state;
};

struct MultisampleState
{
    MultisampleState(SampleCount sampleCount)
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

    void Initalize(VkPipelineMultisampleStateCreateInfo const*& pState) const
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

    void Initalize(VkPipelineDepthStencilStateCreateInfo const*& pState) const
    {
        pState = &state;
    }

    VkPipelineDepthStencilStateCreateInfo state;
};

struct ColorBlendState
{
    ColorBlendState(const GraphicsPipelineColorBlendState& stateDesc)
    {
        // TODO
        (void)stateDesc;

        VkPipelineColorBlendAttachmentState blendState = {};
        blendState.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendState.blendEnable         = VK_FALSE;
        blendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        blendState.colorBlendOp        = VK_BLEND_OP_ADD;       // Optional
        blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        blendState.alphaBlendOp        = VK_BLEND_OP_ADD;       // Optional

        attachmentColorBlend.push_back(blendState);

        state.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        state.pNext             = nullptr;
        state.flags             = 0;
        state.logicOpEnable     = VK_FALSE;
        state.logicOp           = VK_LOGIC_OP_COPY;
        state.attachmentCount   = CountElements(attachmentColorBlend);
        state.pAttachments      = attachmentColorBlend.data();
        state.blendConstants[0] = 0.0f;
        state.blendConstants[1] = 0.0f;
        state.blendConstants[2] = 0.0f;
        state.blendConstants[3] = 0.0f;
    }

    void Initalize(VkPipelineColorBlendStateCreateInfo const*& pState) const
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
        state.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        state.pNext             = nullptr;
        state.flags             = 0;
        state.dynamicStateCount = 2;
        state.pDynamicStates    = dynamicStates;
    }

    void Initalize(VkPipelineDynamicStateCreateInfo const*& pState) const
    {
        pState = &state;
    }

    VkPipelineDynamicStateCreateInfo state;

    VkDynamicState dynamicStates[2] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        // VK_DYNAMIC_STATE_LINE_WIDTH,
        // VK_DYNAMIC_STATE_DEPTH_BIAS,
        // VK_DYNAMIC_STATE_BLEND_CONSTANTS,
        // VK_DYNAMIC_STATE_DEPTH_BOUNDS,
        // VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
        // VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
        // VK_DYNAMIC_STATE_STENCIL_REFERENCE,
    };
};

}  // namespace PipelineStateInitalizers

Expected<Unique<IPipelineState>> Device::CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc)
{
    Unique<PipelineState> pipelineState = CreateUnique<PipelineState>(*this);
    VkResult              result        = pipelineState->Init(desc);

    if (!Utils::IsSuccess(result))
    {
        return Unexpected(ConvertResult(result));
    }

    return std::move(pipelineState);
}

PipelineState::~PipelineState()
{
    vkDestroyPipeline(m_device->GetHandle(), m_handle, nullptr);
}

VkResult PipelineState::Init(const GraphicsPipelineStateDesc& desc)
{
    m_layout        = CreateUnique<PipelineLayout>(*m_device);
    VkResult result = m_layout->Init(desc.pipelineLayoutDesc);
    VK_RETURN_ON_ERROR(result);

    VkGraphicsPipelineCreateInfo createInfo;
    createInfo.sType              = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext              = nullptr;
    createInfo.flags              = 0;
    createInfo.layout             = m_layout->GetHandle();
    createInfo.renderPass         = static_cast<const RenderPass*>(desc.pRenderPass)->GetLayout().GetHandle();
    createInfo.subpass            = 0;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex  = 0;
    PipelineStateInitalizers::ShaderStage        shaderStageStateInitalizer(desc.shaderStages);
    PipelineStateInitalizers::VertexInputState   vertexInputStateInitalizer(desc.vertexInputAttributes);
    PipelineStateInitalizers::InputAssemblyState inputAssemblyStateInitalizer;
    PipelineStateInitalizers::TessellationState  tessellationStateInitalizer;
    PipelineStateInitalizers::ViewportState      viewportStateInitalizer;
    PipelineStateInitalizers::RasterizationState rasterizationStateInitalizer(desc.rasterizationState);
    PipelineStateInitalizers::MultisampleState   multisampleStateInitalizer(SampleCount::Count1);
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

    // const Pass& pass = *static_cast<const Pass*>(desc.pRenderPass);

    // Recreate the entire PipelineStateObject if the RenderPassChanges.

    return vkCreateGraphicsPipelines(m_device->GetHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_handle);
}

}  // namespace Vulkan
}  // namespace RHI