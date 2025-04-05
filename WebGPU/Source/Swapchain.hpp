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

        SurfaceCapabilities GetSurfaceCapabilities() override;
        ResultCode          Resize(ImageSize2D size) override;
        ResultCode          Configure(const SwapchainConfigureInfo& configInfo) override;
        ResultCode          Present() override;

    private:
        ResultCode SwapBackTextures();

        IDevice*               m_device = nullptr;
        WGPUSurface           m_surface = nullptr;
        SwapchainConfigureInfo m_configuration;
    };
} // namespace RHI::WebGPU