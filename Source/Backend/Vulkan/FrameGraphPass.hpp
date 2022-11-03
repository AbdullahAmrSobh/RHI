#pragma once 
#include "RHI/FrameGraphPass.hpp"
#include "Backend/Vulkan/RenderPass.hpp"

namespace RHI 
{
namespace Vulkan 
{
    class CommandAllocator;
    class CommandBuffer;
    
    class Pass final : public IPass 
    {
    public:
        Pass(const Device& device, std::string name, EPassType passType);
        ~Pass();
        
        VkResult Init();

        const RenderPass* GetRenderPass() const; 
        
        const Framebuffer* GetFramebuffer() const; 

    private:
        const Device* m_pDevice;
        
        Unique<RenderPass> m_renderPass;
        
        Unique<Framebuffer> m_framebuffer;
        
        Unique<CommandAllocator> m_commandAllocator; 
    
        std::vector<Semaphore*> m_signalSemaphores; 
        
        std::vector<Semaphore*> m_waitSemaphores; 
    
    };

}
}