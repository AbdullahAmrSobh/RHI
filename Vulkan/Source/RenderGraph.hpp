#pragma once

#include <RHI/RenderGraph.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;
    class ICompiledPass;
    class ICommandList;

    enum class BarrierSlot
    {
        Prilogue,
        Epilogue,
        Resolve,
        Count,
    };

    struct BarrierStage
    {
        VkPipelineStageFlags2 stageMask        = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2        accessMask       = VK_ACCESS_2_NONE;
        VkImageLayout         layout           = VK_IMAGE_LAYOUT_UNDEFINED;
        uint32_t              queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    };

    class CompiledRenderGraphExecuteGroup final : public RenderGraphExecuteGroup
    {
    public:
        TL::Flags<PipelineStage> GetSignalStage() const { return m_signalStages; }

        TL::Span<const VkSemaphoreSubmitInfo> GetWaitSemaphores() const { return m_waitSemaphores; }

        TL::Span<const VkSemaphoreSubmitInfo> GetSignalSemaphores() const { return m_signalSemaphores; }

    private:
        TL::Flags<PipelineStage>                          m_signalStages;
        TL::Vector<VkSemaphoreSubmitInfo, TL::IAllocator> m_waitSemaphores;
        TL::Vector<VkSemaphoreSubmitInfo, TL::IAllocator> m_signalSemaphores;
    };

    class CompiledPass final : public Pass
    {
    public:
        ~CompiledPass();

        TL::Span<const VkMemoryBarrier2> GetMemoryBarriers(BarrierSlot slot) const { return m_memoryBarriers[(int)slot]; }

        TL::Span<const VkImageMemoryBarrier2> GetImageMemoryBarriers(BarrierSlot slot) const { return m_imageMemoryBarriers[(int)slot]; }

        TL::Span<const VkBufferMemoryBarrier2> GetBufferMemoryBarriers(BarrierSlot slot) const { return m_bufferMemoryBarriers[(int)slot]; }

        void PushPassBarrier(BarrierSlot slot, VkMemoryBarrier2&& barrier) { m_memoryBarriers[(int)slot].emplace_back(barrier); }

        void PushPassBarrier(BarrierSlot slot, VkImageMemoryBarrier2&& barrier) { m_imageMemoryBarriers[(int)slot].emplace_back(barrier); }

        void PushPassBarrier(BarrierSlot slot, VkBufferMemoryBarrier2&& barrier) { m_bufferMemoryBarriers[(int)slot].emplace_back(barrier); }

    private:
        TL::Vector<VkMemoryBarrier2, TL::IAllocator>       m_memoryBarriers[(int)BarrierSlot::Count];
        TL::Vector<VkImageMemoryBarrier2, TL::IAllocator>  m_imageMemoryBarriers[(int)BarrierSlot::Count];
        TL::Vector<VkBufferMemoryBarrier2, TL::IAllocator> m_bufferMemoryBarriers[(int)BarrierSlot::Count];
    };

    class IRenderGraph final : public RenderGraph
    {
    public:
        IRenderGraph();
        ~IRenderGraph();

        ResultCode Init(IDevice* device);
        void       Shutdown();

        void OnGraphExecutionBegin() override;
        void OnGraphExecutionEnd() override;

        void ExecutePassGroup(const RenderGraphExecuteGroup& executeGroup, QueueType queueType) override;

    private:
        VkImageSubresourceRange GetAccessedSubresourceRange(const RenderGraphResourceTransition& accessedResource);

        BarrierStage GetBarrierStage(const RenderGraphResourceTransition* accessedResource);

        void EmitBarriers(ICommandList& commandList, Pass& pass, BarrierSlot slot);
    };

} // namespace RHI::Vulkan