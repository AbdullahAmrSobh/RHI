#pragma once

#include <RHI/Pipeline.hpp>
#include <RHI/Result.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;

    VkShaderStageFlagBits ConvertShaderStage(ShaderStage shaderStage);

    VkShaderStageFlags ConvertShaderStage(TL::Flags<ShaderStage> shaderStageFlags);

    VkVertexInputRate ConvertVertexInputRate(PipelineVertexInputRate inputRate);

    VkCullModeFlags ConvertCullModeFlags(PipelineRasterizerStateCullMode cullMode);

    VkPolygonMode ConvertPolygonMode(PipelineRasterizerStateFillMode fillMode);

    VkPrimitiveTopology ConvertPrimitiveTopology(PipelineTopologyMode topologyMode);

    VkFrontFace ConvertFrontFace(PipelineRasterizerStateFrontFace frontFace);

    VkCompareOp ConvertCompareOp(CompareOperator compareOperator);

    VkBlendFactor ConvertBlendFactor(BlendFactor blendFactor);

    VkBlendOp ConvertBlendOp(BlendEquation blendEquation);

    struct IPipelineLayout : PipelineLayout
    {
        VkPipelineLayout handle;

        ResultCode Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IGraphicsPipeline : GraphicsPipeline
    {
        VkPipeline       handle;
        VkPipelineLayout layout;

        ResultCode Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IComputePipeline : ComputePipeline
    {
        VkPipeline       handle;
        VkPipelineLayout layout;

        ResultCode Init(IDevice* device, const ComputePipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

} // namespace RHI::Vulkan