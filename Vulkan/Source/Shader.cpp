#include "Shader.hpp"

#include "Common.hpp"
#include "Device.hpp"

namespace RHI::Vulkan
{
    IShaderModule::IShaderModule()  = default;
    IShaderModule::~IShaderModule() = default;

    ResultCode IShaderModule::Init(IDevice* device, const ShaderModuleCreateInfo& createInfo)
    {
        m_device = device;
        VkShaderModuleCreateInfo shaderModuleCI =
            {
                .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .pNext    = nullptr,
                .flags    = {},
                .codeSize = createInfo.code.size_bytes(),
                .pCode    = createInfo.code.data(),
            };
        auto result = vkCreateShaderModule(device->m_device, &shaderModuleCI, nullptr, &m_shaderModule);
        return ConvertResult(result);
    }

    void IShaderModule::Shutdown()
    {
        vkDestroyShaderModule(m_device->m_device, m_shaderModule, nullptr);
    }
} // namespace RHI::Vulkan