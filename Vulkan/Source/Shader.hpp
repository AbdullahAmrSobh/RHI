#pragma once

#include <RHI/Shader.hpp>
#include <RHI/Result.hpp>

#include <TL/Span.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;

    class IShaderModule final : public ShaderModule
    {
    public:
        IShaderModule(IContext* context)
            : m_context(context)
        {
        }

        ~IShaderModule();

        ResultCode Init(TL::Span<const uint32_t> shaderBlob);

    public:
        IContext* m_context;
        VkShaderModule m_shaderModule;
    };

} // namespace RHI
