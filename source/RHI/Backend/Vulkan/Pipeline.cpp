#pragma once

#include "RHI/Backend/Vulkan/Pipeline.hpp"

#include "RHI/Backend/Vulkan/Context.hpp"
#include "RHI/Backend/Vulkan/Conversion.inl"

namespace Vulkan
{

vk::ShaderStageFlagBits ConvertShaderStage(RHI::ShaderType shader)
{
    return {};
}

vk::PolygonMode GetPolygonMode(RHI::PolygonMode mode)
{
    return {};
}

vk::CullModeFlags GetCullMode(RHI::CullMode cullMode)
{
}

vk::FrontFace GetFrontFace(RHI::FrontFace frontFace)
{
}

struct GraphicsStateInitalizer
{
    GraphicsStateInitalizer(const RHI::GraphicsPipelineCreateInfo& createInfo)
    {
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

        inputAssemblyStateInfo = vk::PipelineInputAssemblyStateCreateInfo {};
        inputAssemblyStateInfo.setTopology(vk::PrimitiveTopology::eTriangleList);

        tessellationStateInfo = vk::PipelineTessellationStateCreateInfo {};

        viewportStateInfo = vk::PipelineViewportStateCreateInfo {};

        {
            rasterizationStateInfo = vk::PipelineRasterizationStateCreateInfo {};
            rasterizationStateInfo.setDepthClampEnable(createInfo.rasterizationState.depthClampEnable ? VK_TRUE : VK_FALSE);
            rasterizationStateInfo.setRasterizerDiscardEnable(createInfo.rasterizationState.rasterizerDiscardEnable ? VK_TRUE : VK_FALSE);
            rasterizationStateInfo.setPolygonMode(GetPolygonMode(createInfo.rasterizationState.polygonMode));
            rasterizationStateInfo.setLineWidth(createInfo.rasterizationState.lineWidth);
            rasterizationStateInfo.setCullMode(GetCullMode(createInfo.rasterizationState.cullMode));
            rasterizationStateInfo.setFrontFace(GetFrontFace(createInfo.rasterizationState.frontFace));
        }

        {
            multisampleStateInfo.setSampleShadingEnable(VK_FALSE);
            multisampleStateInfo.setRasterizationSamples(
                static_cast<vk::SampleCountFlagBits>(static_cast<uint32_t>(createInfo.multisampleState.sampleCount)));
        }

        depthStencilStateInfo = vk::PipelineDepthStencilStateCreateInfo {};
        depthStencilStateInfo.setDepthTestEnable(createInfo.depthStencilState.depthEnable ? VK_TRUE : VK_FALSE);
        depthStencilStateInfo.setDepthWriteEnable(createInfo.depthStencilState.depthEnable ? VK_TRUE : VK_FALSE);
        depthStencilStateInfo.setStencilTestEnable(createInfo.depthStencilState.stencilEnable ? VK_TRUE : VK_FALSE);
        depthStencilStateInfo.setDepthCompareOp(vk::CompareOp::eGreater);
        depthStencilStateInfo.setMinDepthBounds(createInfo.depthStencilState.minDepthBound);
        depthStencilStateInfo.setMaxDepthBounds(createInfo.depthStencilState.maxDepthBound);

        colorBlendStateInfo = vk::PipelineColorBlendStateCreateInfo {};

        std::vector<vk::Format> colorAttachmentFormats;
        for (auto format : createInfo.renderTargetLayout.colorAttachments)
        {
            colorAttachmentFormats.push_back(ConvertFormat(format));
        }

        renderingInfo.setColorAttachmentFormats(colorAttachmentFormats);
        renderingInfo.setDepthAttachmentFormat(ConvertFormat(createInfo.renderTargetLayout.depthStencilAttachment));

        dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eViewportWithCount};
        dynamicStateInfo.setDynamicStates(dynamicStates);
    }

    std::vector<vk::VertexInputBindingDescription>   bindings;
    std::vector<vk::VertexInputAttributeDescription> attributes;
    vk::PipelineVertexInputStateCreateInfo           vertexInputStateInfo;

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo;

    vk::PipelineTessellationStateCreateInfo tessellationStateInfo;

    vk::PipelineViewportStateCreateInfo viewportStateInfo;

    vk::PipelineRasterizationStateCreateInfo rasterizationStateInfo;

    vk::PipelineMultisampleStateCreateInfo multisampleStateInfo;

    vk::PipelineDepthStencilStateCreateInfo depthStencilStateInfo;

    vk::PipelineColorBlendStateCreateInfo colorBlendStateInfo;

    std::vector<vk::DynamicState>      dynamicStates;
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo;

    vk::PipelineRenderingCreateInfo renderingInfo;
};

struct ComputePipelineCreateInfoInitalizer
{
    ComputePipelineCreateInfoInitalizer(const RHI::ComputePipelineCreateInfo& createInfo);
};

