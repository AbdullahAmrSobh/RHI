#pragma once
#include "RHI/Common.hpp"
#include "RHI/FrameGraphPass.hpp"
#include "Backend/Vulkan/RenderPass.hpp"
#include "Backend/Vulkan/Commands.hpp"

namespace RHI
{
namespace Vulkan
{
    class CommandAllocator;
    class CommandBuffer;

    class RenderPass;
    class Framebuffer;

    class Pass final : public IPass
    {
    public:
        Pass(const Device& device, std::string name, EPassType type)
            : IPass(name, type)
        {
        }
        
        ~Pass();

        VkResult Init();

        inline const RenderPass* GetRenderPass() const
        {
            return m_renderPass.get();
        }

        inline const Framebuffer* GetFramebuffer() const
        {
            return m_framebuffer.get();
        }

        ICommandBuffer* GetCurrentCommandBuffer() const;

        const Semaphore& GetPassFinishedSemaphore() const;  
    
    private:
        const Device* m_pDevice;
        
        Unique<RenderPass> m_renderPass;

        Unique<Framebuffer> m_framebuffer;
        
        Unique<CommandAllocator> m_commandAllocator;

        Unique<Semaphore> m_passFinishedSemaphore;
    };

} // namespace Vulkan
} // namespace RHI