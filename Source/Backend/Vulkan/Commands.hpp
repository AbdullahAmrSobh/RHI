#pragma once
#include <vector>
#include "RHI/Commands.hpp"
#include "RHI/Common.hpp"
#include "Backend/Vulkan/RenderPass.hpp"
#include "Backend/Vulkan/Resource.hpp"
#include "Backend/Vulkan/ShaderResourceGroup.hpp"
#include <vulkan/vulkan_core.h>

namespace RHI
{
namespace Vulkan
{
    class Semaphore;
    class CommandAllocator;

    class CommandBuffer final
        : public ICommandBuffer
        , public DeviceObject<VkCommandBuffer>
    {
    public:
        CommandBuffer(const Device& device, CommandAllocator* pParantAllocator, VkCommandBuffer handle)
            : DeviceObject(&device, handle)
            , m_pParantAllocator(pParantAllocator)
        {
        }

        ~CommandBuffer();

        ///////////////////////////////////////////////////////////////////////////////
        // Interface.
        ///////////////////////////////////////////////////////////////////////////////
        virtual void Begin() override;
        virtual void End() override;

        virtual void SetViewports(const std::vector<Viewport>& viewports) override;
        virtual void SetScissors(const std::vector<Rect>& scissors) override;

        virtual void Submit(const DrawCommand& drawCommand) override;
        virtual void Submit(const CopyCommand& copyCommand) override;
        virtual void Submit(const DispatchCommand& dispatchCommand) override;
        ///////////////////////////////////////////////////////////////////////////////

        inline void SetFramebuffer(const Framebuffer& framebuffer);
        
        // Method for waiting on a stage (sync point), for other commands to finish.
        // Method for signaling to other commands, that this command finished a sage.

    private:
        void BeginRenderPass(Extent2D extent, std::vector<VkClearValue> clearValues);
        void EndRenderPass();
        void BindShaderResourceGroups(const std::vector<ShaderResourceGroup*>& groups);
    
    private:
        const Framebuffer* m_renderTarget;
        CommandAllocator*  m_pParantAllocator;
        
        std::vector<VkSemaphoreSubmitInfo*> m_waitSemaphores;
        std::vector<VkSemaphoreSubmitInfo*> m_signalSemaphores;
    };
    
    enum class ECommandPrimaryTask
    {
        Graphics,
        Compute,
        Transfer,
    };
    
    class CommandAllocator final : public DeviceObject<VkCommandPool>
    {
    public:
        static Result<Unique<CommandAllocator>> Create(const Device& device);

        CommandAllocator(const Device& device)
            : DeviceObject(&device)
        {
        }

        ~CommandAllocator();

        VkResult Init(ECommandPrimaryTask task);

        VkResult Reset();
        
        Unique<CommandBuffer> AllocateCommandBuffer();
        
        std::vector<CommandBuffer> AllocateCommandBuffers(uint32_t count);
    };

} // namespace Vulkan
} // namespace RHI