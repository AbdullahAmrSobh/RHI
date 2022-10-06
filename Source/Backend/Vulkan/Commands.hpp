#pragma once
#include "RHI/Commands.hpp"
#include "Backend/Vulkan/RenderPass.hpp"
#include "Backend/Vulkan/Resource.hpp"

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
        CommandBuffer(Device& device, CommandAllocator* pParantAllocator, VkCommandBuffer handle)
            : DeviceObject(&device, handle)
            , m_pParantAllocator(pParantAllocator)
        {
        }
        
        ~CommandBuffer();

        virtual void Begin() override;
        virtual void End() override;

        virtual void SetViewports(const std::vector<Viewport>& viewports) override;
        virtual void SetScissors(const std::vector<Rect>& scissors) override;

        virtual void Submit(const DrawCommand& drawCommand) override;
        virtual void Submit(const CopyCommand& copyCommand) override;
        virtual void Submit(const DispatchCommand& dispatchCommand) override;

    private:
        class CommandAllocator* m_pParantAllocator;
        const Framebuffer*      m_renderTarget;
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
        CommandAllocator();
        ~CommandAllocator();

        VkResult Init(ECommandPrimaryTask task);

        VkResult Reset();

        Unique<CommandBuffer> AllocateCommandBuffer();

        std::vector<CommandBuffer> AllocateCommandBuffers(uint32_t count);
    };

} // namespace Vulkan
} // namespace RHI