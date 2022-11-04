#include "RHI/Resource.hpp"
#include "Backend/Vulkan/Common.hpp"
#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{

    Expected<Unique<IShaderProgram>> Device::CreateShaderProgram(const ShaderProgramDesc& desc)
    {
        Unique<ShaderModule> shaderModule = CreateUnique<ShaderModule>(*this, desc.entryName);
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
        if (m_handle)
        {
            vkDestroyShaderModule(m_pDevice->GetHandle(), m_handle, nullptr);
        }
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
        if (m_handle)
        {
            vkDestroyFence(m_pDevice->GetHandle(), m_handle, nullptr);
        }
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
        if (m_handle)
        {
            vkDestroyImage(m_pDevice->GetHandle(), m_handle, nullptr);
        }
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

        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = ConvertMemoryUsage(allocationDesc.usage);

        VkResult result = vmaCreateImage(m_pDevice->GetAllocator(), &createInfo, &allocationCreateInfo, &m_handle, &m_allocation, &m_allocationInfo);

        if (RHI_SUCCESS(result))
        {
            m_memorySize = m_allocationInfo.size;
        }

        return result;
    }

    ImageView::~ImageView()
    {
        if (m_handle)
        {
            vkDestroyImageView(m_pDevice->GetHandle(), m_handle, nullptr);
        }
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
        if (m_handle)
        {
            vkDestroyBuffer(m_pDevice->GetHandle(), m_handle, nullptr);
        }
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
        
        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = ConvertMemoryUsage(allocationDesc.usage);
        
        VkResult result = vmaCreateBuffer(m_pDevice->GetAllocator(), &createInfo, &allocationCreateInfo, &m_handle, &m_allocation, &m_allocationInfo);
        
        if (RHI_SUCCESS(result))
        {
            m_memorySize = m_allocationInfo.size;
        }

        return result;
    }

    BufferView::~BufferView()
    {
        if (m_handle)
        {
            vkDestroyBufferView(m_pDevice->GetHandle(), m_handle, nullptr);
        }
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
        if (m_handle)
        {
            vkDestroySampler(m_pDevice->GetHandle(), m_handle, nullptr);
        }
    }

    VkResult Sampler::Init(const SamplerDesc& desc)
    {
        VkSamplerCreateInfo createInfo{};
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

        return vkCreateSampler(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    Semaphore::~Semaphore()
    {
        if (m_handle)
        {
            vkDestroySemaphore(m_pDevice->GetHandle(), m_handle, nullptr);
        }
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