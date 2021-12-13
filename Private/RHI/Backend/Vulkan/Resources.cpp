#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include "RHI/Backend/Vulkan/Resources.hpp"
#include "RHI/Backend/Vulkan/Common.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"
#include "RHI/Backend/Vulkan/Utils.hpp"
#include <algorithm>

namespace RHI
{
namespace Vulkan
{

    // Semaphore implementation.
    //---------------------------------------------------------------------------------------
    Semaphore::~Semaphore() { vkDestroySemaphore(m_pDevice->GetHandle(), m_handle, nullptr); }

    VkResult Semaphore::Init()
    {
        VkSemaphoreCreateInfo createInfo = {};
        createInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext                 = nullptr;
        createInfo.flags                 = 0;

        return vkCreateSemaphore(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    // Fence implementation.
    //---------------------------------------------------------------------------------------
    Expected<FencePtr> Factory::CreateFence()
    {
        auto     fence  = CreateUnique<Fence>(*m_device);
        VkResult result = fence->Init();
        if (result != VK_SUCCESS)
            return fence;
        else
            return tl::unexpected(ToResultCode(result));
    }

    Fence::~Fence() { vkDestroyFence(m_pDevice->GetHandle(), m_handle, nullptr); }

    VkResult Fence::Init()
    {
        VkFenceCreateInfo createInfo = {};
        createInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.flags             = 0;
        createInfo.pNext             = nullptr;

        return vkCreateFence(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    EResultCode Fence::Wait() const
    {
        VkResult result = vkWaitForFences(m_pDevice->GetHandle(), 1, &m_handle, VK_TRUE, UINT64_MAX);
        return ToResultCode(result);
    }

    EResultCode Fence::Reset() const
    {
        VkResult result = vkResetFences(m_pDevice->GetHandle(), 1, &m_handle);
        return ToResultCode(result);
    }

    EResultCode Fence::GetStatus() const
    {
        VkResult result = vkGetFenceStatus(m_pDevice->GetHandle(), m_handle);
        return ToResultCode(result);
    }

    // DeviceMemoryAllocation implementation.
    //---------------------------------------------------------------------------------------
    DeviceMemoryAllocation::~DeviceMemoryAllocation() { vmaFreeMemory(m_pDevice->GetAllocator(), m_handle); }

    VkResult DeviceMemoryAllocation::Init(size_t size, size_t alignment, DeviceMemoryAllocationDesc _desc)
    {
        VmaAllocationCreateInfo allocationCreateInfo = {};
        VkMemoryRequirements    memoryRequirements   = {};
        return vmaAllocateMemory(m_pDevice->GetAllocator(), &memoryRequirements, &allocationCreateInfo, &m_handle, &m_allocationInfo);
    }

    VkResult DeviceMemoryAllocation::InitForTexture(VkImage _image, DeviceMemoryAllocationDesc _desc)
    {
        VmaAllocationCreateInfo allocationCreateInfo = {};
        // allocationCreateInfo.flags;
        allocationCreateInfo.usage = Utils::ToVmaMemoryUsage(_desc.usage);
        // allocationCreateInfo.requiredFlags;
        // allocationCreateInfo.preferredFlags;
        // allocationCreateInfo.memoryTypeBits;
        // allocationCreateInfo.pool;
        // allocationCreateInfo.pUserData;
        // allocationCreateInfo.priority;

        return vmaAllocateMemoryForImage(m_pDevice->GetAllocator(), _image, &allocationCreateInfo, &m_handle, &m_allocationInfo);
    }

    VkResult DeviceMemoryAllocation::InitForBuffer(VkBuffer _buffer, DeviceMemoryAllocationDesc _desc)
    {
        VmaAllocationCreateInfo allocationCreateInfo = {};
        // allocationCreateInfo.flags;
        allocationCreateInfo.usage = Utils::ToVmaMemoryUsage(_desc.usage);
        // allocationCreateInfo.requiredFlags;
        // allocationCreateInfo.preferredFlags;
        // allocationCreateInfo.memoryTypeBits;
        // allocationCreateInfo.pool;
        // allocationCreateInfo.pUserData;
        // allocationCreateInfo.priority;

        return vmaAllocateMemoryForBuffer(m_pDevice->GetAllocator(), _buffer, &allocationCreateInfo, &m_handle, &m_allocationInfo);
    }

    size_t DeviceMemoryAllocation::GetSize() const { return m_allocationInfo.size; }

    Expected<DeviceAddress> DeviceMemoryAllocation::Map(size_t offset, size_t range) const
    {
        DeviceAddress pDeviceAddress;
        VkResult      result = vmaMapMemory(m_pDevice->GetAllocator(), m_handle, &pDeviceAddress);
        if (result != VK_SUCCESS)
            return pDeviceAddress;
        else
            return tl::unexpected(ToResultCode(result));
    }

    EResultCode DeviceMemoryAllocation::Unmap() const
    {
        vmaUnmapMemory(m_pDevice->GetAllocator(), m_handle);
        return EResultCode::Success;
    }

    // Texture Implementation.
    //---------------------------------------------------------------------------------------
    Expected<TexturePtr> Factory::CreateTexture(const DeviceMemoryAllocationDesc& _allocDesc, const TextureDesc& _desc)
    {
        // Create a texture resource.
        auto     texture = CreateUnique<Texture>(*m_device);
        VkResult result  = texture->Init(_desc);

        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));

        // Allocate suitable memory for the created resource.
        DeviceMemoryAllocation* pAllocation = new DeviceMemoryAllocation(*m_device);
        result                              = pAllocation->InitForTexture(texture->GetHandle(), _allocDesc);

        // Bind memory to the buffer.
        texture->SetAllocation(*pAllocation);

        return texture;
    }

    Texture::~Texture()
    {
        if (m_allocation != nullptr)
        {
            vkDestroyImage(m_pDevice->GetHandle(), m_handle, nullptr);
            delete m_allocation;
        }
    }

    VkResult Texture::Init(const TextureDesc& _desc)
    {
        VkImageCreateInfo createInfo = {};
        createInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.pNext             = nullptr;
        createInfo.flags             = 0;

        if (_desc.extent.sizeY == 1 && _desc.extent.sizeZ == 1)
            createInfo.imageType = VK_IMAGE_TYPE_1D;
        else if (_desc.extent.sizeZ == 1)
            createInfo.imageType = VK_IMAGE_TYPE_2D;
        else
            createInfo.imageType = VK_IMAGE_TYPE_3D;

        createInfo.format                = Utils::ToVkFormat(_desc.format);
        createInfo.extent                = {_desc.extent.sizeX, _desc.extent.sizeX, _desc.extent.sizeZ};
        createInfo.mipLevels             = _desc.mipLevels;
        createInfo.arrayLayers           = _desc.arrayLayers;
        createInfo.samples               = Utils::ToVkSampleCountFlagBits(_desc.sampleCount);
        createInfo.tiling                = VK_IMAGE_TILING_LINEAR;
        createInfo.usage                 = Utils::ToVkImageUsageFlags(_desc.usage);
        createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
        createInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

        return vkCreateImage(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }
    VkResult Texture::InitForSwapChain(Extent3D _extent, EPixelFormat _format, VkImage _image)
    {

        m_handle           = _image;
        m_extent           = {_extent.sizeX, _extent.sizeY, _extent.sizeZ};
        m_mipLevelsCount   = 1;
        m_arrayLayersCount = 1;
        m_pixelFormat      = _format;
        m_allocation       = nullptr;

        return VK_SUCCESS;
    }

    Extent3D Texture::GetExtent() const { return m_extent; }

    uint32_t                 Texture::GetMipLevelsCount() const { return m_mipLevelsCount; }
    uint32_t                 Texture::GetArrayLayersCount() const { return m_arrayLayersCount; }
    EPixelFormat             Texture::GetPixelFormat() const { return m_pixelFormat; }
    TextureUsageFlags        Texture::GetUsage() const { return m_usage; }
    ESampleCount        Texture::GetSampleCount() const { return m_sampleCount; }
    IDeviceMemoryAllocation& Texture::GetAllocation() { return *m_allocation; }
    
    // TextureView Implementation.
    // --------------------------------------------------------------------------------------
    Expected<TextureViewPtr> Factory::CreateTextureView(const TextureViewDesc& _desc)
    {
        auto     textureView = CreateUnique<TextureView>(*m_device);
        VkResult result      = textureView->Init(_desc);
        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));

        return textureView;
    }

    TextureView::~TextureView() { vkDestroyImageView(m_pDevice->GetHandle(), m_handle, nullptr); }

    VkResult TextureView::Init(const TextureViewDesc& _desc)
    {
        VkImageViewCreateInfo createInfo       = {};
        createInfo.sType                       = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext                       = nullptr;
        createInfo.flags                       = 0;
        createInfo.image                       = static_cast<Texture*>(_desc.pTexture)->GetHandle();
        createInfo.viewType                    = static_cast<VkImageViewType>(_desc.dimensions);
        createInfo.format                      = Utils::ToVkFormat(_desc.format);
        createInfo.components.a                = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.r                = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g                = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b                = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = 0;

        // Choose which aspects of the image this view will contain.
        if (_desc.aspectFlags & ETextureViewAspectFlagBits::Color)
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        if (_desc.aspectFlags & ETextureViewAspectFlagBits::Depth)
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (_desc.aspectFlags & ETextureViewAspectFlagBits::Stencil)
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

        createInfo.subresourceRange.layerCount     = _desc.subresourceRange.arrayCount;
        createInfo.subresourceRange.baseArrayLayer = _desc.subresourceRange.arrayIndex;
        createInfo.subresourceRange.levelCount     = _desc.subresourceRange.mipLevelsCount;
        createInfo.subresourceRange.baseMipLevel   = _desc.subresourceRange.mipLevel;

        return vkCreateImageView(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    // Buffer Implementation.
    //---------------------------------------------------------------------------------------
    Expected<BufferPtr> Factory::CreateBuffer(const DeviceMemoryAllocationDesc& _allocDesc, const BufferDesc& _desc)
    {
        // Create a buffer resource.
        auto     buffer = CreateUnique<Buffer>(*m_device);
        VkResult result = buffer->Init(_desc);

        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));

        // Allocate suitable memory for the created resource.
        DeviceMemoryAllocation* pAllocation = new DeviceMemoryAllocation(*m_device);
        result                              = pAllocation->InitForBuffer(buffer->GetHandle(), _allocDesc);

        // Bind memory to the buffer.
        buffer->SetAllocation(*pAllocation);

        return buffer;
    }

    Buffer::~Buffer()
    {
        vkDestroyBuffer(m_pDevice->GetHandle(), m_handle, nullptr);
        delete m_allocation;
    }

    VkResult Buffer::Init(const BufferDesc& _desc)
    {

        VkBufferCreateInfo createInfo    = {};
        createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext                 = nullptr;
        createInfo.flags                 = 0;
        createInfo.size                  = _desc.size;
        createInfo.usage                 = Utils::ToVkBufferUsageFlags(_desc.usage);
        createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;

        return vkCreateBuffer(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    size_t                   Buffer::GetSize() const { return m_size; }
    IDeviceMemoryAllocation& Buffer::GetAllocation() { return *m_allocation; }

    // SwapChain Implementation.
    //---------------------------------------------------------------------------------------
    static VkSurfaceFormatKHR SelectSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
    {
        for (const auto& availableFormat : formats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;
        }
        return formats[0];
    }

    static VkExtent2D ClampSurfaceExtent(VkExtent2D actualExtent, VkExtent2D currentExtent, VkExtent2D minImageExtent, VkExtent2D maxImageExtent)
    {
        if (actualExtent.width != UINT32_MAX)
        {
            return currentExtent;
        }
        else
        {
            actualExtent.width  = std::max(minImageExtent.width, std::min(maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(minImageExtent.height, std::min(maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    static VkPresentModeKHR SelectSurfacePresentMode(const std::vector<VkPresentModeKHR>& presentModes)
    {
        for (const auto& availablePresentMode : presentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    Expected<SwapChainPtr> Factory::CreateSwapChain(const SwapChainDesc& _desc)
    {
        auto         swapchain = CreateUnique<SwapChain>(*m_device);
        VkSurfaceKHR surface   = VK_NULL_HANDLE;
        VkResult     result    = CreateSurfaceIfNotExist(_desc.windowHandle, &surface);
        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));
        result = swapchain->Init(surface, _desc);
        return swapchain;
    }

    SwapChain::~SwapChain() { vkDestroySwapchainKHR(m_pDevice->GetHandle(), m_handle, nullptr); }

    VkResult SwapChain::Init(VkSurfaceKHR surface, const SwapChainDesc& _desc)
    {
        m_currentFrameIndex = 0;

        // query surface capabilities
        VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
        VkResult                 result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_pDevice->GetPhysicalDevice().GetHandle(), surface, &surfaceCapabilities);
        if (result != VK_SUCCESS)
            return result;

        // query supported surface formats
        uint32_t                        formatCount = 0;
        std::vector<VkSurfaceFormatKHR> formats;
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_pDevice->GetPhysicalDevice().GetHandle(), surface, &formatCount, nullptr);
        formats.resize(formatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_pDevice->GetPhysicalDevice().GetHandle(), surface, &formatCount, formats.data());
        VkSurfaceFormatKHR selectedSurfaceFormat = SelectSurfaceFormat(formats);
        if (result != VK_SUCCESS)
            return result;

        // query supported surface present modes
        uint32_t                      presentModeCount = 0;
        std::vector<VkPresentModeKHR> presentModes;
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_pDevice->GetPhysicalDevice().GetHandle(), surface, &presentModeCount, nullptr);
        presentModes.resize(presentModeCount);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_pDevice->GetPhysicalDevice().GetHandle(), surface, &presentModeCount, presentModes.data());
        VkPresentModeKHR selectedPresentMode = SelectSurfacePresentMode(presentModes);
        if (result != VK_SUCCESS)
            return result;

        // create swapchain
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.surface                  = surface;
        createInfo.minImageCount            = std::clamp(_desc.backBuffersCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
        createInfo.imageFormat              = selectedSurfaceFormat.format;
        createInfo.imageColorSpace          = selectedSurfaceFormat.colorSpace;
        createInfo.imageExtent              = ClampSurfaceExtent({_desc.extent.sizeX, _desc.extent.sizeY}, surfaceCapabilities.currentExtent,
                                                                 surfaceCapabilities.minImageExtent, surfaceCapabilities.maxImageExtent);
        createInfo.imageArrayLayers         = 1;
        createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount    = 0;
        createInfo.pQueueFamilyIndices      = nullptr;
        createInfo.preTransform             = surfaceCapabilities.currentTransform;
        createInfo.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode              = selectedPresentMode;
        createInfo.clipped                  = VK_TRUE;
        createInfo.oldSwapchain             = VK_NULL_HANDLE;

        result = vkCreateSwapchainKHR(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
        if (result != VK_SUCCESS)
            return result;

        // Obtain the swapchain images
        uint32_t imageCount = 0;
        result              = vkGetSwapchainImagesKHR(m_pDevice->GetHandle(), m_handle, &imageCount, nullptr);
        std::vector<VkImage> images(imageCount);
        m_backBuffers.reserve(imageCount);
        result = vkGetSwapchainImagesKHR(m_pDevice->GetHandle(), m_handle, &imageCount, images.data());
        if (result != VK_SUCCESS)
            return result;

        for (auto& image : images)
        {
            m_backBuffers.emplace_back(*m_pDevice);
            m_backBuffers.back().InitForSwapChain({createInfo.imageExtent.width, createInfo.imageExtent.height}, EPixelFormat::BGRA, image);

            VkSemaphoreCreateInfo createInfo = {};
            createInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            createInfo.pNext                 = nullptr;
            createInfo.flags                 = 0;

            m_ImageAvailableSemaphores.push_back(VkSemaphore());
            result = vkCreateSemaphore(m_pDevice->GetHandle(), &createInfo, nullptr, &m_ImageAvailableSemaphores.back());
        }

        return result;
    }

    EResultCode SwapChain::SwapBackBuffers()
    {
        VkSemaphore imageAvailableSemaphore = m_ImageAvailableSemaphores[m_currentFrameIndex];
        VkResult    result = vkAcquireNextImageKHR(m_pDevice->GetHandle(), m_handle, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &m_currentImageIndex);

        m_currentFrameIndex = (m_currentImageIndex + 1) % static_cast<uint32_t>(m_backBuffers.size());
        return ToResultCode(result);
    }

    ITexture* SwapChain::GetBackBuffers() { return m_backBuffers.data(); }
    uint32_t  SwapChain::GetBackBufferCount() const { return static_cast<uint32_t>(m_backBuffers.size()); }
    uint32_t  SwapChain::GetCurrentBackBufferIndex() const { return m_currentImageIndex; }

    // RenderTarget Implementation.
    Expected<RenderTargetPtr> Factory::CreateRenderTarget(const RenderTargetDesc& _desc)
    {
        auto         renderTarget = CreateUnique<RenderTarget>(*m_device);
        VkRenderPass renderPass   = VK_NULL_HANDLE;
        VkResult     result       = CreateRenderPassIfNotExist(_desc, &renderPass);
        result                    = renderTarget->Init(renderPass, _desc);
        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));
        return renderTarget;
    }
    RenderTarget::~RenderTarget() { vkDestroyFramebuffer(m_pDevice->GetHandle(), m_handle, nullptr); }
    VkResult RenderTarget::Init(VkRenderPass renderPass, const RenderTargetDesc& _desc)
    {
        std::vector<VkImageView> m_attachments;
        for (auto& attachment : _desc.colorAttachments)
            m_attachments.push_back(static_cast<TextureView*>(attachment)->GetHandle());

        if (_desc.pDepthStencilAttachment != nullptr)
            m_attachments.push_back(static_cast<TextureView*>(_desc.pDepthStencilAttachment)->GetHandle());

        VkFramebufferCreateInfo createInfo = {};
        createInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext                   = nullptr;
        createInfo.flags                   = 0;
        createInfo.renderPass              = renderPass;
        createInfo.attachmentCount         = static_cast<uint32_t>(m_attachments.size());
        createInfo.pAttachments            = m_attachments.data();
        createInfo.width                   = m_extent.sizeX;
        createInfo.height                  = m_extent.sizeY;
        createInfo.layers                  = 1;

        return vkCreateFramebuffer(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    Extent2D       RenderTarget::GetExtent() const { return m_extent; }
    uint32_t       RenderTarget::GetCount() const { return static_cast<uint32_t>(m_pAttachments.size()); }
    ITextureView** RenderTarget::GetAttachments() { return m_pAttachments.data(); }

    // Sampler Implementation.
    Expected<SamplerPtr> Factory::CreateSampler(const SamplerDesc& _desc)
    {
        auto     sampler = CreateUnique<Sampler>(*m_device);
        VkResult result  = sampler->Init(_desc);
        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));
        return sampler;
    };

    Sampler::~Sampler() { vkDestroySampler(m_pDevice->GetHandle(), m_handle, nullptr); }

    VkResult Sampler::Init(const SamplerDesc& _desc)
    {
        VkSamplerCreateInfo createInfo = {};
        createInfo.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        createInfo.pNext               = nullptr;
        createInfo.flags               = 0;

        return vkCreateSampler(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

} // namespace Vulkan

} // namespace RHI
