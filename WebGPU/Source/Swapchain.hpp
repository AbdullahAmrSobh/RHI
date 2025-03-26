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
        void InitIntenral(const SwapchainCreateInfo& createInfo);

    private:
        IDevice*            m_device;
        WGPUSurface         m_surface;
        SwapchainCreateInfo m_currentCreateInfo;

        Handle<Image> m_currentBackbufferTexture;
    };
} // namespace RHI::WebGPU