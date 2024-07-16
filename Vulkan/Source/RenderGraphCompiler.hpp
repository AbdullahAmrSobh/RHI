#pragma once
#include <RHI/RenderGraph.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;

    namespace BarrierType
    {
        enum T
        {
            PrePass,
            PostPass,
            Count,
        };
    }

    struct IPassSubmitData : PassSubmitData
    {
        IPassSubmitData() = default;

        TL::Vector<VkRenderingAttachmentInfo> colorAttachments;
        VkRenderingAttachmentInfo depthAttachmentInfo, stencilAttachmentInfo;
        bool hasDepthAttachemnt, hasStencilAttachment;

        TL::Vector<VkFormat> colorFormats;
        VkFormat depthFormat;
        VkFormat stencilformat;

        TL::Vector<VkImageMemoryBarrier2> imageBarriers[BarrierType::Count];
        TL::Vector<VkBufferMemoryBarrier2> bufferBarriers[BarrierType::Count];

        TL::UnorderedMap<VkSemaphore, VkPipelineStageFlags2> waitSemaphores;
        TL::UnorderedMap<VkSemaphore, VkPipelineStageFlags2> signalSemaphores;

        VkSemaphore signalSemaphore; // semaphore which will be signaled when the pass is finished.


        VkRect2D renderArea;

        inline void Clear()
        {
            colorAttachments.clear();
            hasDepthAttachemnt = false;
            hasStencilAttachment = false;

            colorFormats.clear();
            depthFormat = VK_FORMAT_UNDEFINED;
            stencilformat = VK_FORMAT_UNDEFINED;

            imageBarriers[BarrierType::PrePass].clear();
            imageBarriers[BarrierType::PostPass].clear();
            bufferBarriers[BarrierType::PrePass].clear();
            bufferBarriers[BarrierType::PostPass].clear();

            waitSemaphores.clear();
            signalSemaphores.clear();
        }
    };

    class RenderGraphCompiler
    {
    public:
        static void CompilePass(IContext* context, RenderGraph& renderGraph, Pass* pass);

        static VkRenderingAttachmentInfo GetAttachmentInfo(IContext* context, RenderGraph& renderGraph, Handle<ImageAttachment> attachmentHandle);

        static TL::Vector<VkImageMemoryBarrier2> GetPrepassImageBarrier(
            IContext* context,
            RenderGraph& renderGraph,
            TL::Span<Handle<ImageAttachment>> attachments,
            TL::UnorderedMap<VkSemaphore, VkPipelineStageFlags2>& waitSemaphores);

        static TL::Vector<VkImageMemoryBarrier2> GetPostpassImageBarrier(
            IContext* context,
            RenderGraph& renderGraph,
            TL::Span<Handle<ImageAttachment>> attachments,
            TL::UnorderedMap<VkSemaphore, VkPipelineStageFlags2>& signalSemaphores);

        static TL::Vector<VkBufferMemoryBarrier2> GetPrepassBufferBarrier(
            IContext* context,
            RenderGraph& renderGraph,
            TL::Span<Handle<BufferAttachment>> attachments,
            TL::UnorderedMap<VkSemaphore, VkPipelineStageFlags2>& waitSemaphores);

        static TL::Vector<VkBufferMemoryBarrier2> GetPostpassBufferBarrier(
            IContext* context,
            RenderGraph& renderGraph,
            TL::Span<Handle<BufferAttachment>> attachments,
            TL::UnorderedMap<VkSemaphore, VkPipelineStageFlags2>& signalSemaphores);
    };
} // namespace RHI::Vulkan