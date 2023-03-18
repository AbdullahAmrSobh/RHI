#include "RHI/Pch.hpp"

#include "Backend/Vulkan/Common.hpp"

#include "Backend/Vulkan/Resource.hpp"

#include "Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{

Expected<std::unique_ptr<IShaderProgram>> Device::CreateShaderProgram(const ShaderProgramDesc& desc)
{
    std::unique_ptr<ShaderModule> shaderModule = std::make_unique<ShaderModule>(*this, desc.entryName);
    VkResult             result       = shaderModule->Init(desc);

    if (Utils::IsSuccess(result))
        return std::move(shaderModule);

    return Unexpected(ConvertResult(result));
}

Expected<std::unique_ptr<IFence>> Device::CreateFence()
{
    std::unique_ptr<Fence> fence  = std::make_unique<Fence>(*this);
    VkResult      result = fence->Init();

    if (Utils::IsSuccess(result))
        return std::move(fence);

    return Unexpected(ConvertResult(result));
}

Expected<std::unique_ptr<ISampler>> Device::CreateSampler(const SamplerDesc& desc)
{
    std::unique_ptr<Sampler> sampler = std::make_unique<Sampler>(*this);
    VkResult        result  = sampler->Init(desc);

    if (Utils::IsSuccess(result))
        return std::move(sampler);

    return Unexpected(ConvertResult(result));
}

VkResult UploadResourceData(Device& device, VmaAllocation allocation, size_t byteOffset, const uint8_t* bufferData, size_t bufferDataByteSize)
{
    uint8_t* data   = nullptr;
    VkResult result = vmaMapMemory(device.GetAllocator(), allocation, reinterpret_cast<void**>(&data));
    if (Utils::IsError(result))
    {
        return result;
    }

    std::memcpy(data + byteOffset, bufferData, bufferDataByteSize);

    vmaUnmapMemory(device.GetAllocator(), allocation);

    return VK_SUCCESS;
}

ShaderModule::~ShaderModule()
{
    if (m_handle)
    {
        vkDestroyShaderModule(m_device->GetHandle(), m_handle, nullptr);
    }
}

VkResult ShaderModule::Init(const ShaderProgramDesc& desc)
{
    VkShaderModuleCreateInfo createInfo {};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext    = nullptr;
    createInfo.flags    = 0;
    createInfo.codeSize = desc.shaderCode.size() * sizeof(uint32_t);
    createInfo.pCode    = desc.shaderCode.data();

    return vkCreateShaderModule(m_device->GetHandle(), &createInfo, nullptr, &m_handle);
}

Fence::~Fence()
{
    if (m_handle)
    {
        vkDestroyFence(m_device->GetHandle(), m_handle, nullptr);
    }
}

VkResult Fence::Init()
{
    VkFenceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    return vkCreateFence(m_device->GetHandle(), &createInfo, nullptr, &m_handle);
}

ResultCode Fence::Reset()
{
    return ConvertResult(vkResetFences(m_device->GetHandle(), 1, &m_handle));
}

ResultCode Fence::Wait() const
{
    return ConvertResult(vkWaitForFences(m_device->GetHandle(), 1, &m_handle, VK_TRUE, UINT64_MAX));
}

ResultCode Fence::GetStatus() const
{
    return ConvertResult(vkGetFenceStatus(m_device->GetHandle(), m_handle));
}

VkFilter ConvertFilter(SamplerFilter filter)
{
    return filter == SamplerFilter::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
}

VkSamplerMipmapMode ConvertSamplerMipMapMode(SamplerFilter filter)
{
    return filter == SamplerFilter::Linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
}

VkSamplerAddressMode ConvertSamplerAddressMode(SamplerAddressMode addressMode)
{
    return addressMode == SamplerAddressMode::Repeat ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
}

VkCompareOp ConvertSamplerCompareOp(SamplerCompareOp compareOp)
{
    switch (compareOp)
    {
        case SamplerCompareOp::Equal: return VK_COMPARE_OP_EQUAL;
        case SamplerCompareOp::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
        case SamplerCompareOp::Always: return VK_COMPARE_OP_ALWAYS;
        case SamplerCompareOp::Greater: return VK_COMPARE_OP_GREATER;
        case SamplerCompareOp::GreaterEq: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case SamplerCompareOp::Less: return VK_COMPARE_OP_LESS;
        case SamplerCompareOp::LessEq: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case SamplerCompareOp::Never: return VK_COMPARE_OP_NEVER;
    }
    return VK_COMPARE_OP_MAX_ENUM;
}

Sampler::~Sampler()
{
    if (m_handle)
    {
        vkDestroySampler(m_device->GetHandle(), m_handle, nullptr);
    }
}

VkResult Sampler::Init(const SamplerDesc& desc)
{
    VkSamplerCreateInfo createInfo {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.pNext                   = nullptr;
    createInfo.flags                   = 0;
    createInfo.magFilter               = ConvertFilter(desc.filter);
    createInfo.minFilter               = ConvertFilter(desc.filter);
    createInfo.mipmapMode              = ConvertSamplerMipMapMode(desc.filter);
    createInfo.addressModeU            = ConvertSamplerAddressMode(desc.addressU);
    createInfo.addressModeV            = ConvertSamplerAddressMode(desc.addressV);
    createInfo.addressModeW            = ConvertSamplerAddressMode(desc.addressW);
    createInfo.mipLodBias              = desc.mipLodBias;
    createInfo.anisotropyEnable        = VK_FALSE;
    createInfo.maxAnisotropy           = desc.maxAnisotropy;
    createInfo.compareEnable           = VK_TRUE;
    createInfo.compareOp               = ConvertSamplerCompareOp(desc.compare);
    createInfo.minLod                  = desc.minLod;
    createInfo.maxLod                  = desc.maxLod;
    createInfo.borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    createInfo.unnormalizedCoordinates = VK_TRUE;

    if (desc.maxAnisotropy != 0.0f)
    {
        createInfo.magFilter        = VK_FILTER_LINEAR;
        createInfo.minFilter        = VK_FILTER_LINEAR;
        createInfo.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        createInfo.anisotropyEnable = VK_TRUE;
        createInfo.maxAnisotropy    = desc.maxAnisotropy;
    }

    return vkCreateSampler(m_device->GetHandle(), &createInfo, nullptr, &m_handle);
}

Semaphore::Semaphore(Device& device, VkSemaphoreCreateFlags flags)
    : DeviceObject(device)
{
    VkSemaphoreCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = flags;

    Utils::AssertSuccess(vkCreateSemaphore(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

Semaphore::~Semaphore()
{
    if (m_handle)
    {
        vkDestroySemaphore(m_device->GetHandle(), m_handle, nullptr);
    }
}

}  // namespace Vulkan
}  // namespace RHI