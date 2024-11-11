#include "RHI/Swapchain.hpp"

namespace RHI
{

    Swapchain::Swapchain()  = default;

    Swapchain::~Swapchain() = default;

    uint32_t Swapchain::GetCurrentImageIndex() const
    {
        return m_imageIndex;
    }

    uint32_t Swapchain::GetImagesCount() const
    {
        return m_imageCount;
    }

    Handle<Image> Swapchain::GetImage() const
    {
        return m_image[GetCurrentImageIndex()];
    }

} // namespace RHI