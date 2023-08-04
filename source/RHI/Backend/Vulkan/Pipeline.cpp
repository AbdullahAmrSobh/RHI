#pragma once

#include "RHI/Backend/Vulkan/Pipeline.hpp"

#include "RHI/Backend/Vulkan/Context.hpp"
#include "RHI/Backend/Vulkan/Conversion.inl"

namespace Vulkan
{

vk::ShaderStageFlagBits ConvertShaderStage(RHI::ShaderStage shader)
{
    switch (shader)
    {
        case RHI::ShaderStage::Vertex: return vk::ShaderStageFlagBits::eVertex;
        case RHI::ShaderStage::TessellationControl: return vk::ShaderStageFlagBits::eTessellationControl;
        case RHI::ShaderStage::TessellationEvaluation: return vk::ShaderStageFlagBits::eTessellationEvaluation;
        case RHI::ShaderStage::Pixel: return vk::ShaderStageFlagBits::eFragment;
        default: RHI_ASSERT_MSG(false, "invalid enum case");
    }
    return {};
}

vk::PolygonMode GetPolygonMode(RHI::PolygonMode mode)
{
    switch (mode)
    {
        case RHI::PolygonMode::Fill: return vk::PolygonMode::eFill;
        case RHI::PolygonMode::Line: return vk::PolygonMode::eLine;
        case RHI::PolygonMode::Point: return vk::PolygonMode::ePoint;
        default: RHI_ASSERT_MSG(false, "invalid enum case");
    }
    return {};
}

vk::CullModeFlags GetCullMode(RHI::CullMode cullMode)
{
    switch (cullMode)
    {
        case RHI::CullMode::None: return vk::CullModeFlagBits::eNone;
        case RHI::CullMode::Back: return vk::CullModeFlagBits::eBack;
        case RHI::CullMode::Front: return vk::CullModeFlagBits::eFront;
        case RHI::CullMode::Both: return vk::CullModeFlagBits::eFront | vk::CullModeFlagBits::eBack;
        default: RHI_ASSERT_MSG(false, "invalid enum case");
    }
    return {};
}

vk::FrontFace GetFrontFace(RHI::FrontFace frontFace)
{
    switch (frontFace)
    {
        case RHI::FrontFace::Clockwise: return vk::FrontFace::eClockwise;
        case RHI::FrontFace::CounterClockwise: return vk::FrontFace::eCounterClockwise;
        default: RHI_ASSERT_MSG(false, "invalid enum case");
    }
    return {};
}

PipelineState::~PipelineState()
{
    Context&   context = static_cast<Context&>(*m_context);
    vk::Device device  = context.GetDevice();
}

RHI::ResultCode PipelineState::Init(const RHI::GraphicsPipelineCreateInfo& createInfo)
{
    Context&   context = static_cast<Context&>(*m_context);
    vk::Device device  = context.GetDevice();

    m_layout = context.CreatePipelineLayout(createInfo.shadersLayout);

    vk::UniqueShaderModule vertexShaderModule;
    vk::UniqueShaderModule tessCtrlShaderModule;
    vk::UniqueShaderModule tessEvalShaderModule;
    vk::UniqueShaderModule pixelShaderModule;

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    if (auto shader = createInfo.shaders.vertex; shader != nullptr)
    {
        vertexShaderModule = context.CreateModule(shader->m_code);

        vk::PipelineShaderStageCreateInfo shaderStage {};
        shaderStage.setPName(shader->m_name.c_str());
        shaderStage.setStage(ConvertShaderStage(shader->m_shaderType));
        shaderStage.setModule(vertexShaderModule.get());
        shaderStages.push_back(shaderStage);
    }

    if (auto shader = createInfo.shaders.tessControl; shader != nullptr)
    {
        tessCtrlShaderModule = context.CreateModule(shader->m_code);

        vk::PipelineShaderStageCreateInfo shaderStage {};
        shaderStage.setPName(shader->m_name.c_str());
        shaderStage.setStage(ConvertShaderStage(shader->m_shaderType));
        shaderStage.setModule(tessCtrlShaderModule.get());
        shaderStages.push_back(shaderStage);
    }

    if (auto shader = createInfo.shaders.tessEval; shader != nullptr)
    {
        tessEvalShaderModule = context.CreateModule(shader->m_code);

        vk::PipelineShaderStageCreateInfo shaderStage {};
        shaderStage.setPName(shader->m_name.c_str());
        shaderStage.setStage(ConvertShaderStage(shader->m_shaderType));
        shaderStage.setModule(tessEvalShaderModule.get());
        shaderStages.push_back(shaderStage);
    }

    if (auto shader = createInfo.shaders.pixel; shader != nullptr)
    {
        pixelShaderModule = context.CreateModule(shader->m_code);

        vk::PipelineShaderStageCreateInfo shaderStage {};
        shaderStage.setPName(shader->m_name.c_str());
        shaderStage.setStage(ConvertShaderStage(shader->m_shaderType));
        shaderStage.setModule(pixelShaderModule.get());
        shaderStages.push_back(shaderStage);
    }

    std::vector<vk::VertexInputBindingDescription>   bindings;
    std::vector<vk::VertexInputAttributeDescription> attributes;
    vk::PipelineVertexInputStateCreateInfo           vertexInputStateInfo {};
    // vertex input state
    {
        // per vertex
        uint32_t location = 0;
        uint32_t offset   = 0;
        for (auto format : createInfo.vertexInputLayout.vertexAttributes)
        {
            vk::VertexInputAttributeDescription attribute {};
            attribute.setBinding(0);
            attribute.setLocation(location);
            attribute.setFormat(ConvertFormat(format));
            attribute.setOffset(offset);
            offset += GetFormatByteSize(format);
            location++;
            attributes.push_back(attribute);
        }

        vk::VertexInputBindingDescription binding = {};
        binding.setBinding(0);
        binding.setInputRate(vk::VertexInputRate::eVertex);
        binding.setStride(offset);
        bindings.push_back(binding);
    }

    {
        // per instance
        uint32_t location = 0;
        uint32_t offset   = 0;
        for (auto format : createInfo.vertexInputLayout.instanceAttributes)
        {
            vk::VertexInputAttributeDescription attribute {};
            attribute.setBinding(0);
            attribute.setLocation(location);
            attribute.setFormat(ConvertFormat(format));
            attribute.setOffset(offset);
            offset += GetFormatByteSize(format);
            location++;
            attributes.push_back(attribute);
        }
        vk::VertexInputBindingDescription binding = {};
        binding.setBinding(1);
        binding.setInputRate(vk::VertexInputRate::eInstance);
        binding.setStride(offset);
        bindings.push_back(binding);
    }
    vertexInputStateInfo.setVertexBindingDescriptions(bindings);
    vertexInputStateInfo.setVertexAttributeDescriptions(attributes);

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo {};
    inputAssemblyStateInfo.setTopology(vk::PrimitiveTopology::eTriangleList);

    vk::PipelineTessellationStateCreateInfo tessellationStateInfo {};
    vk::PipelineViewportStateCreateInfo     viewportStateInfo {};

    vk::PipelineRasterizationStateCreateInfo rasterizationStateInfo {};
    rasterizationStateInfo.setDepthClampEnable(createInfo.rasterizationState.depthClampEnable ? VK_TRUE : VK_FALSE);
    rasterizationStateInfo.setRasterizerDiscardEnable(createInfo.rasterizationState.rasterizerDiscardEnable ? VK_TRUE : VK_FALSE);
    rasterizationStateInfo.setPolygonMode(GetPolygonMode(createInfo.rasterizationState.polygonMode));
    rasterizationStateInfo.setLineWidth(createInfo.rasterizationState.lineWidth);
    rasterizationStateInfo.setCullMode(GetCullMode(createInfo.rasterizationState.cullMode));
    rasterizationStateInfo.setFrontFace(GetFrontFace(createInfo.rasterizationState.frontFace));

    vk::PipelineMultisampleStateCreateInfo multisampleStateInfo {};
    multisampleStateInfo.setSampleShadingEnable(VK_FALSE);
    multisampleStateInfo.setRasterizationSamples(
        static_cast<vk::SampleCountFlagBits>(static_cast<uint32_t>(createInfo.multisampleState.sampleCount)));

    vk::PipelineDepthStencilStateCreateInfo depthStencilStateInfo {};
    depthStencilStateInfo.setDepthTestEnable(createInfo.depthStencilState.depthEnable ? VK_TRUE : VK_FALSE);
    depthStencilStateInfo.setDepthWriteEnable(createInfo.depthStencilState.depthEnable ? VK_TRUE : VK_FALSE);
    depthStencilStateInfo.setStencilTestEnable(createInfo.depthStencilState.stencilEnable ? VK_TRUE : VK_FALSE);
    depthStencilStateInfo.setDepthCompareOp(vk::CompareOp::eGreater);
    depthStencilStateInfo.setMinDepthBounds(createInfo.depthStencilState.minDepthBound);
    depthStencilStateInfo.setMaxDepthBounds(createInfo.depthStencilState.maxDepthBound);

    vk::PipelineColorBlendStateCreateInfo colorBlendStateInfo {};

    std::vector<vk::DynamicState>      dynamicStates {vk::DynamicState::eViewportWithCount, vk::DynamicState::eScissorWithCount};
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo {};
    dynamicStateInfo.setDynamicStates(dynamicStates);

    std::vector<vk::Format>         colorAttachmentFormats;
    vk::PipelineRenderingCreateInfo renderingInfo {};
    for (auto format : createInfo.renderTargetLayout.colorAttachments)
    {
        colorAttachmentFormats.push_back(ConvertFormat(format));
    }

    renderingInfo.setColorAttachmentFormats(colorAttachmentFormats);
    if (createInfo.depthStencilState.depthEnable)
    {
        renderingInfo.setDepthAttachmentFormat(ConvertFormat(createInfo.renderTargetLayout.depthStencilAttachment));
    }

    vk::GraphicsPipelineCreateInfo pipelineStateInfo {};
    pipelineStateInfo.setPNext(&renderingInfo);
    pipelineStateInfo.setStages(shaderStages);
    pipelineStateInfo.setPVertexInputState(&vertexInputStateInfo);
    pipelineStateInfo.setPInputAssemblyState(&inputAssemblyStateInfo);
    pipelineStateInfo.setPTessellationState(&tessellationStateInfo);
    pipelineStateInfo.setPViewportState(&viewportStateInfo);
    pipelineStateInfo.setPRasterizationState(&rasterizationStateInfo);
    pipelineStateInfo.setPMultisampleState(&multisampleStateInfo);
    pipelineStateInfo.setPDepthStencilState(&depthStencilStateInfo);
    pipelineStateInfo.setPColorBlendState(&colorBlendStateInfo);
    pipelineStateInfo.setPDynamicState(&dynamicStateInfo);
    pipelineStateInfo.setLayout(m_layout->get());

    m_handle = device.createGraphicsPipeline({}, pipelineStateInfo).value;

    m_bindPoint = vk::PipelineBindPoint::eGraphics;

    return RHI::ResultCode::Success;
}

}  // namespace Vulkan