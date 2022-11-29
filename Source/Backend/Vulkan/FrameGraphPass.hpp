#pragma once
#include "Backend/Vulkan/Commands.hpp"

#include "RHI/FrameGraphPass.hpp"

namespace RHI
{
namespace Vulkan
{
    class Device;
    class FrameGraph;
    class RenderPass;
    class Framebuffer;
    class Fence;
    class Semaphore;
    class CommandAllocator;

    class Pass final : public IPass
    {
    public:
        Pass(const Device& device, FrameGraph& frameGraph, std::string name, EPassType type)
            : IPass(name, type)
            , m_pDevice(&device)
            , m_pFrameGraph(&frameGraph)
            , m_renderPass(CreateUnique<RenderPass>(*m_pDevice))
            , m_framebuffer(CreateUnique<Framebuffer>(*m_pDevice))
            , m_commandAllocator(CreateUnique<CommandAllocator>(*m_pDevice))
            , m_semaphore(CreateUnique<Semaphore>(*m_pDevice))
        {
        }
        ~Pass() = default;

        VkResult Init();

        const RenderPass& GetRenderPass() const;
        
        const Framebuffer& GetFramebuffer() const;

        const Fence& GetFence() const;

        const Semaphore& GetSemaphore() const;

    private:
        const Device* m_pDevice;
        FrameGraph* m_pFrameGraph;
        
        Unique<RenderPass> m_renderPass;

        Unique<Framebuffer> m_framebuffer;
        
        Unique<CommandAllocator> m_commandAllocator;

        Unique<Semaphore> m_semaphore;
    };

} // namespace Vulkan
} // namespace RHI