struct RayTracingPipelineCreateInfoInitalizer
{
    RayTracingPipelineCreateInfoInitalizer(const RHI::RayTracingPipelineCreateInfo& createInfo);
};

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

    vk::ShaderModuleCreateInfo                     moduleCreateInfo {};
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    if (auto shader = createInfo.shaders.vertex; shader != nullptr)
    {
        vertexShaderModule = device.createShaderModuleUnique(moduleCreateInfo).value;
        moduleCreateInfo.setCode(createInfo.shaders.pixel->m_code);
        vk::PipelineShaderStageCreateInfo shaderStage {};
        shaderStage.setPName(shader->m_name.c_str());
        shaderStage.setStage(ConvertShaderStage(shader->m_shaderType));
        shaderStage.setModule(vertexShaderModule.get());
        shaderStages.push_back(shaderStage);
    }

    if (auto shader = createInfo.shaders.tessControl; shader != nullptr)
    {
        tessCtrlShaderModule = device.createShaderModuleUnique(moduleCreateInfo).value;
        moduleCreateInfo.setCode(createInfo.shaders.tessControl->m_code);
        vk::PipelineShaderStageCreateInfo shaderStage {};
        shaderStage.setPName(shader->m_name.c_str());
        shaderStage.setStage(ConvertShaderStage(shader->m_shaderType));
        shaderStage.setModule(tessCtrlShaderModule.get());
        shaderStages.push_back(shaderStage);
    }

    if (auto shader = createInfo.shaders.tessEval; shader != nullptr)
    {
        tessEvalShaderModule = device.createShaderModuleUnique(moduleCreateInfo).value;
        moduleCreateInfo.setCode(createInfo.shaders.tessEval->m_code);
        vk::PipelineShaderStageCreateInfo shaderStage {};
        shaderStage.setPName(shader->m_name.c_str());
        shaderStage.setStage(ConvertShaderStage(shader->m_shaderType));
        shaderStage.setModule(tessEvalShaderModule.get());
        shaderStages.push_back(shaderStage);
    }

    if (auto shader = createInfo.shaders.pixel; shader != nullptr)
    {
        pixelShaderModule = device.createShaderModuleUnique(moduleCreateInfo).value;
        moduleCreateInfo.setCode(createInfo.shaders.pixel->m_code);
        vk::PipelineShaderStageCreateInfo shaderStage {};
        shaderStage.setPName(shader->m_name.c_str());
        shaderStage.setStage(ConvertShaderStage(shader->m_shaderType));
        shaderStage.setModule(pixelShaderModule.get());
        shaderStages.push_back(shaderStage);
    }

    vk::GraphicsPipelineCreateInfo pipelineStateInfo {};
    pipelineStateInfo.setStages(shaderStages);

    GraphicsStateInitalizer initalizer {createInfo};
    pipelineStateInfo.setPVertexInputState(&initalizer.vertexInputStateInfo);
    pipelineStateInfo.setPInputAssemblyState(&initalizer.inputAssemblyStateInfo);
    pipelineStateInfo.setPTessellationState(&initalizer.tessellationStateInfo);
    pipelineStateInfo.setPViewportState(&initalizer.viewportStateInfo);
    pipelineStateInfo.setPRasterizationState(&initalizer.rasterizationStateInfo);
    pipelineStateInfo.setPMultisampleState(&initalizer.multisampleStateInfo);
    pipelineStateInfo.setPDepthStencilState(&initalizer.depthStencilStateInfo);
    pipelineStateInfo.setPColorBlendState(&initalizer.colorBlendStateInfo);
    pipelineStateInfo.setPDynamicState(&initalizer.dynamicStateInfo);
    pipelineStateInfo.setPNext(&initalizer.renderingInfo);
    pipelineStateInfo.setLayout(m_layout->get());

    m_handle = device.createGraphicsPipeline({}, pipelineStateInfo).value;

    return RHI::ResultCode::Success;
}

RHI::ResultCode PipelineState::Init(const RHI::ComputePipelineCreateInfo& createInfo)
{
    Context&   context = static_cast<Context&>(*m_context);
    vk::Device device  = context.GetDevice();

    ComputePipelineCreateInfoInitalizer initalizer {createInfo};

    vk::ComputePipelineCreateInfo pipelineStateInfo {};

    m_handle = device.createComputePipeline(VK_NULL_HANDLE, pipelineStateInfo).value;

    return RHI::ResultCode::Success;
}

RHI::ResultCode PipelineState::Init(const RHI::RayTracingPipelineCreateInfo& createInfo)
{
    Context&   context = static_cast<Context&>(*m_context);
    vk::Device device  = context.GetDevice();

    RayTracingPipelineCreateInfoInitalizer initalizer {createInfo};

    vk::RayTracingPipelineCreateInfoKHR pipelineStateInfo {};

    m_handle = device.createRayTracingPipelineKHR({}, VK_NULL_HANDLE, pipelineStateInfo).value;

    return RHI::ResultCode::Success;
}

}  // namespace Vulkan