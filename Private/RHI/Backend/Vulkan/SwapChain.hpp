#pragma once
#include "RHI/SwapChain.hpp"
#include "RHI/Backend/Vulkan/Surface.hpp"
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/Semaphore.hpp"
#include "RHI/Backend/Vulkan/Image.hpp"

#include <array>

namespace RHI
{
namespace Vulkan
{
    
    class SwapChain final
        : public ISwapChain
        , public DeviceObject<VkSwapchainKHR>
    {
    private:
        friend class Queue;
    
    public:
        SwapChain(Device& _device)
            : DeviceObject(_device)
        {
        }
        ~SwapChain();
        
        VkResult Init(const SwapChainDesc& desc);
        
        virtual EResultCode SwapBuffers() override;
    
    private:
        void ObtainBackBuffers();
    
    private:
        Queue* m_pPresentQueue;
    };

} // namespace Vulkan
} // namespace RHI
