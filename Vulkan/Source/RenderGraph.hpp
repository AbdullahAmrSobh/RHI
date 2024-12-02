#pragma once

#include <RHI/RenderGraph.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;
    class ICompiledPass;
    class ICommandList;

    enum BarrierSlot
    {
        BarrierSlot_Prilogue,
        BarrierSlot_Epilogue,
        BarrierSlot_Resolve,
        BarrierSlot_Count,
    };

    struct BarrierStage
    {
        VkPipelineStageFlags2 stageMask        = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2        accessMask       = VK_ACCESS_2_NONE;
        VkImageLayout         layout           = VK_IMAGE_LAYOUT_UNDEFINED;
        uint32_t              queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    };

    class IRenderGraph final : public RenderGraph
    {
    public:
        IRenderGraph();
        ~IRenderGraph();

        ResultCode Init(IDevice* device);
        void       Shutdown();

        void OnGraphExecutionBegin() override;
        void OnGraphExecutionEnd()   override;

        void ExecutePassGroup(const PassGroup& passGroup, QueueType queueType) override;

    private:
        VkImageSubresourceRange GetAccessedSubresourceRange(const PassAccessedResource& accessedResource);

        BarrierStage            GetBarrierStage(const PassAccessedResource* accessedResource);

        void EmitBarriers(ICommandList& commandList, Pass& pass, BarrierSlot slot);

    private:
        std::atomic_uint64_t m_asyncQueuesTimelineValues[AsyncQueuesCount];
    };

} // namespace RHI::Vulkan