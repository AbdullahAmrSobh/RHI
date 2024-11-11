#include "Shader.hpp"
#include "Device.hpp"
#include "Common.hpp"

namespace RHI::Vulkan
{
    IShaderModule::IShaderModule() = default;

    IShaderModule::~IShaderModule() = default;

    ResultCode IShaderModule::Init(IDevice* device, TL::Span<const uint32_t> shaderBlob)
    {
        m_device = device;

        VkShaderModuleCreateInfo createInfo{
            .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext    = nullptr,
            .flags    = {},
            .codeSize = shaderBlob.size_bytes(),
            .pCode    = shaderBlob.data(),
        };

        auto result = vkCreateShaderModule(device->m_device, &createInfo, nullptr, &m_shaderModule);
        return ConvertResult(result);
    }

    void IShaderModule::Shutdown()
    {
        vkDestroyShaderModule(m_device->m_device, m_shaderModule, nullptr);
    }

} // namespace RHI::Vulkan