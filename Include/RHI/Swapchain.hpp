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

    inline const std::vector<IImage*>& GetBackImages() const
    {
        return m_backBuffers;
    }

    inline const ImageDesc& GetBackBuffersDesc() const
    {
        return *m_backBuffersDesc;
    }

    inline uint32_t GetCurrentBackBufferIndex() const
    {
        return m_currentImageIndex;
    }

    virtual EResultCode SwapBuffers()                               = 0;
    virtual EResultCode Resize(Extent2D newExtent)                  = 0;
    virtual EResultCode SetFullscreenExeclusive(bool enable = true) = 0;

protected:
    ISurface*               m_pSurface          = nullptr;
    uint32_t                m_currentImageIndex = 0;
    std::vector<IImage*>    m_backBuffers;
    Unique<const ImageDesc> m_backBuffersDesc;
};

} // namespace RHI
