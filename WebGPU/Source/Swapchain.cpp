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
        switch (alpha)
        {
        case SwapchainAlphaMode::None:
        case SwapchainAlphaMode::PreMultiplied:  return WGPUCompositeAlphaMode_Premultiplied;
        case SwapchainAlphaMode::PostMultiplied: return WGPUCompositeAlphaMode_Unpremultiplied;
        }
        return WGPUCompositeAlphaMode_Force32;
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

    ISwapchain::ISwapchain()  = default;
    ISwapchain::~ISwapchain() = default;

    ResultCode ISwapchain::Init(IDevice* device, const SwapchainCreateInfo& createInfo)
    {
        m_device            = device;
        m_currentCreateInfo = createInfo;

        WGPUSurfaceDescriptor desc{
            .nextInChain = nullptr,
            .label       = ConvertToStringView(createInfo.name),
        };
        m_surface = wgpuInstanceCreateSurface(device->m_instance, &desc);

        return Recreate(createInfo.imageSize);
    }

    void ISwapchain::Shutdown()
    {
        wgpuSurfaceRelease(m_surface);
    }

    ResultCode ISwapchain::Recreate(ImageSize2D newSize)
    {
        m_currentCreateInfo.imageSize = newSize;
        InitIntenral(m_currentCreateInfo);
        return ResultCode::Success;
    }

    ResultCode ISwapchain::Present()
    {
        wgpuSurfacePresent(m_surface);

        WGPUSurfaceTexture currentBackBuffer;
        wgpuSurfaceGetCurrentTexture(m_surface, &currentBackBuffer);

        return ResultCode::Success;
    }

    void ISwapchain::InitIntenral(const SwapchainCreateInfo& createInfo)
    {
#ifdef WIN32
        WGPUSurfaceSourceWindowsHWND windowDesc{
            .chain     = {},
            .hinstance = createInfo.win32Window.hinstance,
            .hwnd      = createInfo.win32Window.hwnd,
        };
#else
    #error "Not implemented yet!"
#endif
        WGPUTextureFormat format = ConvertToTextureFormat(createInfo.imageFormat);
        WGPUChainedStruct nextInChain{
            .next  = (WGPUChainedStruct*)&windowDesc.chain,
            .sType = WGPUSType_SurfaceSourceWindowsHWND,
        };
        WGPUSurfaceConfiguration config{
            .nextInChain     = &nextInChain,
            .device          = m_device->m_device,
            .format          = ConvertToTextureFormat(createInfo.imageFormat),
            .usage           = ConvertToTextureUsage(createInfo.imageUsage),
            .viewFormatCount = 1,
            .viewFormats     = &format,
            .alphaMode       = ConvertToAlphaMode(createInfo.alphaMode),
            .width           = createInfo.imageSize.width,
            .height          = createInfo.imageSize.height,
            .presentMode     = ConvertToPresentMode(createInfo.presentMode),
        };

        wgpuSurfaceConfigure(m_surface, &config);
    }

} // namespace RHI::WebGPU