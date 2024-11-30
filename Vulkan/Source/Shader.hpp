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

        ResultCode Init(IDevice* device, TL::Span<const uint32_t> shaderBlob);
        void       Shutdown();

    public:
        IDevice*       m_device;
        VkShaderModule m_shaderModule;
    };

} // namespace RHI::Vulkan
