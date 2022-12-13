#pragma once
#include "RHI/Surface.hpp"

namespace RHI
{

enum class EFormat;
struct ImageDesc;
class IImage;

struct SwapchainDesc
{
    ISurface* pSurface;
    Extent2D  extent;
    uint32_t  backImagesCount;
    EFormat   imageFormat;
};

class ISwapchain
{
public:
    virtual ~ISwapchain() = default;

    IImage& GetCurrentImage() const
    {
        return *m_images[m_currentImageIndex];
    }

    const std::vector<IImage*>& GetImages() const
    {
        return m_images;
    }

    const ImageDesc& GetImageDescription() const
    {
        return *m_imageDescription;
    }

    uint32_t GetCurrentImageIndex() const
    {
        return m_currentImageIndex;
    }

    uint32_t GetImagesCount() const
    {
        return CountElements(m_images);
    }

    virtual void SwapImages() = 0;

protected:
    ISurface*            m_pSurface          = nullptr;
    uint32_t             m_currentImageIndex = 0;
    std::vector<IImage*> m_images;
    Unique<ImageDesc>    m_imageDescription;
};

}  // namespace RHI
