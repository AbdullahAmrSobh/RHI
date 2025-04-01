#pragma once

#include "RHI/Common.hpp"
#include "RHI/Export.hpp"
#include "RHI/RenderGraphResources.hpp"

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

    class RHI_EXPORT Pass
    {
        friend class RenderGraph;

    public:
        RHI_INTERFACE_BOILERPLATE(Pass);

        explicit Pass(const PassCreateInfo& createInfo, TL::IAllocator* allocator) noexcept;

        /// @brief Gets the name of the pass.
        const char*                                    GetName() const;

        /// @brief Gets the queue type which execute this pass.
        QueueType                                      GetQueueType() const { return m_queueType; }

        /// @brief Gets the current size of the pass.
        ImageSize2D                                    GetSize() const;

        /// @brief Gets the color attachments associated with this pass.
        TL::Span<const ColorRGAttachment>              GetColorAttachment() const;

        /// @brief Gets the depth/stencil attachment if present.
        const DepthStencilRGAttachment*                GetDepthStencilAttachment() const;

        /// @brief Gets the accessed resources for this pass.
        TL::Span<RenderGraphResourceTransition* const> GetRenderGraphResourceTransitions() const;

        // clang-format off
        RenderGraphResourceTransition* AddTransition(
            TL::IAllocator& allocator, RenderGraphResource& resource, ImageUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access, ImageSubresourceRange subresourceRange);
        RenderGraphResourceTransition* AddTransition(
            TL::IAllocator& allocator, RenderGraphResource& resource, BufferUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access, BufferSubregion subregion);
        // clang-format on

        virtual void AddSwapchainPresentBarrier(
                                              [[maybe_unused]] Device&                        device,
                                              [[maybe_unused]] Swapchain&                     swapchain,
                                              [[maybe_unused]] RenderGraphResourceTransition& transition)
        {
        }

        RenderGraphExecuteGroup* GetExecuteGroup() const { return m_group; }

    private:
        const char*                                                m_name;
        QueueType                                                  m_queueType;
        PassSetupCallback                                          m_onSetupCallback;
        PassCompileCallback                                        m_onCompileCallback;
        PassExecuteCallback                                        m_onExecuteCallback;
        ImageSize2D                                                m_size;
        TL::Vector<struct ColorRGAttachment, TL::IAllocator>       m_colorAttachments;
        TL::Optional<struct DepthStencilRGAttachment>              m_depthStencilAttachment;
        TL::Vector<RenderGraphResourceTransition*, TL::IAllocator> m_resourceTransitions;

    public:
        RenderGraphExecuteGroup* m_group;
    };
} // namespace RHI
