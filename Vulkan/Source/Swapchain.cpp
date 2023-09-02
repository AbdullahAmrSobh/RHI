#include "Swapchain.hpp"

#include "Context.hpp"

namespace Vulkan
{

RHI::Result<Swapchain*> Swapchain::Create(Context* context, const RHI::SwapchainCreateInfo& createInfo)
{
    auto swapchain = new Swapchain(context);
    return {swapchain, RHI::ResultCode::Success};
}

Swapchain::~Swapchain()
{
}

void Swapchain::SetVSync(uint32_t vsync)
{
}

RHI::ResultCode Swapchain::Resize(uint32_t newWidth, uint32_t newHeight)
{
    return {};
}

RHI::ResultCode Swapchain::SetExclusiveFullScreenMode(bool enableFullscreen)
{
    return {};
}

uint32_t Swapchain::GetCurrentImageIndex() const
{
    return {};
}

uint32_t Swapchain::GetImagesCount() const
{
    return {};
}

RHI::Handle<RHI::Image> Swapchain::GetImage()
{
    return {};
}

RHI::ResultCode Swapchain::Present()
{
    return {};
}

}  // namespace Vulkan