#include "RHI/Resources.hpp"

#include "RHI/Context.hpp"
#include "RHI/Swapchain.hpp"

namespace RHI
{
    Handle<ImageView> Swapchain::GetImageView(const ImageViewCreateInfo& createInfo)
    {
        if (auto it = m_imageViewsLRU.find(createInfo); it != m_imageViewsLRU.end())
            return it->second;

        auto imageView = m_context->CreateImageView(createInfo);
        m_imageViewsLRU[createInfo] = imageView;
        return imageView;
    }
} // namespace RHI