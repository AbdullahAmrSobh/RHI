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
        IShaderModule(IDevice* device)
            : m_device(device)
        {
        }

        ~IShaderModule();

        ResultCode Init(TL::Span<const uint32_t> shaderBlob);

    public:
        IDevice*       m_device;
        VkShaderModule m_shaderModule;
    };

} // namespace RHI::Vulkan
