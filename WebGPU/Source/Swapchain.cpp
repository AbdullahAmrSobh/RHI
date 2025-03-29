#include "Swapchain.hpp"

#include <TL/Log.hpp>

#include <tracy/Tracy.hpp>

#include "Common.hpp"
#include "Device.hpp"
#include "Resources.hpp"

namespace RHI::WebGPU
{
    WGPUCompositeAlphaMode ConvertToAlphaMode(SwapchainAlphaMode alpha)
    {
        return WGPUCompositeAlphaMode_Auto;

        // switch (alpha)
        // {
        // case SwapchainAlphaMode::None:
        // case SwapchainAlphaMode::PreMultiplied:  return WGPUCompositeAlphaMode_Premultiplied;
        // case SwapchainAlphaMode::PostMultiplied: return WGPUCompositeAlphaMode_Unpremultiplied;
        // }
        // return WGPUCompositeAlphaMode_Force32;
    }

    WGPUPresentMode ConvertToPresentMode(SwapchainPresentMode present)
    {
        switch (present)
        {
        case SwapchainPresentMode::Immediate:   return WGPUPresentMode_Immediate;
        case SwapchainPresentMode::Fifo:        return WGPUPresentMode_Fifo;
        case SwapchainPresentMode::FifoRelaxed: return WGPUPresentMode_FifoRelaxed;
        case SwapchainPresentMode::Mailbox:     return WGPUPresentMode_Mailbox;
        }
        return WGPUPresentMode_Force32;
    }

    inline static ResultCode ConvertToResultCode(WGPUSurfaceGetCurrentTextureStatus status)
    {
        switch (status)
        {
        case WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal:    return ResultCode::Success;
        case WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal: return ResultCode::SuccessSuboptimal;
        case WGPUSurfaceGetCurrentTextureStatus_Timeout:           return ResultCode::ErrorTimeout;
        case WGPUSurfaceGetCurrentTextureStatus_Outdated:          return ResultCode::ErrorOutdated;
        case WGPUSurfaceGetCurrentTextureStatus_Lost:              return ResultCode::ErrorSurfaceLost;
        case WGPUSurfaceGetCurrentTextureStatus_Force32:
        case WGPUSurfaceGetCurrentTextureStatus_Error:             return ResultCode::ErrorUnknown;
        }
        TL_UNREACHABLE();
        return ResultCode::ErrorUnknown;
    }

    ISwapchain::ISwapchain()  = default;
    ISwapchain::~ISwapchain() = default;

    ResultCode ISwapchain::Init(IDevice* device, const SwapchainCreateInfo& createInfo)
    {
        m_device = device;

#ifdef WIN32
        WGPUSurfaceSourceWindowsHWND windowDesc{
            .chain     = {.next = nullptr, .sType = WGPUSType_SurfaceSourceWindowsHWND},
            .hinstance = createInfo.win32Window.hinstance,
            .hwnd      = createInfo.win32Window.hwnd,
        };
#else
    #error "Not implemented yet!"
#endif

        WGPUSurfaceDescriptor desc{
            .nextInChain = &windowDesc.chain,
            .label       = ConvertToStringView(createInfo.name),
        };
        m_surface = wgpuInstanceCreateSurface(device->m_instance, &desc);

        IImage image{};
        m_image[0]   = m_device->m_imageOwner.Emplace(std::move(image));
        m_imageIndex = 0;
        m_imageCount = 1;

        m_surfaceTextureFormat = ConvertToTextureFormat(createInfo.imageFormat);
        m_surfaceTextureUsages = ConvertToTextureUsage(createInfo.imageUsage);
        m_presentMode          = ConvertToPresentMode(createInfo.presentMode);
        m_alphaMode            = ConvertToAlphaMode(createInfo.alphaMode);

        return Configure(createInfo.imageSize, m_surfaceTextureFormat, m_surfaceTextureUsages, m_presentMode, m_alphaMode);
    }

