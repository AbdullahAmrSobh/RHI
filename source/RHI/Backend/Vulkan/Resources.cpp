#pragma once

#include "RHI/Backend/Vulkan/Resources.hpp"

#include "RHI/Result.hpp"

#include "RHI/Backend/Vulkan/Context.hpp"
#include "RHI/Backend/Vulkan/Conversion.inl"

namespace Vulkan
{

VmaMemoryUsage GetVmaMemoryUsage(RHI::ResourceMemoryType type)
{
    switch (type)
    {
        case RHI::ResourceMemoryType::Stage: return VMA_MEMORY_USAGE_CPU_TO_GPU;
        case RHI::ResourceMemoryType::DeviceLocal: return VMA_MEMORY_USAGE_GPU_ONLY;
        case RHI::ResourceMemoryType::None:
        default: RHI_ASSERT_MSG(false, "Invalid usage");
    };
    return VMA_MEMORY_USAGE_MAX_ENUM;
}

vk::BufferUsageFlags GetBufferUsageFlags(RHI::Flags<RHI::BufferUsage> usage)
{
    RHI_ASSERT_MSG(usage, "invalid unspecified usage")

    vk::BufferUsageFlags flags;

    if (usage & RHI::BufferUsage::Storage)
    {
        flags |= vk::BufferUsageFlagBits::eStorageBuffer;
    }

    if (usage & RHI::BufferUsage::Uniform)
    {
        flags |= vk::BufferUsageFlagBits::eUniformBuffer;
    }

    if (usage & RHI::BufferUsage::Vertex)
    {
        flags |= vk::BufferUsageFlagBits::eVertexBuffer;
    }

    if (usage & RHI::BufferUsage::Index)
    {
        flags |= vk::BufferUsageFlagBits::eIndexBuffer;
    }

    if (usage & RHI::BufferUsage::CopySrc)
    {
        flags |= vk::BufferUsageFlagBits::eTransferSrc;
    }

    if (usage & RHI::BufferUsage::CopyDst)
    {
        flags |= vk::BufferUsageFlagBits::eTransferDst;
    }

    return flags;
}

vk::ImageUsageFlags GetImageUsageFlags(RHI::Flags<RHI::ImageUsage> usage)
{
    RHI_ASSERT_MSG(usage, "invalid unspecified usage")

    vk::ImageUsageFlags flags;

    if (usage & RHI::ImageUsage::Color)
    {
        flags |= vk::ImageUsageFlagBits::eColorAttachment;
    }

    if (usage & RHI::ImageUsage::Depth)
    {
        return flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
    }

    if (usage & RHI::ImageUsage::ShaderRead)
    {
        return flags |= vk::ImageUsageFlagBits::eSampled;
    }

    if (usage & RHI::ImageUsage::ShaderRead)
    {
        return flags |= vk::ImageUsageFlagBits::eStorage;
    }

    if (usage & RHI::ImageUsage::CopyDst)
    {
        return flags |= vk::ImageUsageFlagBits::eTransferDst;
    }

    if (usage & RHI::ImageUsage::CopySrc)
    {
        return flags |= vk::ImageUsageFlagBits::eTransferSrc;
    }

    return flags;
}

vk::ImageType GetImageType(RHI::ImageType type)
{
    switch (type)
    {
        case RHI::ImageType::Image1D: return vk::ImageType::e1D;
        case RHI::ImageType::Image2D: return vk::ImageType::e2D;
        case RHI::ImageType::Image3D: return vk::ImageType::e3D;
        default: RHI_ASSERT_MSG(false, "Invalid image type");
    }

    return {};
}

vk::ImageViewType GetImageViewType(RHI::ImageType type, bool asArray)
{
    switch (type)
    {
        case RHI::ImageType::Image1D: return asArray ? vk::ImageViewType::e1D : vk::ImageViewType::e1DArray;
        case RHI::ImageType::Image2D: return asArray ? vk::ImageViewType::e2D : vk::ImageViewType::e2DArray;
        case RHI::ImageType::Image3D: RHI_ASSERT(!asArray); return vk::ImageViewType::e3D;
        default: RHI_ASSERT_MSG(false, "Invalid image type");
    }

    return {};
}

vk::SampleCountFlags GetSampleCount(RHI::SampleCount sampleCount)
{
    return static_cast<vk::SampleCountFlags>(static_cast<uint32_t>(sampleCount));
}

vk::Filter GetFilter(RHI::SamplerFilter filter)
{
    switch (filter)
    {
        case RHI::SamplerFilter::Linear: return vk::Filter::eLinear;
        case RHI::SamplerFilter::Nearest: return vk::Filter::eNearest;
        default: RHI_ASSERT_MSG(false, "Invalid image type");
    }
    return {};
}

vk::SamplerAddressMode GetSamplerAddressMode(RHI::SamplerAddressMode addressMode)
{
    switch (addressMode)
    {
        case RHI::SamplerAddressMode::Clamp: return vk::SamplerAddressMode::eClampToEdge;
        case RHI::SamplerAddressMode::Repeat: return vk::SamplerAddressMode::eRepeat;
        default: RHI_ASSERT_MSG(false, "Invalid image type");
    }
    return {};
}

vk::CompareOp GetSamplerCompareOp(RHI::SamplerCompareOp compareOp)
{
    switch (compareOp)
    {
        case RHI::SamplerCompareOp::Never: return vk::CompareOp::eNever;
        case RHI::SamplerCompareOp::Equal: return vk::CompareOp::eEqual;
        case RHI::SamplerCompareOp::NotEqual: return vk::CompareOp::eNotEqual;
        case RHI::SamplerCompareOp::Always: return vk::CompareOp::eAlways;
        case RHI::SamplerCompareOp::Less: return vk::CompareOp::eLess;
        case RHI::SamplerCompareOp::LessEq: return vk::CompareOp::eLessOrEqual;
        case RHI::SamplerCompareOp::Greater: return vk::CompareOp::eGreater;
        case RHI::SamplerCompareOp::GreaterEq: return vk::CompareOp::eGreaterOrEqual;
        default: RHI_ASSERT_MSG(false, "Invalid image type");
    }
    return {};
}

vk::ComponentSwizzle ConvertComponentSwizzle(RHI::ComponentSwizzle swizzle)
{
    switch (swizzle)
    {
        case RHI::ComponentSwizzle::None: return vk::ComponentSwizzle::eIdentity;
        case RHI::ComponentSwizzle::Zero: return vk::ComponentSwizzle::eZero;
        case RHI::ComponentSwizzle::One: return vk::ComponentSwizzle::eOne;
        case RHI::ComponentSwizzle::R: return vk::ComponentSwizzle::eR;
        case RHI::ComponentSwizzle::B: return vk::ComponentSwizzle::eB;
        case RHI::ComponentSwizzle::G: return vk::ComponentSwizzle::eG;
        case RHI::ComponentSwizzle::A: return vk::ComponentSwizzle::eA;
        default: RHI_ASSERT_MSG(false, "Invalid image type");
    }
    return {};
}

Buffer::~Buffer()
{
    auto& context   = static_cast<Context&>(*m_context);
    auto  device    = context.GetDevice();
    auto  allocator = context.GetAllocator();
    vmaDestroyBuffer(allocator, m_handle, m_allocation);
}

RHI::ResultCode Buffer::Init(const RHI::ResourceAllocationInfo& allocationInfo, const RHI::BufferCreateInfo& createInfo)
{
    auto& context   = static_cast<Context&>(*m_context);
    auto  device    = context.GetDevice();
    auto  allocator = context.GetAllocator();

    VmaAllocationCreateInfo allocationCreateInfo {};
    allocationCreateInfo.usage = GetVmaMemoryUsage(allocationInfo.usage);

    VkBufferCreateInfo bufferCreateInfo {};
    bufferCreateInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext                 = nullptr;
    bufferCreateInfo.flags                 = 0;
    bufferCreateInfo.size                  = createInfo.byteSize;
    bufferCreateInfo.usage                 = (VkBufferUsageFlags)GetBufferUsageFlags(createInfo.usageFlags);
    bufferCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.queueFamilyIndexCount = 0;
    bufferCreateInfo.pQueueFamilyIndices   = nullptr;

    VkBuffer buffer = VK_NULL_HANDLE;
    auto     result = vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &m_allocation, &m_allocationInfo);
    RHI_ASSERT(result == VK_SUCCESS);
    m_handle = buffer;

