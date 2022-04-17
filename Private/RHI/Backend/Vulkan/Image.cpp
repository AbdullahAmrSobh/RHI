#include "RHI/Backend/Vulkan/Image.hpp"

#include "RHI/Backend/Vulkan/Factory.hpp"
#include "RHI/Backend/Vulkan/Utils.hpp"

namespace RHI
{
namespace Vulkan
{
    Expected<ImagePtr> Factory::CreateImage(const MemoryAllocationDesc& allocDesc, const ImageDesc& desc)
    {
        auto     image  = CreateUnique<Image>(*m_device);
        VkResult result = image->Init(allocDesc, desc);
        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));

        return image;
    }

    Image::~Image()
    {
        if (m_allocation != VK_NULL_HANDLE)
            vmaDestroyImage(m_pDevice->GetAllocator(), m_handle, m_allocation);
        else if (m_handle != VK_NULL_HANDLE)
            vkDestroyImage(m_pDevice->GetHandle(), m_handle, nullptr);
    }
    
    VkResult Image::Init(const MemoryAllocationDesc& allocDesc, const ImageDesc& desc)
    {
        VkImageCreateInfo createInfo = {};
        createInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.pNext             = nullptr;
        createInfo.flags             = 0;
        {
            if (desc.extent.sizeY == 1 && desc.extent.sizeZ == 1)
                createInfo.imageType = VK_IMAGE_TYPE_1D;
            else if (desc.extent.sizeZ == 1)
                createInfo.imageType = VK_IMAGE_TYPE_2D;
            else
                createInfo.imageType = VK_IMAGE_TYPE_3D;
        }
        createInfo.format      = Utils::ConvertPixelFormat(desc.format);
        createInfo.extent      = {desc.extent.sizeX, desc.extent.sizeX, desc.extent.sizeZ};
        createInfo.mipLevels   = desc.mipLevels;
        createInfo.arrayLayers = desc.arraySize;
        createInfo.samples     = static_cast<VkSampleCountFlagBits>(desc.sampleCount);
        createInfo.tiling      = VK_IMAGE_TILING_LINEAR;
        memcpy(&createInfo.usage, &desc.usage, sizeof(desc.usage));
        createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
        createInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage                   = static_cast<VmaMemoryUsage>(allocDesc.usage);

        Assert(m_resourceIsReadySemaphore.Init());

        return vmaCreateImage(m_pDevice->GetAllocator(), &createInfo, &allocInfo, &m_handle, &m_allocation, &m_allocationInfo);
    }

    Expected<ImageViewPtr> Factory::CreateImageView(const ImageViewDesc& desc)
    {
        auto     imageView = CreateUnique<ImageView>(*m_device);
        VkResult result    = imageView->Init(desc);
        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));

        return imageView;
    }

    ImageView::~ImageView() { vkDestroyImageView(m_pDevice->GetHandle(), m_handle, nullptr); }

    VkResult ImageView::Init(const ImageViewDesc& desc)
    {
        VkImageViewCreateInfo createInfo       = {};
        createInfo.sType                       = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext                       = nullptr;
        createInfo.flags                       = 0;
        createInfo.image                       = static_cast<Image*>(desc.pImage)->GetHandle();
        createInfo.viewType                    = static_cast<VkImageViewType>(desc.viewDimension);
        createInfo.format                      = Utils::ConvertPixelFormat(desc.format);
        createInfo.components.a                = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.r                = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g                = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b                = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = 0;

        // Choose which aspects of the image this view will contain.
        if (desc.aspectFlags & EImageViewAspectFlagBits::Color)
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        if (desc.aspectFlags & EImageViewAspectFlagBits::Depth)
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (desc.aspectFlags & EImageViewAspectFlagBits::Stencil)
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

        createInfo.subresourceRange.layerCount     = desc.viewRange.arrayCount;
        createInfo.subresourceRange.baseArrayLayer = desc.viewRange.arrayIndex;
        createInfo.subresourceRange.levelCount     = desc.viewRange.mipLevelsCount;
        createInfo.subresourceRange.baseMipLevel   = desc.viewRange.mipLevel;

        return vkCreateImageView(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

} // namespace Vulkan
} // namespace RHI
