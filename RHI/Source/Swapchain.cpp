#include "RHI/Swapchain.hpp"

namespace RHI
{
    Swapchain::Swapchain(Device* device)
        : m_device(device)
        , m_name()
        , m_imageUsage()
        , m_imageFormat()
        , m_presentMode()
        , m_imageCount()
        , m_imageIndex()
        , m_image()
    {
    }

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