#include "RHI/Resources.hpp"

#include "RHI/Context.hpp"
#include "RHI/Swapchain.hpp"

namespace RHI
{
    Swapchain::Swapchain(Context* context)
        : m_context(context)
        , m_currentImageIndex(0)
        , m_swapchainImagesCount(0)
        , m_images()
    {
    }

    uint32_t Swapchain::GetCurrentImageIndex() const
    {
        return m_currentImageIndex;
    }

    uint32_t Swapchain::GetImagesCount() const
    {
        return m_swapchainImagesCount;
    }

    Handle<Image> Swapchain::GetImage() const
    {
        return m_images[m_currentImageIndex];
    }

    Handle<ImageView> Swapchain::GetImageView(const ImageViewCreateInfo& createInfo)
    {
        if (auto it = m_imageViewsLRU.find(createInfo); it != m_imageViewsLRU.end())
            return it->second;

        auto imageView = m_context->CreateImageView(createInfo);
        m_imageViewsLRU[createInfo] = imageView;
        return imageView;
    }
} // namespace RHI