    void ISwapchain::Shutdown()
    {
        wgpuSurfaceRelease(m_surface);
    }

    ResultCode ISwapchain::Recreate(ImageSize2D newSize)
    {
        return Configure(newSize, m_surfaceTextureFormat, m_surfaceTextureUsages, m_presentMode, m_alphaMode);
    }

    ResultCode ISwapchain::Present()
    {
        wgpuSurfacePresent(m_surface);
        return SwapBackTextures();
    }

    ResultCode ISwapchain::Configure(ImageSize2D textureSize, WGPUTextureFormat format, WGPUTextureUsage usage, WGPUPresentMode presentMode, WGPUCompositeAlphaMode alphaMode)
    {
        WGPUStatus status = WGPUStatus_Force32;

        WGPUSurfaceCapabilities capabilities;
        status = wgpuSurfaceGetCapabilities(m_surface, m_device->m_adapter, &capabilities);
        if (status != WGPUStatus_Success)
        {
            wgpuSurfaceCapabilitiesFreeMembers(capabilities);
            return ResultCode::ErrorInvalidArguments;
        }
        if (!(capabilities.usages & usage))
        {
            wgpuSurfaceCapabilitiesFreeMembers(capabilities);
            return ResultCode::ErrorInvalidArguments;
        }
        if (!std::find(capabilities.formats, capabilities.formats + capabilities.formatCount, format))
        {
            wgpuSurfaceCapabilitiesFreeMembers(capabilities);
            return ResultCode::ErrorInvalidArguments;
        }
        if (!std::find(capabilities.presentModes, capabilities.presentModes + capabilities.presentModeCount, presentMode))
        {
            wgpuSurfaceCapabilitiesFreeMembers(capabilities);
            return ResultCode::ErrorInvalidArguments;
        }
        if (!std::find(capabilities.alphaModes, capabilities.alphaModes + capabilities.alphaModeCount, alphaMode))
        {
            wgpuSurfaceCapabilitiesFreeMembers(capabilities);
            return ResultCode::ErrorInvalidArguments;
        }
        wgpuSurfaceCapabilitiesFreeMembers(capabilities);

        m_surfaceTextureFormat = format;
        m_surfaceTextureUsages = usage;
        m_presentMode          = presentMode;
        m_alphaMode            = alphaMode;

        WGPUSurfaceConfiguration config{
            .nextInChain     = nullptr,
            .device          = m_device->m_device,
            .format          = m_surfaceTextureFormat,
            .usage           = m_surfaceTextureUsages,
            .viewFormatCount = 1,
            .viewFormats     = &m_surfaceTextureFormat,
            .alphaMode       = m_alphaMode,
            .width           = textureSize.width,
            .height          = textureSize.height,
            .presentMode     = m_presentMode,
        };
        wgpuSurfaceConfigure(m_surface, &config);

        return SwapBackTextures();
    }

    ResultCode ISwapchain::SwapBackTextures()
    {
        WGPUSurfaceTexture surfaceTexture;
        wgpuSurfaceGetCurrentTexture(m_surface, &surfaceTexture);

        WGPUTextureViewDescriptor descriptor{
            .nextInChain     = {},
            .label           = {},
            .format          = m_surfaceTextureFormat,
            .dimension       = WGPUTextureViewDimension_2D,
            .baseMipLevel    = 0,
            .mipLevelCount   = 1,
            .baseArrayLayer  = 0,
            .arrayLayerCount = 1,
            .aspect          = WGPUTextureAspect_All,
            .usage           = m_surfaceTextureUsages,
        };

        auto image = m_device->m_imageOwner.Get(GetImage());
        if (auto oldView = image->view)
        {
            wgpuTextureViewRelease(oldView);
        }
        image->texture = surfaceTexture.texture;
        image->view    = wgpuTextureCreateView(image->texture, &descriptor);
        return ConvertToResultCode(surfaceTexture.status);
    }

} // namespace RHI::WebGPU