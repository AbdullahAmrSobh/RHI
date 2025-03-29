#pragma once

#include <RHI/Swapchain.hpp>

#include <webgpu/webgpu.h>
#include <webgpu/webgpu_glfw.h>

namespace RHI::WebGPU
{
    class IDevice;

    class ISwapchain final : public Swapchain
    {
    public:
        ISwapchain();
        ~ISwapchain();

        ResultCode Init(IDevice* device, const SwapchainCreateInfo& createInfo);
        void       Shutdown();
        ResultCode Recreate(ImageSize2D newSize) override;
        ResultCode Present() override;

    private:
        ResultCode Configure(ImageSize2D textureSize, WGPUTextureFormat format, WGPUTextureUsage usage, WGPUPresentMode presentMode, WGPUCompositeAlphaMode alphaMode);

        ResultCode SwapBackTextures();

    private:
        IDevice*               m_device;
        WGPUSurface            m_surface;
        WGPUTextureFormat      m_surfaceTextureFormat;
        WGPUTextureUsage       m_surfaceTextureUsages;
        WGPUPresentMode        m_presentMode;
        WGPUCompositeAlphaMode m_alphaMode;
    };
} // namespace RHI::WebGPU