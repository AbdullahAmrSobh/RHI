#pragma once

#include <RHI/Pipeline.hpp>
#include <RHI/Result.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;

    VkShaderStageFlags ConvertShaderStage(TL::Flags<ShaderStage> shaderStageFlags);

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

    class IShaderModule final : public ShaderModule
    {
    public:
        IShaderModule();
        ~IShaderModule();

        ResultCode Init(IDevice* device, const ShaderModuleCreateInfo& createInfo);
        void       Shutdown();

    public:
        IDevice*       m_device;
        VkShaderModule m_shaderModule;
    };
} // namespace RHI::Vulkan