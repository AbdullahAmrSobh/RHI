#pragma once
#include "RHI/Fence.hpp"
#include "RHI/Image.hpp"

namespace RHI
{

struct SwapChainDesc
{
    SwapChainDesc() = default;

    Extent2D           extent;
    uint32_t           bufferCount;
    ImageUsageFlags    bufferUsage;
    NativeWindowHandle windowHandle;
};

class ISwapChain
{
public:
    virtual ~ISwapChain() = default;

    inline uint32_t GetCurrentImageIndex() const
    {
        return m_currentImageIndex;
    }

    inline IImage& GetCurrentImage()
    {
        return *m_images[m_currentImageIndex];
    }

    inline IImage& GetImageAt(uint32_t index)
    {
        return *m_images[index];
    }

    inline uint32_t GetImageCount() const
    {
        return CountElements(m_images);
    }

    inline std::vector<Unique<IImage>>& GetImages()
    {
        return m_images;
    }

    inline EPixelFormat GetImagePixelFormat() const
    {
        return m_imagePixelFormat;
    };

    virtual void        Resize(Extent2D newExtent) = 0;
    virtual EResultCode SwapBuffers()              = 0;

    virtual struct SwapChainAttachmentId GetSwapChainAttachmentId() const;

protected:
    EPixelFormat                m_imagePixelFormat;
    uint32_t                    m_currentImageIndex = 0;
    std::vector<Unique<IImage>> m_images;
};
using SwapChainPtr = Unique<ISwapChain>;

} // namespace RHI
