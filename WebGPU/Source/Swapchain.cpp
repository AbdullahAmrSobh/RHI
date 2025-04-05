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
        case SwapchainAlphaMode::None:           return WGPUCompositeAlphaMode_Auto;
        case SwapchainAlphaMode::PreMultiplied:  return WGPUCompositeAlphaMode_Premultiplied;
        case SwapchainAlphaMode::PostMultiplied: return WGPUCompositeAlphaMode_Unpremultiplied;
        }
        return WGPUCompositeAlphaMode_Auto;
    }

    WGPUPresentMode ConvertToPresentMode(SwapchainPresentMode present)
    {
        switch (present)
        {
        case SwapchainPresentMode::Immediate:   return WGPUPresentMode_Immediate;
        case SwapchainPresentMode::Fifo:        return WGPUPresentMode_Fifo;
        case SwapchainPresentMode::FifoRelaxed: return WGPUPresentMode_FifoRelaxed;
        case SwapchainPresentMode::Mailbox:     return WGPUPresentMode_Mailbox;
        default:                                TL_UNREACHABLE(); break;
        }
        return WGPUPresentMode_Fifo;
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
        if (!m_surface)
        {
            return ResultCode::ErrorSurfaceLost;
        }

        // Initialize empty image handle
        IImage image{};
        m_image      = m_device->m_imageOwner.Emplace(std::move(image));
        m_imageCount = 1;

        return ResultCode::Success;
    }

    void ISwapchain::Shutdown()
    {
        if (m_surface)
        {
            wgpuSurfaceRelease(m_surface);
            m_surface = nullptr;
        }

        if (auto image = m_device->m_imageOwner.Get(m_image))
        {
            if (image->view)
            {
                wgpuTextureViewRelease(image->view);
                image->view = nullptr;
            }
            if (image->texture)
            {
                wgpuTextureRelease(image->texture);
                image->texture = nullptr;
            }
        }

        m_device = nullptr;
    }

    SurfaceCapabilities ISwapchain::GetSurfaceCapabilities()
    {
        WGPUSurfaceCapabilities capabilities;
        wgpuSurfaceGetCapabilities(m_surface, m_device->m_adapter, &capabilities);

        SurfaceCapabilities outCaps{};

        /// @fixme: Dont hardcode values here!
        outCaps.usages |= ImageUsage::Color;
        outCaps.usages |= ImageUsage::CopyDst;
        outCaps.formats.push_back(Format::RGBA8_UNORM);

        for (size_t i = 0; i < capabilities.presentModeCount; i++)
        {
            switch (capabilities.presentModes[i])
            {
            case WGPUPresentMode_Immediate:   outCaps.presentModes |= SwapchainPresentMode::Immediate; break;
            case WGPUPresentMode_Mailbox:     outCaps.presentModes |= SwapchainPresentMode::Mailbox; break;
            case WGPUPresentMode_Fifo:        outCaps.presentModes |= SwapchainPresentMode::Fifo; break;
            case WGPUPresentMode_FifoRelaxed: outCaps.presentModes |= SwapchainPresentMode::FifoRelaxed; break;
            default:                          TL_UNREACHABLE(); break;
            }
        }

        for (size_t i = 0; i < capabilities.alphaModeCount; i++)
        {
            switch (capabilities.alphaModes[i])
            {
            case WGPUCompositeAlphaMode_Auto:            outCaps.alphaModes |= SwapchainAlphaMode::None; break;
            case WGPUCompositeAlphaMode_Premultiplied:   outCaps.alphaModes |= SwapchainAlphaMode::PreMultiplied; break;
            case WGPUCompositeAlphaMode_Unpremultiplied: outCaps.alphaModes |= SwapchainAlphaMode::PostMultiplied; break;
            default:                                     TL_UNREACHABLE(); break;
            }
        }

        wgpuSurfaceCapabilitiesFreeMembers(capabilities);
        return outCaps;
    }

    ResultCode ISwapchain::Configure(const SwapchainConfigureInfo& configInfo)
    {
        m_configuration = configInfo;

        WGPUSurfaceCapabilities capabilities;
        auto                    status = wgpuSurfaceGetCapabilities(m_surface, m_device->m_adapter, &capabilities);
        if (status != WGPUStatus_Success)
        {
            wgpuSurfaceCapabilitiesFreeMembers(capabilities);
            return ResultCode::ErrorInvalidArguments;
        }

        auto usage       = ConvertToTextureUsage(configInfo.imageUsage);
        auto format      = ConvertToTextureFormat(configInfo.format);
        auto presentMode = ConvertToPresentMode(configInfo.presentMode);
        auto alphaMode   = ConvertToAlphaMode(configInfo.alphaMode);

        if (!(capabilities.usages & usage) ||
            !std::find(capabilities.formats, capabilities.formats + capabilities.formatCount, format) ||
            !std::find(capabilities.presentModes, capabilities.presentModes + capabilities.presentModeCount, presentMode) ||
            !std::find(capabilities.alphaModes, capabilities.alphaModes + capabilities.alphaModeCount, alphaMode))
        {
            wgpuSurfaceCapabilitiesFreeMembers(capabilities);
            return ResultCode::ErrorInvalidArguments;
        }

        wgpuSurfaceCapabilitiesFreeMembers(capabilities);

        WGPUSurfaceConfiguration config{
            .nextInChain     = nullptr,
            .device          = m_device->m_device,
            .format          = format,
            .usage           = usage,
            .viewFormatCount = 1,
            .viewFormats     = &format,
            .alphaMode       = alphaMode,
            .width           = configInfo.size.width,
            .height          = configInfo.size.height,
            .presentMode     = presentMode,
        };

        wgpuSurfaceConfigure(m_surface, &config);
        return SwapBackTextures();
    }

    ResultCode ISwapchain::Resize(ImageSize2D size)
    {
        m_configuration.size = size;
        return Configure(m_configuration);
    }

    ResultCode ISwapchain::Present()
    {
        wgpuSurfacePresent(m_surface);
        return SwapBackTextures();
    }

    ResultCode ISwapchain::SwapBackTextures()
    {
        WGPUSurfaceTexture surfaceTexture;
        wgpuSurfaceGetCurrentTexture(m_surface, &surfaceTexture);

        auto status = surfaceTexture.status;
        if (status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal)
        {
            return ConvertToResultCode(status);
        }

        auto image = m_device->m_imageOwner.Get(m_image);
        if (image->view)
        {
            wgpuTextureViewRelease(image->view);
        }
        if (image->texture)
        {
            wgpuTextureRelease(image->texture);
        }

        image->texture = surfaceTexture.texture;

        WGPUTextureViewDescriptor viewDesc{
            .format          = ConvertToTextureFormat(m_configuration.format),
            .dimension       = WGPUTextureViewDimension_2D,
            .baseMipLevel    = 0,
            .mipLevelCount   = 1,
            .baseArrayLayer  = 0,
            .arrayLayerCount = 1,
            .aspect          = WGPUTextureAspect_All};

        image->view = wgpuTextureCreateView(image->texture, &viewDesc);
        return ResultCode::Success;
    }

} // namespace RHI::WebGPU