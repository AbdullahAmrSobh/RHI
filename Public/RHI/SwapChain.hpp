#pragma once
#include "RHI/Fence.hpp"
#include "RHI/Texture.hpp"

namespace RHI
{

struct SwapChainDesc
{
    Extent2D           extent;
    uint32_t           bufferCount;
    TextureUsageFlags  bufferUsage;
    NativeWindowHandle windowHandle;
};

class ISwapChain
{
public:
    virtual ~ISwapChain() = default;

    inline uint32_t             GetCurrentImageIndex() const { return m_currentImageIndex; }
    inline ITexture&            GetCurrentImage() { return *m_images[m_currentImageIndex]; }
    inline ITexture&            GetImageAt(uint32_t index) { return *m_images[index]; }
    inline uint32_t             GetImageCount() const { return m_imageCount; }
    inline ArrayView<ITexture*> GetImages() { return ArrayView<ITexture*>(m_images.begin(), m_images.begin() + m_imageCount); }
    
    virtual EResultCode SwapBuffers() = 0;
    virtual EResultCode Present(IFence& renderFinished)     = 0;

protected:
    constexpr static uint32_t MAX_BACK_BUFFERS = 3;

    uint32_t                                m_imageCount        = 0;
    uint32_t                                m_currentImageIndex = 0;
    std::array<ITexture*, MAX_BACK_BUFFERS> m_images;
};

using SwapChainPtr = Unique<ISwapChain>;

} // namespace RHI
