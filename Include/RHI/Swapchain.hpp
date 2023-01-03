#pragma once
#include "RHI/Image.hpp"

namespace RHI
{

// so we don't have to include Windows.h
struct Win32SurfaceDesc
{
    void* hwnd;
    void* instance;
};

class ISurface
{
public:
    virtual ~ISurface() = default;
};

struct SwapchainDesc
{
    ISurface* pSurface;
    Extent2D  extent;
    uint32_t  backImagesCount;
    Format    imageFormat;
};

class ISwapchain
{
public:
    virtual ~ISwapchain() = default;

    IImage& GetCurrentImage() const
    {
        return *m_images[m_currentImageIndex];
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
    ISurface*                   m_pSurface          = nullptr;
    uint32_t                    m_currentImageIndex = 0;
    std::vector<Unique<IImage>> m_images;
    Unique<ImageDesc>           m_imageDescription;
};

}  // namespace RHI