    return RHI::ResultCode::Success;
}

std::shared_ptr<RHI::BufferView> Buffer::CreateView(const RHI::BufferViewCreateInfo& createInfo)
{
    size_t key = RHI::HashAny(createInfo);
    if (auto view = m_viewCache.Get(key); view != nullptr)
        return view;

    std::shared_ptr<BufferView> view = std::make_shared<BufferView>(*m_context);
    view->Init(*this, createInfo);
    m_viewCache.Insert(key, view);
    return view;
}

Image::~Image()
{
    auto& context   = static_cast<Context&>(*m_context);
    auto  device    = context.GetDevice();
    auto  allocator = context.GetAllocator();
    vmaDestroyImage(allocator, m_handle, m_allocation);
}

RHI::ResultCode Image::Init(const RHI::ResourceAllocationInfo& allocationInfo, const RHI::ImageCreateInfo& createInfo)
{
    auto& context   = static_cast<Context&>(*m_context);
    auto  device    = context.GetDevice();
    auto  allocator = context.GetAllocator();

    VmaAllocationCreateInfo allocationCreateInfo {};
    allocationCreateInfo.usage = GetVmaMemoryUsage(allocationInfo.usage);

    VkImageCreateInfo imageCreateInfo {};
    imageCreateInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext                 = nullptr;
    imageCreateInfo.flags                 = {};
    imageCreateInfo.imageType             = static_cast<VkImageType>(GetImageType(createInfo.type));
    imageCreateInfo.format                = static_cast<VkFormat>(ConvertFormat(createInfo.format));
    imageCreateInfo.extent.width          = createInfo.size.width;
    imageCreateInfo.extent.height         = createInfo.size.height;
    imageCreateInfo.extent.depth          = createInfo.size.depth;
    imageCreateInfo.mipLevels             = createInfo.mipLevels;
    imageCreateInfo.arrayLayers           = createInfo.arrayCount;
    imageCreateInfo.samples               = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage                 = static_cast<VkImageUsageFlags>(GetImageUsageFlags(createInfo.usageFlags));
    imageCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.pQueueFamilyIndices   = nullptr;
    imageCreateInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage image  = VK_NULL_HANDLE;
    auto    result = vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &image, &m_allocation, &m_allocationInfo);
    RHI_ASSERT(result == VK_SUCCESS);
    m_handle = image;

    return RHI::ResultCode::Success;
}

