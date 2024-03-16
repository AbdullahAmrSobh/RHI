#pragma once

#include <RHI/FrameScheduler.hpp>
#include <RHI/CommandList.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;
    class ICommandList;

    class ICommandListAllocator final : public CommandListAllocator
    {
    public:
        ICommandListAllocator(IContext* context);

        ~ICommandListAllocator();

        VkResult Init();

        void Reset() override;
        CommandList* Allocate(QueueType queueType) override;
        std::vector<CommandList*> Allocate(QueueType queueType, uint32_t count) override;
        void Release(TL::Span<CommandList*> commandLists) override;

    private:
        std::vector<VkCommandBuffer> AllocateCommandBuffers(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level);
        void ReleaseCommandBuffers(VkCommandPool pool, TL::Span<VkCommandBuffer> commandBuffers);

    private:
        IContext* m_context;
        std::vector<VkCommandPool> m_commandPools[uint32_t(QueueType::Count)];
    };

    class ICommandList final : public CommandList
    {
    public:
        ICommandList(IContext* context, VkCommandPool commandPool, VkCommandBuffer commandBuffer);

        void Begin() override;
        void Begin(Pass& pass) override;
        void End() override;
        void SetViewport(const Viewport& viewport) override;
        void SetSicssor(const Scissor& sicssor) override;
        void Draw(const DrawInfo& command) override;
        void Dispatch(const DispatchInfo& command) override;
        void Copy(const BufferCopyInfo& command) override;
        void Copy(const ImageCopyInfo& command) override;
        void Copy(const BufferToImageCopyInfo& command) override;
        void Copy(const ImageToBufferCopyInfo& command) override;
        void DebugMarkerPush(const char* name, const ColorValue& color) override;
        void DebugMarkerPop() override;

        VkCommandBuffer m_commandBuffer;
        VkCommandPool m_commandPool;

        std::vector<VkSemaphoreSubmitInfo> m_signalSemaphores;
        std::vector<VkSemaphoreSubmitInfo> m_waitSemaphores;

    private:
        void BindShaderBindGroups(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<Handle<BindGroup>> bindGroups);

        void RenderingBegin(Pass& pass);

        void RenderingEnd(Pass& pass);

        void PipelineBarrier(TL::Span<VkMemoryBarrier2> memoryBarriers, TL::Span<VkBufferMemoryBarrier2> bufferBarriers, TL::Span<VkImageMemoryBarrier2> imageBarriers);

        void PipelineBarrier(TL::Span<VkBufferMemoryBarrier2> bufferBarriers, TL::Span<VkImageMemoryBarrier2> imageBarriers);

        void PipelineBarrier(TL::Span<VkMemoryBarrier2> memoryBarriers);

        void PipelineBarrier(TL::Span<VkBufferMemoryBarrier2> bufferBarriers);

        void PipelineBarrier(TL::Span<VkImageMemoryBarrier2> imageBarriers);

    private:
        IContext* m_context;
        Pass* m_pass;
    };

    inline void ICommandList::PipelineBarrier(TL::Span<VkBufferMemoryBarrier2> bufferBarriers, TL::Span<VkImageMemoryBarrier2> imageBarriers)
    {
        PipelineBarrier({}, bufferBarriers, imageBarriers);
    }

    inline void ICommandList::PipelineBarrier(TL::Span<VkMemoryBarrier2> memoryBarriers)
    {
        PipelineBarrier(memoryBarriers, {}, {});
    }

    inline void ICommandList::PipelineBarrier(TL::Span<VkBufferMemoryBarrier2> bufferBarriers)
    {
        PipelineBarrier({}, bufferBarriers, {});
    }

    inline void ICommandList::PipelineBarrier(TL::Span<VkImageMemoryBarrier2> imageBarriers)
    {
        PipelineBarrier({}, {}, imageBarriers);
    }

} // namespace RHI::Vulkan