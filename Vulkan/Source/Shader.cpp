#include "Shader.hpp"
#include "Context.hpp"
#include "Common.hpp"

namespace RHI::Vulkan
{
    IShaderModule::~IShaderModule()
    {
        vkDestroyShaderModule(((IContext*)m_context)->m_device, m_shaderModule, nullptr);
    }

    ResultCode IShaderModule::Init(TL::Span<const uint32_t> shaderBlob)
    {
        auto context = static_cast<IContext*>(m_context);

        VkShaderModuleCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .codeSize = shaderBlob.size_bytes(),
            .pCode = shaderBlob.data()
        };

        return ConvertResult(vkCreateShaderModule(context->m_device, &createInfo, nullptr, &m_shaderModule));
    }
} // namespace RHI::Vulkan