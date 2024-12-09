#pragma once

#include "RHI/Common.hpp"
#include "RHI/Export.hpp"
#include "RHI/RenderGraphResources.hpp"

#include <TL/Containers.hpp>
#include <TL/Memory.hpp>

namespace RHI
{
    class Pass;
    class RenderGraph;
    class CommandList;
    class Swapchain;

    using PassSetupCallback   = TL::Function<void(RenderGraph& renderGraph, Pass& pass)>;
    using PassCompileCallback = TL::Function<void(RenderGraph& renderGraph, Pass& pass)>;
    using PassExecuteCallback = TL::Function<void(CommandList& commandList)>;

    enum class PassFlags
    {
        None      = 0,
        Graphics  = 1 << 0,
        Compute   = 1 << 1,
        Transfer  = 1 << 2,
        Immediate = 1 << 3
    };

    struct PassCreateInfo
    {
        const char*          name;
        TL::Flags<PassFlags> flags;
        PassSetupCallback    setupCallback;
        PassCompileCallback  compileCallback;
        PassExecuteCallback  executeCallback;
    };

    class RenderGraphResource;

    class RHI_EXPORT Pass
    {
        friend class RenderGraph;

    public:
        RHI_INTERFACE_BOILERPLATE(Pass);

        explicit Pass(const PassCreateInfo& createInfo, TL::IAllocator* allocator) noexcept;

        /// @brief Gets the name of the pass.
        const char*                                    GetName() const;

        /// @brief Resizes the pass for a new image size.
        void                                           Resize(ImageSize2D size);

        /// @brief Gets the queue type which execute this pass.
        QueueType                                      GetQueueType() const { return m_queueType; }

        /// @brief Gets the current size of the pass.
        ImageSize2D                                    GetSize() const;

        /// @brief Gets the color attachments associated with this pass.
        TL::Span<const RenderTargetInfo>               GetColorAttachment() const;

        /// @brief Gets the depth/stencil attachment if present.
        const RenderTargetInfo*                        GetDepthStencilAttachment() const;

        /// @brief Gets the accessed resources for this pass.
        TL::Span<RenderGraphResourceTransition* const> GetRenderGraphResourceTransitions() const;

        /// @brief Adds a new resource access to the pass.
        RenderGraphResourceTransition*                 AddTransition(
                            TL::IAllocator&          allocator,
                            RenderGraphResource&     resource,
                            ImageUsage               usage,
                            TL::Flags<PipelineStage> stage,
                            TL::Flags<Access>        access,
                            ImageSubresourceRange    subresourceRange);

        RenderGraphResourceTransition* AddTransition(
            TL::IAllocator&          allocator,
            RenderGraphResource&     resource,
            BufferUsage              usage,
            TL::Flags<PipelineStage> stage,
            TL::Flags<Access>        access,
            BufferSubregion          subregion);

    private:
        const char*                                                m_name;
        QueueType                                                  m_queueType;
        PassSetupCallback                                          m_onSetupCallback;
        PassCompileCallback                                        m_onCompileCallback;
        PassExecuteCallback                                        m_onExecuteCallback;
        ImageSize2D                                                m_size;
        TL::Vector<RenderTargetInfo, TL::IAllocator>               m_colorAttachments;
        TL::Optional<RenderTargetInfo>                             m_depthStencilAttachment;
        TL::Vector<RenderGraphResourceTransition*, TL::IAllocator> m_resourceTransitions;
    };
} // namespace RHI
