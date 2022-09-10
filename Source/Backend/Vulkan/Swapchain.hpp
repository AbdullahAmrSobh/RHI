#pragma once
#include "RHI/Device.hpp"
#include "RHI/Swapchain.hpp"

#include <vulkan/vulkan.h>

#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{

    class Surface final
        : public ISurface
        , public Resource<VkSurfaceKHR>
    {
    public:
        ~Surface();
    };

    class Swapchain final
        : public ISwapchain
        , public Resource<VkSwapchainKHR>
    {
    public:
        ~Swapchain();

        // Swap the back buffers
        virtual EResultCode SwapBuffers() override;
        virtual EResultCode Resize(Extent2D newExtent) override;
        virtual EResultCode SetFullscreenExeclusive(bool enable = true) override;
    };

} // namespace Vulkan
} // namespace RHI