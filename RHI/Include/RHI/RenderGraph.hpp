#pragma once

#include "RHI/Common/Handle.hpp"
#include "RHI/Common/Ptr.hpp"

#include "RHI/Resources.hpp"
#include "RHI/RGInternals.hpp"
#include "RHI/Attachment.hpp"
#include "RHI/RGPass.hpp"

/// @todo: remove graaf as dependency, and write custom graph data structure
#pragma warning(disable : 4702) // disable unreachable code warning (avoid compilation error with -Werror)
#include <graaflib/graph.h>

namespace RHI
{
    class Context;
    class Swapchain;
    class CommandList;
    class RenderGraph;
    class CommandEncoder;

    /// @brief Render Graph is Directed Acyclic Graph (DAG) that represents
    /// a set of passes, their dependencies, and their resources. It used to
    /// schedule and synchronize rendering operations
    class RHI_EXPORT RenderGraph final
    {
    public:
        RenderGraph(Context* context);
        ~RenderGraph();

        /// @brief Creates a new pass with the specified creation info.
        ///
        /// @param createInfo Information for creating the pass.
        /// @return Handle to the created pass.
        RHI_NODISCARD Handle<Pass> CreatePass(const PassCreateInfo& createInfo);

        /// @brief Resizes an existing pass to the given image size.
        ///
        /// @param pass Handle to the pass.
        /// @param size New size for the pass.
        void PassResize(Handle<Pass> pass, ImageSize2D size);

        /// @brief Retrieves the render target layout of a pass.
        ///
        /// @param pass Handle to the pass.
        /// @return Description of the render target layout.
        RHI_NODISCARD RenderTargetLayoutDesc PassGetRenderTargetLayout(Handle<Pass> pass) const;

        /// @brief Imports a swapchain image into the render graph.
        ///
        /// @param name Name of the swapchain.
        /// @param swapchain Reference to the swapchain.
        /// @return Handle to the imported image attachment.
        RHI_NODISCARD Handle<ImageAttachment> ImportSwapchain(const char* name, Swapchain& swapchain);

        /// @brief Imports an existing image into the render graph.
        ///
        /// @param name Name of the image.
        /// @param image Handle to the image.
        /// @return Handle to the imported image attachment.
        RHI_NODISCARD Handle<ImageAttachment> ImportImage(const char* name, Handle<Image> image);

        /// @brief Imports an existing buffer into the render graph.
        ///
        /// @param name Name of the buffer.
        /// @param buffer Handle to the buffer.
        /// @return Handle to the imported buffer attachment.
        RHI_NODISCARD Handle<BufferAttachment> ImportBuffer(const char* name, Handle<Buffer> buffer);

        /// @brief Creates a new image with the specified creation info.
        ///
        /// @param createInfo Information for creating the image.
        /// @return Handle to the created image attachment.
        RHI_NODISCARD Handle<ImageAttachment> CreateImage(const ImageCreateInfo& createInfo);

        /// @brief Creates a new buffer with the specified creation info.
        ///
        /// @param createInfo Information for creating the buffer.
        /// @return Handle to the created buffer attachment.
        RHI_NODISCARD Handle<BufferAttachment> CreateBuffer(const BufferCreateInfo& createInfo);

        /// @brief Uses an image in a pass with specified usage and access.
        ///
        /// @param pass Handle to the pass.
        /// @param attachment Handle to the image attachment.
        /// @param usage Usage flags for the image.
        /// @param stage Shader stage flags.
        /// @param access Access flags.
        void PassUseImage(Handle<Pass> pass, Handle<ImageAttachment> attachment, ImageUsage usage, Flags<ShaderStage> stage, Access access);

        /// @brief Uses an image in a pass with view info, usage, and access.
        ///
        /// @param pass Handle to the pass.
        /// @param attachment Handle to the image attachment.
        /// @param viewInfo View information for the image.
        /// @param usage Usage flags for the image.
        /// @param stage Shader stage flags.
        /// @param access Access flags.
        void PassUseImage(Handle<Pass> pass, Handle<ImageAttachment> attachment, const ImageViewInfo& viewInfo, ImageUsage usage, Flags<ShaderStage> stage, Access access);

        /// @brief Uses a buffer in a pass with specified usage and access.
        ///
        /// @param pass Handle to the pass.
        /// @param attachment Handle to the buffer attachment.
        /// @param usage Usage flags for the buffer.
        /// @param stage Shader stage flags.
        /// @param access Access flags.
        void PassUseBuffer(Handle<Pass> pass, Handle<BufferAttachment> attachment, BufferUsage usage, Flags<ShaderStage> stage, Access access);

        /// @brief Uses a buffer in a pass with view info, usage, and access.
        ///
        /// @param pass Handle to the pass.
        /// @param attachment Handle to the buffer attachment.
        /// @param viewInfo View information for the buffer.
        /// @param usage Usage flags for the buffer.
        /// @param stage Shader stage flags.
        /// @param access Access flags.
        void PassUseBuffer(Handle<Pass> pass, Handle<BufferAttachment> attachment, const BufferViewInfo& viewInfo, BufferUsage usage, Flags<ShaderStage> stage, Access access);

        /// @brief Retrieves the image from an image attachment.
        ///
        /// @param attachment Handle to the image attachment.
        /// @return Handle to the image.
        RHI_NODISCARD Handle<Image> GetImage(Handle<ImageAttachment> attachment) const;

