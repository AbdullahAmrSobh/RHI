#include "Shader.hpp"
#include "Device.hpp"
#include "Common.hpp"

namespace RHI::Vulkan
{
    IShaderModule::~IShaderModule()
    {
        vkDestroyShaderModule(((IDevice*)m_device)->m_device, m_shaderModule, nullptr);
    }

    ResultCode IShaderModule::Init(TL::Span<const uint32_t> shaderBlob)
    {
        auto device = static_cast<IDevice*>(m_device);

        VkShaderModuleCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .codeSize = shaderBlob.size_bytes(),
            .pCode = shaderBlob.data()
        };

        return ConvertResult(vkCreateShaderModule(device->m_device, &createInfo, nullptr, &m_shaderModule));
    }
} // namespace RHI::Vulkan