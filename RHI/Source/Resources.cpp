#include "RHI/Resources.hpp"
#include "RHI/Context.hpp"
#include "RHI/Swapchain.hpp"

#include <TL/Assert.hpp>

namespace RHI
{
    Swapchain::Swapchain(Context* context)
        : m_context(context)
        , m_imageIndex(0)
        , m_imageCount(0)
        , m_images()
    {
    }

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