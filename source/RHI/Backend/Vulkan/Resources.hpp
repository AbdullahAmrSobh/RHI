#pragma once

#include "RHI/Backend/Vulkan/Vulkan.hpp"
#include "RHI/Resources.hpp"

namespace Vulkan
{

class Context;

class RHI_EXPORT Buffer final
    : public RHI::Buffer
    , public DeviceObject<vk::Buffer>
{
public:
    Buffer(RHI::Context& context)
        : RHI::Buffer(context)
    {
    }

    ~Buffer();

    RHI::ResultCode Init(const RHI::ResourceAllocationInfo& allocationInfo, const RHI::BufferCreateInfo& createInfo) override;

private:
    VmaAllocation     m_allocation;
    VmaAllocationInfo m_allocationInfo;
};

class RHI_EXPORT Image final
    : public RHI::Image
    , public DeviceObject<vk::Image>
{
public:
    Image(RHI::Context& context)
        : RHI::Image(context)
    {
    }

    Image(RHI::Context& context, vk::Image image)
        : RHI::Image(context)
        , DeviceObject(image)
    {
    }

    ~Image();

    RHI::ResultCode Init(const RHI::ResourceAllocationInfo& allocationInfo, const RHI::ImageCreateInfo& createInfo) override;

    bool IsSwapchainImage() const
    {
        return m_allocation == VK_NULL_HANDLE && m_handle;
    }

private:
    VmaAllocation     m_allocation;
    VmaAllocationInfo m_allocationInfo;
};

class RHI_EXPORT BufferView final
    : public RHI::BufferView
    , public DeviceObject<vk::BufferView>
{
public:
    BufferView(RHI::Context& context)
        : RHI::BufferView(context)
    {
    }

    ~BufferView();

    RHI::ResultCode Init(RHI::Buffer& buffer, const RHI::BufferViewCreateInfo& createInfo) override;
};

class RHI_EXPORT ImageView final
    : public RHI::ImageView
    , public DeviceObject<vk::ImageView>
{
public:
    ImageView(RHI::Context& context)
        : RHI::ImageView(context)
    {
    }

    ~ImageView();

    RHI::ResultCode Init(RHI::Image& image, const RHI::ImageViewCreateInfo& createInfo) override;
};

class RHI_EXPORT Swapchain final
    : public RHI::Swapchain
    , public DeviceObject<vk::SwapchainKHR>
{
public:
    Swapchain(RHI::Context& context)
        : RHI::Swapchain(context)
    {
    }

    ~Swapchain();

    RHI::ResultCode Init(const RHI::SwapchainCreateInfo& createInfo) override;

    RHI::ResultCode Resize(uint32_t newWidth, uint32_t newHeight) override;

    RHI::ResultCode SwapImages() override;

protected:
    std::shared_ptr<vk::UniqueSurfaceKHR> m_surface;
};

class RHI_EXPORT Sampler final
    : public RHI::Sampler
    , public DeviceObject<vk::Sampler>
{
public:
    Sampler(RHI::Context& context)
        : RHI::Sampler(context)
    {
    }

    ~Sampler();

    RHI::ResultCode Init(const RHI::SamplerCreateInfo& createInfo) override;
};

class RHI_EXPORT Fence final
    : public RHI::Fence
    , public DeviceObject<vk::Fence>
{
public:
    Fence(RHI::Context& context)
        : RHI::Fence(context)
    {
    }

    ~Fence();

    RHI::ResultCode Init() override;

    RHI::ResultCode Reset() override;
    RHI::ResultCode Wait() const override;
    RHI::FenceState GetState() const override;
};

}  // namespace Vulkan