#pragma once

#include <RHI/Common/Containers.h>
#include <RHI/Common/Span.hpp>
#include <RHI/Common/Handle.hpp>

#include <RHI/RenderGraph.hpp>

#include <vulkan/vulkan.hpp>

#include <optional>

namespace RHI::Vulkan
{
    class ICommandList;

    namespace BarrierSlot
    {
        enum Type
        {
            Priloge,
            Epiloge,
            Resolve,
            Count,
        };
    }

    struct PipelineBarriers
    {
        TL::Vector<VkMemoryBarrier2> memoryBarriers;
        TL::Vector<VkBufferMemoryBarrier2> bufferBarriers;
        TL::Vector<VkImageMemoryBarrier2> imageBarriers;
    };

    class PassExecuteContext
    {
    public:
        PassExecuteContext(RenderGraph& renderGraph, Handle<Pass> pass, TL::Span<const LoadStoreOperations> loadStoreOperations);

        void Begin(ICommandList& commandList);
        void End(ICommandList& commandList);

    private:
        IContext* m_context;

        RenderGraph* m_renderGraph;
        Handle<Pass> m_pass;

        PipelineBarriers m_barriers[BarrierSlot::Count];

        Flags<PassFlags> m_flags;

        TL::Vector<VkRenderingAttachmentInfo> m_colorAttachments;
        std::optional<VkRenderingAttachmentInfo> m_depthAttachment;
        std::optional<VkRenderingAttachmentInfo> m_stencilAttachment;
        VkRect2D m_renderArea;

    public:
        TL::Vector<VkSemaphoreSubmitInfo> m_waitSemaphores;
        TL::Vector<VkSemaphoreSubmitInfo> m_signalSemaphores;
    };
} // namespace RHI::Vulkan