#pragma once
#include "RHI/Common.hpp"
#include "RHI/Texture.hpp"

#include <array>

namespace RHI
{

using NativeWindowHandle = void*;

struct SwapChainDesc
{
    Extent2D          extent;
    uint32_t          bufferCount;
    TextureUsageFlags bufferUsage;
};

class ISwapChain
{
public:
    virtual ~ISwapChain() = default;
    
    inline uint32_t   GetImageCount() const { return m_imageCount; }
    inline uint32_t   GetCurrentImageIndex() const { return m_currentImageIndex; }
    inline ITexture*  GetCurrentImage() { return m_images[m_currentImageIndex]; }
    inline ITexture*  GetImageAt(uint32_t index) { return m_images[index]; }
    inline ITexture** GetImages() { return m_images.data(); }
    
    virtual EResultCode SwapBuffers() = 0;
    virtual EResultCode Present() = 0;

protected:
    constexpr static uint32_t MAX_BACK_BUFFERS = 3;
    
    uint32_t                                m_imageCount        = 0;
    uint32_t                                m_currentImageIndex = 0;
    std::array<ITexture*, MAX_BACK_BUFFERS> m_images;
};

using SwapChainPtr = Unique<ISwapChain>;

} // namespace RHI
