#include "RHI/Backend/Vulkan/Texture.hpp"

#include "RHI/Backend/Vulkan/Factory.hpp"
#include "RHI/Backend/Vulkan/Utils.hpp"

namespace RHI
{
namespace Vulkan
{
    Expected<TexturePtr> Factory::CreateTexture(const MemoryAllocationDesc& allocDesc, const TextureDesc& desc)
    {
        // Create a texture resource.
        auto     texture = CreateUnique<Texture>(*m_device);
        VkResult result  = texture->Init(allocDesc, desc);

        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));

        return texture;
    }

    Texture::~Texture()
    {
        if (m_allocation != VK_NULL_HANDLE)
            vmaDestroyImage(m_pDevice->GetAllocator(), m_handle, m_allocation);
        else if (m_handle != VK_NULL_HANDLE)
            vkDestroyImage(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult Texture::Init(const MemoryAllocationDesc& allocDesc, const TextureDesc& desc)
    {
        VkImageCreateInfo createInfo = {};
        createInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.pNext             = nullptr;
        createInfo.flags             = 0;

        if (desc.extent.sizeY == 1 && desc.extent.sizeZ == 1)
            createInfo.imageType = VK_IMAGE_TYPE_1D;
        else if (desc.extent.sizeZ == 1)
            createInfo.imageType = VK_IMAGE_TYPE_2D;
        else
            createInfo.imageType = VK_IMAGE_TYPE_3D;

        createInfo.format      = Utils::ToVkFormat(desc.format);
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

        return vmaCreateImage(m_pDevice->GetAllocator(), &createInfo, &allocInfo, &m_handle, &m_allocation, &m_allocationInfo);
    }

    Expected<TextureViewPtr> Factory::CreateTextureView(const TextureViewDesc& desc)
    {
        auto     textureView = CreateUnique<TextureView>(*m_device);
        VkResult result      = textureView->Init(desc);
        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));

        return textureView;
    }

    TextureView::~TextureView() { vkDestroyImageView(m_pDevice->GetHandle(), m_handle, nullptr); }

    VkResult TextureView::Init(const TextureViewDesc& desc)
    {
        VkImageViewCreateInfo createInfo       = {};
        createInfo.sType                       = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext                       = nullptr;
        createInfo.flags                       = 0;
        createInfo.image                       = static_cast<Texture*>(desc.pTexture)->GetHandle();
        createInfo.viewType                    = static_cast<VkImageViewType>(desc.viewDimension);
        createInfo.format                      = Utils::ToVkFormat(desc.format);
        createInfo.components.a                = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.r                = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g                = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b                = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = 0;

        // Choose which aspects of the image this view will contain.
        if (desc.aspectFlags & ETextureViewAspectFlagBits::Color)
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        if (desc.aspectFlags & ETextureViewAspectFlagBits::Depth)
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (desc.aspectFlags & ETextureViewAspectFlagBits::Stencil)
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

        createInfo.subresourceRange.layerCount     = desc.viewRange.arrayCount;
        createInfo.subresourceRange.baseArrayLayer = desc.viewRange.arrayIndex;
        createInfo.subresourceRange.levelCount     = desc.viewRange.mipLevelsCount;
        createInfo.subresourceRange.baseMipLevel   = desc.viewRange.mipLevel;

        return vkCreateImageView(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

} // namespace Vulkan
} // namespace RHI
