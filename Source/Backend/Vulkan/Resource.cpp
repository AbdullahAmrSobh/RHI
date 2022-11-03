#include "RHI/Resource.hpp"
#include "Backend/Vulkan/Common.hpp"
#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{

    VmaMemoryUsage ConvertMemoryUsage(EMemoryUsage usage)
    {
        switch (usage)
        {
        case EMemoryUsage::Stream: return VMA_MEMORY_USAGE_CPU_COPY;
        case EMemoryUsage::Hosted: return VMA_MEMORY_USAGE_CPU_ONLY;
        case EMemoryUsage::Stage: return VMA_MEMORY_USAGE_CPU_TO_GPU;
        case EMemoryUsage::Local: return VMA_MEMORY_USAGE_GPU_ONLY;
        };

        return VMA_MEMORY_USAGE_UNKNOWN;
    }

    Expected<Unique<IShaderProgram>> Device::CreateShaderProgram(const ShaderProgramDesc& desc)
    {
        Unique<ShaderModule> shaderModule = CreateUnique<ShaderModule>(*this);
        VkResult             result       = shaderModule->Init(desc);

        if (RHI_SUCCESS(result))
            return std::move(shaderModule);

        return Unexpected(ConvertResult(result));
    }

    Expected<Unique<IFence>> Device::CreateFence()
    {
        Unique<Fence> fence  = CreateUnique<Fence>(*this);
        VkResult      result = fence->Init();

        if (RHI_SUCCESS(result))
            return std::move(fence);

        return Unexpected(ConvertResult(result));
    }

    Expected<Unique<IImage>> Device::CreateImage(const AllocationDesc& allocationDesc, const ImageDesc& desc)
    {
        Unique<Image> image  = CreateUnique<Image>(*this);
        VkResult      result = image->Init(allocationDesc, desc);

        if (RHI_SUCCESS(result))
            return std::move(image);

        return Unexpected(ConvertResult(result));
    }

    Expected<Unique<IImageView>> Device::CreateImageView(const IImage& image, const ImageViewDesc& desc)
    {
        Unique<ImageView> imageView = CreateUnique<ImageView>(*this);
        VkResult          result    = imageView->Init(static_cast<const Image&>(image), desc);

        if (RHI_SUCCESS(result))
            return std::move(imageView);

        return Unexpected(ConvertResult(result));
    }

    Expected<Unique<IBuffer>> Device::CreateBuffer(const AllocationDesc& allocationDesc, const BufferDesc& desc)
    {
        Unique<Buffer> buffer = CreateUnique<Buffer>(*this);
        VkResult       result = buffer->Init(allocationDesc, desc);

        if (RHI_SUCCESS(result))
            return std::move(buffer);

        return Unexpected(ConvertResult(result));
    }

    Expected<Unique<IBufferView>> Device::CreateBufferView(const IBuffer& buffer, const BufferViewDesc& desc)
    {
        Unique<BufferView> bufferView = CreateUnique<BufferView>(*this);
        VkResult           result     = bufferView->Init(static_cast<const Buffer&>(buffer), desc);

        if (RHI_SUCCESS(result))
            return std::move(bufferView);

        return Unexpected(ConvertResult(result));
    }

    Expected<Unique<ISampler>> Device::CreateSampler(const SamplerDesc& desc)
    {
        Unique<Sampler> sampler = CreateUnique<Sampler>(*this);
        VkResult        result  = sampler->Init(desc);

        if (RHI_SUCCESS(result))
            return std::move(sampler);

        return Unexpected(ConvertResult(result));
    }

    ShaderModule::~ShaderModule()
    {
        vkDestroyShaderModule(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult ShaderModule::Init(const ShaderProgramDesc& desc)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext    = nullptr;
        createInfo.flags    = 0;
        createInfo.codeSize = desc.shaderCode.size();
        createInfo.pCode    = desc.shaderCode.data();

        return vkCreateShaderModule(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    Fence::~Fence()
    {
        vkDestroyFence(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult Fence::Init()
    {
        VkFenceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;

        return vkCreateFence(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    EResultCode Fence::Wait() const
    {
        return ConvertResult(vkWaitForFences(m_pDevice->GetHandle(), 1, &m_handle, VK_TRUE, UINT64_MAX));
    }

    EResultCode Fence::Reset() const
    {
        return ConvertResult(vkResetFences(m_pDevice->GetHandle(), 1, &m_handle));
    }

    EResultCode Fence::GetStatus() const
    {
        return ConvertResult(vkGetFenceStatus(m_pDevice->GetHandle(), m_handle));
    }

    Image::~Image()
    {
        vkDestroyImage(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult Image::Init(const AllocationDesc& allocationDesc, const ImageDesc& desc)
    {
        VkImageCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        if (desc.extent.sizeY == 1 && desc.extent.sizeZ == 1)
        {
            createInfo.imageType = VK_IMAGE_TYPE_1D;
        }
        else if (desc.extent.sizeY == 1)
        {
            createInfo.imageType = VK_IMAGE_TYPE_2D;
        }
        else
        {
            createInfo.imageType = VK_IMAGE_TYPE_3D;
        }
        createInfo.format                = ConvertFormat(desc.format);
        createInfo.extent                = ConvertExtent(desc.extent);
        createInfo.mipLevels             = desc.mipLevelsCount;
        createInfo.arrayLayers           = desc.arraySize;
        createInfo.samples               = ConvertSampleCount(desc.sampleCount);
        createInfo.tiling                = VK_IMAGE_TILING_LINEAR;
        createInfo.usage                 = ConvertImageUsage(desc.usage);
        createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
        createInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

        VmaAllocationCreateInfo allocationCreateInfo;
        allocationCreateInfo.usage = ConvertMemoryUsage(allocationDesc.usage);

        return vmaCreateImage(m_pDevice->GetAllocator(), &createInfo, &allocationCreateInfo, &m_handle, &m_allocation, &m_allocationInfo);
    }

    ImageView::~ImageView()
    {
        vkDestroyImageView(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult ImageView::Init(const Image& image, const ImageViewDesc& desc)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext                           = nullptr;
        createInfo.flags                           = 0;
        createInfo.image                           = image.GetHandle();
        createInfo.viewType                        = ConvertImageViewType(desc.type);
        createInfo.format                          = ConvertFormat(desc.format);
        createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask     = ConvertViewAspect(desc.viewAspect);
        createInfo.subresourceRange.baseMipLevel   = desc.range.baseMipLevel;
        createInfo.subresourceRange.levelCount     = desc.range.mipLevelsCount;
        createInfo.subresourceRange.baseArrayLayer = desc.range.baseArrayElement;
        createInfo.subresourceRange.layerCount     = desc.range.arraySize;

        return vkCreateImageView(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    Buffer::~Buffer()
    {
        vkDestroyBuffer(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult Buffer::Init(const AllocationDesc& allocationDesc, const BufferDesc& desc)
    {
        VkBufferCreateInfo createInfo{};
        createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext                 = nullptr;
        createInfo.flags                 = 0;
        createInfo.size                  = desc.size;
        createInfo.usage                 = ConvertBufferUsage(desc.usage);
        createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;

        VmaAllocationCreateInfo allocationCreateInfo;
        allocationCreateInfo.usage = ConvertMemoryUsage(allocationDesc.usage);

        return vmaCreateBuffer(m_pDevice->GetAllocator(), &createInfo, &allocationCreateInfo, &m_handle, &m_allocation, &m_allocationInfo);
    }

    BufferView::~BufferView()
    {
        vkDestroyBufferView(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult BufferView::Init(const Buffer& buffer, const BufferViewDesc& desc)
    {
        VkBufferViewCreateInfo createInfo{};
        createInfo.sType  = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        createInfo.pNext  = nullptr;
        createInfo.flags  = 0;
        createInfo.buffer = buffer.GetHandle();
        createInfo.format = ConvertFormat(desc.format);
        createInfo.offset = desc.range.byteOffset;
        createInfo.range  = desc.range.byteRange;

        return vkCreateBufferView(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    Sampler::~Sampler()
    {
        vkDestroySampler(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkFilter ConvertFilter(ESamplerFilter filter)
    {
        return filter == ESamplerFilter::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
    }

    VkSamplerMipmapMode ConvertMipMapMode(ESamplerFilter filter)
    {
        return filter == ESamplerFilter::Linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }

    VkSamplerAddressMode ConvertAddressMode(ESamplerAddressMode addressMode)
    {
        return addressMode == ESamplerAddressMode::Repeat ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    }

    VkCompareOp ConvertCompareOp(ESamplerCompareOp compareOp)
    {
        switch (compareOp)
        {
        case ESamplerCompareOp::Equal: return VK_COMPARE_OP_EQUAL;
        case ESamplerCompareOp::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
        case ESamplerCompareOp::Always: return VK_COMPARE_OP_ALWAYS;
        case ESamplerCompareOp::Greater: return VK_COMPARE_OP_GREATER;
        case ESamplerCompareOp::GreaterEq: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case ESamplerCompareOp::Less: return VK_COMPARE_OP_LESS;
        case ESamplerCompareOp::LessEq: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case ESamplerCompareOp::Never: return VK_COMPARE_OP_NEVER;
        }
        return VK_COMPARE_OP_MAX_ENUM;
    }

    VkResult Sampler::Init(const SamplerDesc& desc)
    {
        VkSamplerCreateInfo createInfo{};
        createInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        createInfo.pNext                   = nullptr;
        createInfo.flags                   = 0;
        createInfo.magFilter               = ConvertFilter(desc.filter);
        createInfo.minFilter               = ConvertFilter(desc.filter);
        createInfo.mipmapMode              = ConvertMipMapMode(desc.filter);
        createInfo.addressModeU            = ConvertAddressMode(desc.addressU);
        createInfo.addressModeV            = ConvertAddressMode(desc.addressV);
        createInfo.addressModeW            = ConvertAddressMode(desc.addressW);
        createInfo.mipLodBias              = desc.mipLodBias;
        createInfo.anisotropyEnable        = VK_FALSE;
        createInfo.maxAnisotropy           = desc.maxAnisotropy;
        createInfo.compareEnable           = VK_TRUE;
        createInfo.compareOp               = ConvertCompareOp(desc.compare);
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

        return vkCreateSampler(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    Semaphore::~Semaphore()
    {
        vkDestroySemaphore(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult Semaphore::Init(bool bin)
    {
        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;

        return vkCreateSemaphore(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

} // namespace Vulkan
} // namespace RHI