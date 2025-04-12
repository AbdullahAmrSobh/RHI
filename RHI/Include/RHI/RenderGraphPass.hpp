#pragma once

#include "RHI/Common.hpp"
#include "RHI/Export.hpp"
#include "RHI/RenderGraphResources.hpp"
#include "RHI/CommandList.hpp"

#include <TL/Containers.hpp>
#include <TL/Memory.hpp>

namespace RHI
{
    class Device;
    class Pass;
    class RenderGraph;
    class CommandList;
    class Swapchain;
    class RenderGraphResource;
    class RenderGraphExecuteGroup;

    using PassSetupCallback   = TL::Function<void(RenderGraph& renderGraph, Pass& pass)>;
    using PassCompileCallback = TL::Function<void(RenderGraph& renderGraph, Pass& pass)>;
    using PassExecuteCallback = TL::Function<void(CommandList& commandList)>;

    struct PassCreateInfo
    {
        const char*         name;
        QueueType           queue;
        ImageSize2D         size;
        PassSetupCallback   setupCallback;
        PassCompileCallback compileCallback;
        PassExecuteCallback executeCallback;
    };

    enum BarrierSlot
    {
        BarrierSlot_None,
        BarrierSlot_Epilogue,
        BarrierSlot_Prilogue,
        BarrierSlot_Resolve,
        BarrierSlot_Count,
    };

    class RHI_EXPORT Pass
    {
        friend class RenderGraph;

    public:
        Pass(const PassCreateInfo& createInfo, TL::IAllocator* allocator);
        ~Pass();

        TL::Span<GraphTransition* const>       GetTransitions() const;
        TL::Span<ImageGraphTransition* const>  GetImageTransitions() const;
        TL::Span<BufferGraphTransition* const> GetBufferTransitions() const;

        void                                   Execute(CommandList& commandList);

        void                                   UseResource(RenderGraphImage& resource, ImageSubresourceRange subresourceRange, ImageUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access);

        void                                   UseResource(RenderGraphBuffer& resource, BufferSubregion subregion, BufferUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access);

        void                                   PresentSwapchain(RenderGraphImage& resource);

        void                                   AddRenderTarget(const ColorRGAttachment& attachment);

        void                                   AddRenderTarget(const DepthStencilRGAttachment& attachment);

    private:
        TL::Span<const BarrierInfo>       GetMemoryBarriers(BarrierSlot slot) const;
        TL::Span<const ImageBarrierInfo>  GetImageBarriers(BarrierSlot slot) const;
        TL::Span<const BufferBarrierInfo> GetBufferBarriers(BarrierSlot slot) const;

        void                              PrepareBarriers();

        RenderGraphExecuteGroup*          GetExecuteGroup() const { return m_group; }

        void                              SetExecuteGroup(RenderGraphExecuteGroup* group) { m_group = group; }

    private:
        TL::IAllocator*                    m_allocator;
        const char*                        m_name;
        QueueType                          m_queueType;
        ImageSize2D                        m_size;
        PassSetupCallback                  m_onSetupCallback;
        PassCompileCallback                m_onCompileCallback;
        PassExecuteCallback                m_onExecuteCallback;

        TL::Vector<GraphTransition*>       m_transitions;
        TL::Vector<ImageGraphTransition*>  m_imageTransitions;
        TL::Vector<BufferGraphTransition*> m_bufferTransitions;

        struct RenderPass
        {
            ImageSize2D                            m_size;
            TL::Vector<ColorRGAttachment>          m_colorAttachments;
            TL::Optional<DepthStencilRGAttachment> m_depthStencilAttachment;
        };

        RenderPass                    m_renderPass;

        TL::Vector<BarrierInfo>       m_memoryBarriers[BarrierSlot_Count];
        TL::Vector<ImageBarrierInfo>  m_imageBarriers[BarrierSlot_Count];
        TL::Vector<BufferBarrierInfo> m_bufferBarriers[BarrierSlot_Count];

    public:
        RenderGraphExecuteGroup* m_group;
    };
} // namespace RHI