        /// @brief Retrieves the buffer from a buffer attachment.
        ///
        /// @param attachment Handle to the buffer attachment.
        /// @return Handle to the buffer.
        RHI_NODISCARD Handle<Buffer> GetBuffer(Handle<BufferAttachment> attachment) const;

        /// @brief Retrieves the image view from a pass and image attachment.
        ///
        /// @param pass Handle to the pass.
        /// @param attachment Handle to the image attachment.
        /// @return Handle to the image view.
        RHI_NODISCARD Handle<ImageView> PassGetImageView(Handle<Pass> pass, Handle<ImageAttachment> attachment) const;

        /// @brief Retrieves the buffer view from a pass and buffer attachment.
        ///
        /// @param pass Handle to the pass.
        /// @param attachment Handle to the buffer attachment.
        /// @return Handle to the buffer view.
        RHI_NODISCARD Handle<BufferView> PassGetBufferView(Handle<Pass> pass, Handle<BufferAttachment> attachment) const;

        /// @brief Submits a pass with command lists and an optional signal fence.
        ///
        /// @param pass Handle to the pass.
        /// @param commandList Span of command lists to execute.
        /// @param signalFence Optional fence to signal after execution.
        void Submit(Handle<Pass> pass, const CommandEncoder& encoder, TL::Span<const Handle<CommandList>> commandList, Fence* signalFence = nullptr);

    private:
        enum class State
        {
            Initial,
            Building,
            Compiled,
            Invalidated,
        };

        using PassAttachment   = Handle<Pass>;
        using PassGraphID      = graaf::vertex_id_t;
        using PassDependencyID = graaf::edge_id_t;
        using Graph            = graaf::directed_graph<Handle<Pass>, unsigned>;

    private:
        /// @brief Gets the current state of the render graph.
        /// @return Current state of the graph.
        State GetGraphState() const;

        /// @brief Adds a dependency for an image attachment in a pass.
        ///
        /// @param pass Handle to the pass.
        /// @param attachment Handle to the image attachment.
        void AddDependency(Handle<Pass> pass, Handle<ImageAttachment> attachment);

        /// @brief Adds a dependency for a buffer attachment in a pass.
        ///
        /// @param pass Handle to the pass.
        /// @param attachment Handle to the buffer attachment.
        void AddDependency(Handle<Pass> pass, Handle<BufferAttachment> attachment);

        /// @brief Compiles the render graph.
        void Compile();

        /// @brief Compiles transient resources in the render graph.
        void CompileTransientResources();

        /// @brief Compiles attachment views in the render graph.
        void CompileAttachmentViews();

        /// @brief Cleans up resources used by the render graph.
        void Cleanup();

        /// @brief Cleans up transient resources in the render graph.
        void CleanupTransientResources();

        /// @brief Cleans up attachment views in the render graph.
        void CleanupResourceViews();

        /// @brief Executes the render graph
        ///
        /// @param signalFence Optional fence to signal after execution is complete.
        void Execute(Fence* signalFence);

    private:
        Context* m_context;

        // Directed asyclic graph, represents dependencies between passes.
        Graph m_graph;

        // lookup used to find graph VertexID from pass handle.
        TL::UnorderedMap<Handle<Pass>, PassGraphID> m_passToVertexLut;

        // lookup used to find pass handle from pass graph vertex id.
        TL::UnorderedMap<PassGraphID, Handle<Pass>> m_vertexToPassLut;

        // current state of the graph.
        State m_state;

        // current frame counter, incremented after graph execution
        uint64_t m_frameCounter;

        // graph resource's pool
        HandlePool<Pass>             m_passPool;
        HandlePool<ImageAttachment>  m_imageAttachmentPool;
        HandlePool<BufferAttachment> m_bufferAttachmentPool;

        // list of all passes in the graph
        TL::Vector<Handle<Pass>> m_passes;

        // list of all imported swapchain images in the graph
        TL::Vector<Handle<ImageAttachment>> m_importedSwapchainImageAttachments;

        // list of all imported images in the graph
        TL::Vector<Handle<ImageAttachment>> m_importedImageAttachments;

        // list of all imported buffers in the graph
        TL::Vector<Handle<BufferAttachment>> m_importedBufferAttachments;

        // list of all transient image in the graph
        TL::Vector<Handle<ImageAttachment>> m_transientImageAttachments;

        // list of all transient buffer in the graph
        TL::Vector<Handle<BufferAttachment>> m_transientBufferAttachments;

        // list of all image attachments in the graph
        TL::Vector<Handle<ImageAttachment>> m_imageAttachments;

        // list of all buffer attachments in the graph
        TL::Vector<Handle<BufferAttachment>> m_bufferAttachments;

        // Allocator used to create graph GPU resources (e.g. Images, Buffers...etc).
        // it can alias resources if their lifetimes don't overlap, and their memory properties are compatible.
        Ptr<TransientAliasingAllocator> m_transientAliasingAllocator;

        // Frame context, encapsulates the current frame state. and handles resource rotation and swaping.
        // All images and buffers are actually owned by the Frame Context.
        Ptr<RGResourcePool> m_frameContext;

        friend class TransientAliasingAllocator;
    };
} // namespace RHI