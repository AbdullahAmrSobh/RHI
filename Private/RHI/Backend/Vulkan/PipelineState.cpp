#include "RHI/Backend/Vulkan/PipelineState.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"
#include "RHI/Backend/Vulkan/PipelineLayout.hpp"
#include "RHI/Backend/Vulkan/RenderGraph.hpp"

namespace RHI
{
namespace Vulkan
{

    Expected<ShaderModulePtr> Factory::CreateShaderModule(const ShaderModuleDesc& desc)
    {
        auto                     shaderModule = CreateUnique<ShaderModule>(*m_device, desc);
        VkShaderModuleCreateInfo createInfo   = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, desc.bytecode.size(),
                                               reinterpret_cast<const uint32_t*>(desc.bytecode.data())};

        VkResult result = shaderModule->Init(createInfo);

        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));

        return shaderModule;
    }

    VkResult ShaderModule::Init(const VkShaderModuleCreateInfo& createInfo)
    {
        return vkCreateShaderModule(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

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
        PipelineStateInitalizers::ShaderStage        shaderStageStateInitalizer(desc.shaders.pVertexShader, nullptr, nullptr, nullptr, desc.shaders.pPixelShader);
        PipelineStateInitalizers::VertexInputState   vertexInputStateInitalizer(desc.vertexAttributes);
        PipelineStateInitalizers::InputAssemblyState inputAssemblyStateInitalizer;
        PipelineStateInitalizers::TessellationState  tessellationStateInitalizer;
        PipelineStateInitalizers::ViewportState      viewportStateInitalizer;
        PipelineStateInitalizers::RasterizationState rasterizationStateInitalizer;
        PipelineStateInitalizers::MultisampleState   multisampleStateInitalizer(desc.multisampleState.sampleCount);
        PipelineStateInitalizers::DepthStencilState  depthStencilStateInitalizer(false);
        PipelineStateInitalizers::ColorBlendState    colorBlendStateInitalizer;
        PipelineStateInitalizers::DynamicState       dynamicStateInitalizer;
        
        RenderPass& renderPass = RenderPass::FindOrCreate(*m_pDevice, desc.renderTargetLayout);
        
        VkGraphicsPipelineCreateInfo createInfo = {};
        createInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        createInfo.pNext                        = nullptr;
        createInfo.flags                        = 0;
        createInfo.layout                       = static_cast<const PipelineLayout*>(desc.pPipelineLayout)->GetHandle();
        createInfo.renderPass                   = renderPass.GetHandle();
        createInfo.subpass                      = 0;
        createInfo.basePipelineHandle           = VK_NULL_HANDLE;
        createInfo.basePipelineIndex            = 0;

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
        createInfo.layout             = static_cast<const PipelineLayout*>(desc.pPipelineLayout)->GetHandle();
        createInfo.basePipelineHandle = VK_NULL_HANDLE;
        createInfo.basePipelineIndex  = 0;

        return vkCreateComputePipelines(m_pDevice->GetHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_handle);
    }

} // namespace Vulkan
} // namespace RHI