std::shared_ptr<RHI::ImageView> Image::CreateView(const RHI::ImageViewCreateInfo& createInfo)
{
    size_t key = HashAny(createInfo);
    if (auto view = m_viewCache.Get(key); view != nullptr)
        return view;

    std::shared_ptr<ImageView> view = std::make_shared<ImageView>(*m_context);
    view->Init(*this, createInfo);
    m_viewCache.Insert(key, view);
    return view;
}

BufferView::~BufferView()
{
    auto& context = static_cast<Context&>(*m_context);
    auto  device  = context.GetDevice();
    device.destroyBufferView(m_handle);
}

RHI::ResultCode BufferView::Init(RHI::Buffer& _buffer, const RHI::BufferViewCreateInfo& createInfo)
{
    auto&   context = static_cast<Context&>(*m_context);
    auto    device  = context.GetDevice();
    Buffer& buffer  = static_cast<Buffer&>(_buffer);

    vk::BufferViewCreateInfo viewCreateInfo {};
    viewCreateInfo.setBuffer(buffer.GetHandle());
    viewCreateInfo.setOffset(createInfo.byteOffset);
    viewCreateInfo.setRange(createInfo.byteSize);
    viewCreateInfo.setFormat(ConvertFormat(createInfo.format));

    m_handle = device.createBufferView(viewCreateInfo).value;

    return RHI::ResultCode::Success;
}

ImageView::~ImageView()
{
    auto& context = static_cast<Context&>(*m_context);
    auto  device  = context.GetDevice();
    device.destroyImageView(m_handle);
}

RHI::ResultCode ImageView::Init(RHI::Image& _image, const RHI::ImageViewCreateInfo& createInfo)
{
    auto& context = static_cast<Context&>(*m_context);
    auto  device  = context.GetDevice();
    auto& image   = static_cast<Image&>(_image);

    vk::ImageSubresourceRange subresourceRange {};
    subresourceRange.setBaseArrayLayer(createInfo.subresource.baseArrayLayer);
    subresourceRange.setLayerCount(createInfo.subresource.layerCount);
    subresourceRange.setBaseMipLevel(createInfo.subresource.mipLevel);
    subresourceRange.setLevelCount(createInfo.subresource.levelsCount);

    vk::ComponentMapping componentMapping {};
    componentMapping.setR(ConvertComponentSwizzle(createInfo.components.r));
    componentMapping.setG(ConvertComponentSwizzle(createInfo.components.g));
    componentMapping.setB(ConvertComponentSwizzle(createInfo.components.b));
    componentMapping.setA(ConvertComponentSwizzle(createInfo.components.a));

    vk::ImageViewCreateInfo viewCreateInfo {};
    viewCreateInfo.setFormat(ConvertFormat(image.GetInfo().format));
    viewCreateInfo.setImage(image.GetHandle());
    viewCreateInfo.setSubresourceRange(subresourceRange);
    viewCreateInfo.setComponents(componentMapping);
    viewCreateInfo.setViewType(GetImageViewType(image.GetInfo().type, false));

    m_handle = device.createImageView(viewCreateInfo).value;

    return RHI::ResultCode::Success;
}

