#include "RHI/Swapchain.hpp"

namespace RHI
{
    Swapchain::Swapchain(Context* context)
        : m_context(context)
        , m_imageIndex(0)
        , m_imageCount(0)
        , m_images()
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
        return m_images[m_imageIndex];
    }
} // namespace RHI