#pragma once

#include <RHI/Shader.hpp>
#include <RHI/Result.hpp>

#include <TL/Span.hpp>

#include <vulkan/vulkan.h>

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