Swapchain::~Swapchain()
{
    Shutdown();
}

RHI::ResultCode Swapchain::Init(const RHI::SwapchainCreateInfo& createInfo)
{
    // clang-format off
    auto& context        = static_cast<Context&>(*m_context);
    auto  device         = context.GetDevice();
    auto  physcialDevice = context.GetPhysicalDevice();

    m_surface = context.CreateSurface(createInfo.nativeWindowHandle);

    vk::SurfaceCapabilities2KHR        surfaceCapabilities = physcialDevice.getSurfaceCapabilities2KHR(m_surface->get()).value;
    std::vector<vk::PresentModeKHR>    presentModes        = physcialDevice.getSurfacePresentModesKHR(m_surface->get()).value;
    std::vector<vk::SurfaceFormat2KHR> surfaceFormats      = physcialDevice.getSurfaceFormats2KHR(m_surface->get()).value;

    // Clamp the min images count
    auto imagesCount = std::clamp(createInfo.imageCount, surfaceCapabilities.surfaceCapabilities.minImageCount, surfaceCapabilities.surfaceCapabilities.maxImageCount);
    auto surfaceFormatResult = std::find_if(surfaceFormats.begin(), surfaceFormats.end(), [createInfo](vk::SurfaceFormat2KHR surfaceFormat) { return surfaceFormat.surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear && surfaceFormat.surfaceFormat.format == ConvertFormat(createInfo.imageFormat); });

    if (surfaceFormatResult == surfaceFormats.end())
        surfaceFormatResult = surfaceFormats.begin();

    vk::PresentModeKHR presentMode = std::find_if(presentModes.begin(), presentModes.end(), [](vk::PresentModeKHR mode) { return mode == vk::PresentModeKHR::eMailbox; }) == presentModes.end() ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;

    vk::Extent2D extent {};
    extent.setWidth(std::clamp(createInfo.imageSize.width, surfaceCapabilities.surfaceCapabilities.minImageExtent.width, surfaceCapabilities.surfaceCapabilities.maxImageExtent.width));
    extent.setHeight(std::clamp(createInfo.imageSize.height, surfaceCapabilities.surfaceCapabilities.minImageExtent.height, surfaceCapabilities.surfaceCapabilities.maxImageExtent.height));

    vk::SwapchainCreateInfoKHR swapchainCreateInfo {};
    swapchainCreateInfo.setSurface(m_surface->get());
    swapchainCreateInfo.setMinImageCount(imagesCount);
    swapchainCreateInfo.setImageFormat(surfaceFormatResult->surfaceFormat.format);
    swapchainCreateInfo.setImageColorSpace(surfaceFormatResult->surfaceFormat.colorSpace);
    swapchainCreateInfo.setImageExtent(extent);
    swapchainCreateInfo.setImageArrayLayers(1);
    swapchainCreateInfo.setImageUsage(GetImageUsageFlags(createInfo.imageUsage));
    swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    swapchainCreateInfo.setQueueFamilyIndexCount(0);
    swapchainCreateInfo.setPQueueFamilyIndices(nullptr);
    swapchainCreateInfo.setPreTransform(surfaceCapabilities.surfaceCapabilities.currentTransform);
    swapchainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    swapchainCreateInfo.setPresentMode(presentMode);
    swapchainCreateInfo.setClipped(VK_FALSE);
    swapchainCreateInfo.setOldSwapchain(VK_NULL_HANDLE);

    m_handle = device.createSwapchainKHR(swapchainCreateInfo).value;

    m_usage = createInfo.imageUsage;
    m_format = createInfo.imageFormat;

    // Acquire all images in the swapchain
    for (auto image : device.getSwapchainImagesKHR(m_handle).value)
        m_images.push_back(std::make_unique<Image>(*m_context, image));

    return RHI::ResultCode::Success;
    // clang-format on
}

