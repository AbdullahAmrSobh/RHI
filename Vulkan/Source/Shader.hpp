#pragma once

#include <RHI/Result.hpp>
#include <RHI/Shader.hpp>

#include <vulkan/vulkan.h>

#include <TL/Span.hpp>

namespace RHI::Vulkan
{
    class IDevice;

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
