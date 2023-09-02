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

    uint32_t GetCurrentImageIndex() const override;

    uint32_t GetImagesCount() const override;

    RHI::Handle<RHI::Image> GetImage() override;

    RHI::ResultCode Present() override;

private:
    uint32_t m_currentImageIndex;

    uint32_t m_imagesCount;

    std::vector<RHI::Handle<RHI::Image>> m_images;
};

}  // namespace Vulkan