void Swapchain::Shutdown()
{
    auto& context = static_cast<Context&>(*m_context);
    auto  device  = context.GetDevice();

    device.destroySwapchainKHR(m_handle);
}

RHI::ResultCode Swapchain::Resize(uint32_t newWidth, uint32_t newHeight)
{
    RHI::SwapchainCreateInfo createInfo {};
    createInfo.imageCount       = static_cast<uint32_t>(m_images.size());
    createInfo.imageFormat      = m_format;
    createInfo.imageUsage       = m_usage;
    createInfo.imageSize.width  = newWidth;
    createInfo.imageSize.height = newHeight;
    createInfo.imageSize.depth  = 1;

    Shutdown();
    Init(createInfo);

    return RHI::ResultCode::Success;
}

RHI::ResultCode Swapchain::SwapImages()
{
    auto& context = static_cast<Context&>(*m_context);
    auto  device  = context.GetDevice();

    // acquire associated attachment.

    return {};
}

Sampler::~Sampler()
{
    auto& context = static_cast<Context&>(*m_context);
    auto  device  = context.GetDevice();
    device.destroySampler(m_handle);
}

RHI::ResultCode Sampler::Init(const RHI::SamplerCreateInfo& createInfo)
{
    auto& context = static_cast<Context&>(*m_context);
    auto  device  = context.GetDevice();

    vk::SamplerCreateInfo samplerCreateInfo {};
    samplerCreateInfo.setMagFilter(GetFilter(createInfo.filter));
    samplerCreateInfo.setAddressModeU(GetSamplerAddressMode(createInfo.addressU));
    samplerCreateInfo.setAddressModeV(GetSamplerAddressMode(createInfo.addressV));
    samplerCreateInfo.setAddressModeW(GetSamplerAddressMode(createInfo.addressW));
    samplerCreateInfo.setMipLodBias(createInfo.mipLodBias);
    samplerCreateInfo.setAnisotropyEnable(createInfo.maxAnisotropy != 0 ? VK_TRUE : VK_FALSE);
    samplerCreateInfo.setMaxAnisotropy(createInfo.maxAnisotropy);
    samplerCreateInfo.setCompareEnable(createInfo.compare != RHI::SamplerCompareOp::None ? VK_TRUE : VK_FALSE);
    samplerCreateInfo.setMinLod(createInfo.minLod);
    samplerCreateInfo.setMinLod(createInfo.maxLod);
    samplerCreateInfo.setBorderColor(vk::BorderColor::eFloatOpaqueBlack);
    samplerCreateInfo.setUnnormalizedCoordinates(VK_FALSE);

    m_handle = device.createSampler(samplerCreateInfo).value;

    return RHI::ResultCode::Success;
}

Fence::~Fence()
{
    auto& context = static_cast<Context&>(*m_context);
    auto  device  = context.GetDevice();
    device.destroyFence(m_handle);
}

RHI::ResultCode Fence::Init()
{
    auto& context = static_cast<Context&>(*m_context);
    auto  device  = context.GetDevice();

    vk::FenceCreateInfo createInfo {};

    m_handle = device.createFence(createInfo).value;
    return RHI::ResultCode::Success;
}

RHI::ResultCode Fence::Reset()
{
    auto&      context = static_cast<Context&>(*m_context);
    auto       device  = context.GetDevice();
    vk::Result result  = device.resetFences(m_handle);
    RHI_ASSERT_MSG(result == vk::Result::eSuccess, "Operation failed");
    return RHI::ResultCode::Success;
}

RHI::ResultCode Fence::Wait() const
{
    auto&      context = static_cast<Context&>(*m_context);
    auto       device  = context.GetDevice();
    vk::Result result  = device.waitForFences(1, &m_handle, VK_TRUE, UINT64_MAX);
    RHI_ASSERT_MSG(result == vk::Result::eSuccess, "Operation failed");
    return RHI::ResultCode::Success;
}

RHI::FenceState Fence::GetState() const
{
    auto& context = static_cast<Context&>(*m_context);
    auto  device  = context.GetDevice();
    return (device.getFenceStatus(m_handle) == vk::Result::eSuccess) ? RHI::FenceState::Signaled : RHI::FenceState::Unsignaled;
}

}  // namespace Vulkan