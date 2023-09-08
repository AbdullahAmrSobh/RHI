#pragma once

#include <RHI/Swapchain.hpp>

namespace Vulkan
{

class Context;

class Swapchain final : public RHI::Swapchain
{
public:
    static RHI::Result<Swapchain*> Create(Context* context, const RHI::SwapchainCreateInfo& createInfo);

    using RHI::Swapchain::Swapchain;
    ~Swapchain();

    void SetVSync(uint32_t vsync) override;

    RHI::ResultCode Resize(uint32_t newWidth, uint32_t newHeight) override;

    RHI::ResultCode SetExclusiveFullScreenMode(bool enable_fullscreen) override;

    RHI::ResultCode Present() override;
};

}  // namespace Vulkan
