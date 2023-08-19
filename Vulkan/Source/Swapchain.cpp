#include "Swapchain.hpp"

namespace Vulkan
{

Result<Swapchain*> Swapchain::Create(Context* context, const SwapchainCreateInfo& createInfo)
{
}

Swapchain::~Swapchain()
{
}

void Swapchain::SetVSync(uint32_t vsync)
{
}

ResultCode Swapchain::Resize(uint32_t newWidth, uint32_t newHeight)
{
}

ResultCode Swapchain::SetExclusiveFullScreenMode(bool enable_fullscreen)
{
}

uint32_t Swapchain::GetCurrentImageIndex() const
{
}

uint32_t Swapchain::GetImageCount() const
{
}

Image& Swapchain::GetCurrentImage()
{
}

ResultCode Swapchain::Present() override
{
}

}  // namespace Vulkan