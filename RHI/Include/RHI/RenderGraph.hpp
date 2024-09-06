#pragma once

#include "RHI/Handle.hpp"

#include "RHI/Resources.hpp"
#include "RHI/Fence.hpp"
#include "RHI/RGResources.hpp"
#include "RHI/RGPass.hpp"

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
        RenderGraph(RenderGraph&&) = default;
        ~RenderGraph();

        /// @brief Creates a new pass with the specified creation info.
        ///
        /// @param createInfo Information for creating the pass.
        /// @return Handle to the created pass.
        TL_NODISCARD Handle<Pass>             CreatePass(const PassCreateInfo& createInfo);

        /// @brief Resizes an existing pass to the given image size.
        ///
        /// @param pass Handle to the pass.
        /// @param size New size for the pass.
        void                                  PassResize(Handle<Pass> pass, ImageSize2D size);

        /// @brief Returns the pass size.
        ///
        /// @param pass Handle to the pass.
        /// @return Size of the pass.
        ImageSize2D                           GetPassSize(Handle<Pass> pass) const;

        /// @brief Imports a swapchain image into the render graph.
        ///
        /// @param name Name of the swapchain.
        /// @param swapchain Reference to the swapchain.
        /// @return Handle to the imported image attachment.
        TL_NODISCARD Handle<ImageAttachment>  ImportSwapchain(const char* name, Swapchain& swapchain);

        /// @brief Imports an existing image into the render graph.
        ///
        /// @param name Name of the image.
        /// @param image Handle to the image.
        /// @return Handle to the imported image attachment.
        TL_NODISCARD Handle<ImageAttachment>  ImportImage(const char* name, Handle<Image> image);

        /// @brief Imports an existing buffer into the render graph.
        ///
        /// @param name Name of the buffer.
        /// @param buffer Handle to the buffer.
        /// @return Handle to the imported buffer attachment.
        TL_NODISCARD Handle<BufferAttachment> ImportBuffer(const char* name, Handle<Buffer> buffer);

        /// @brief Creates a new image with the specified creation info.
        ///
        /// @param createInfo Information for creating the image.
        /// @return Handle to the created image attachment.
        TL_NODISCARD Handle<ImageAttachment>  CreateImage(const ImageCreateInfo& createInfo);

        /// @brief Creates a new buffer with the specified creation info.
        ///
        /// @param createInfo Information for creating the buffer.
        /// @return Handle to the created buffer attachment.
        TL_NODISCARD Handle<BufferAttachment> CreateBuffer(const BufferCreateInfo& createInfo);

        /// @brief Uses an image in a pass with view info, usage, and access.
        ///
        /// @param pass Handle to the pass.
        /// @param attachment Handle to the image attachment.
        /// @param viewInfo View information for the image.
        /// @param usage Usage flags for the image.
        /// @param stage Shader stage flags.
        /// @param access Access flags.
        void                                  PassUseImage(Handle<Pass> pass, Handle<ImageAttachment> attachment, const ImageViewInfo& viewInfo, ImageUsage usage, TL::Flags<ShaderStage> stage, Access access);

        /// @brief Uses a buffer in a pass with view info, usage, and access.
        ///
        /// @param pass Handle to the pass.
        /// @param attachment Handle to the buffer attachment.
        /// @param viewInfo View information for the buffer.
        /// @param usage Usage flags for the buffer.
        /// @param stage Shader stage flags.
        /// @param access Access flags.
        void                                  PassUseBuffer(Handle<Pass> pass, Handle<BufferAttachment> attachment, const BufferViewInfo& viewInfo, BufferUsage usage, TL::Flags<ShaderStage> stage, Access access);

        /// @brief Retrieves the image from an image attachment.
        ///
        /// @param attachment Handle to the image attachment.
        /// @return Handle to the image.
        TL_NODISCARD Handle<Image>            GetImage(Handle<ImageAttachment> attachment) const;

        /// @brief Retrieves the buffer from a buffer attachment.
        ///
        /// @param attachment Handle to the buffer attachment.
        /// @return Handle to the buffer.
        TL_NODISCARD Handle<Buffer>           GetBuffer(Handle<BufferAttachment> attachment) const;

        /// @brief Retrieves the image view from a pass and image attachment.
        ///
        /// @param pass Handle to the pass.
        /// @param attachment Handle to the image attachment.
        /// @return Handle to the image view.
        TL_NODISCARD Handle<ImageView>        PassGetImageView(Handle<Pass> pass, Handle<ImageAttachment> attachment) const;

        /// @brief Retrieves the buffer view from a pass and buffer attachment.
        ///
        /// @param pass Handle to the pass.
        /// @param attachment Handle to the buffer attachment.
        /// @return Handle to the buffer view.
        TL_NODISCARD Handle<BufferView>       PassGetBufferView(Handle<Pass> pass, Handle<BufferAttachment> attachment) const;

        /// @brief Submits a pass with command lists and an optional signal fence.
        ///
        /// @param pass Handle to the pass.
        /// @param commandList Span of command lists to execute.
        /// @param signalFence Optional fence to signal after execution.
        void                                  Submit(Handle<Pass> pass, TL::Span<CommandList*> commandList, Fence* signalFence = nullptr);

        void                                  Invalidate() {}

        // private:
        /// @brief Compiles the render graph.
        void                                  Compile();

        /// @brief Cleans up resources used by the render graph.
        void                                  Cleanup();

        void                                  CleanupAttachmentViews();
        void                                  CleanupTransientAttachments();

    public:
        Context*                             m_context;

        // current frame counter, incremented after graph execution
        uint64_t                             m_frameCounter;

        // graph resource's pool
        HandlePool<Pass>                     m_passPool;
        HandlePool<ImageAttachment>          m_imageAttachmentPool;
        HandlePool<BufferAttachment>         m_bufferAttachmentPool;

        // list of all passes in the graph
        TL::Vector<Handle<Pass>>             m_passes;

        // list of all imported swapchain images in the graph
        TL::Vector<Handle<ImageAttachment>>  m_importedSwapchainImageAttachments;

        // list of all imported images in the graph
        TL::Vector<Handle<ImageAttachment>>  m_importedImageAttachments;

        // list of all imported buffers in the graph
        TL::Vector<Handle<BufferAttachment>> m_importedBufferAttachments;

        // list of all transient image in the graph
        TL::Vector<Handle<ImageAttachment>>  m_transientImageAttachments;

        // list of all transient buffer in the graph
        TL::Vector<Handle<BufferAttachment>> m_transientBufferAttachments;

        // list of all image attachments in the graph
        TL::Vector<Handle<ImageAttachment>>  m_imageAttachments;

        // list of all buffer attachments in the graph
        TL::Vector<Handle<BufferAttachment>> m_bufferAttachments;

    private:
        TL::Arena                                            m_arena;
        mutable TL::UnorderedMap<size_t, Handle<Image>>      m_imagesLRU;
        mutable TL::UnorderedMap<size_t, Handle<Buffer>>     m_buffersLRU;
        mutable TL::UnorderedMap<size_t, Handle<ImageView>>  m_imageViewsLRU;
        mutable TL::UnorderedMap<size_t, Handle<BufferView>> m_bufferViewsLRU;
    };
} // namespace RHI