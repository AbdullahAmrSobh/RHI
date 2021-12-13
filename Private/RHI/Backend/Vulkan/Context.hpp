#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Context.hpp"

namespace RHI
{
namespace Vulkan
{
    class Context final : public IContext
    {
    public:
        Context(Device& device);

        virtual EResultCode Present(uint32_t _swapchainCount, ISwapChain** _ppSwapchains) override;
        
    
    private:
        Device* m_pDevice;
        VkQueue m_PresentQueue;
		
    };
} // namespace Vulkan
} // namespace RHI
