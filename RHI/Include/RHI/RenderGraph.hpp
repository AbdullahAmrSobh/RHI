#pragma once

#include "RHI/Handle.hpp"
#include "RHI/Common.hpp"
#include "RHI/PipelineAccess.hpp"
#include "RHI/RGResources.hpp"
#include "RHI/RGPass.hpp"

#include <TL/Containers.hpp>

namespace RHI
{
    class Device;
    class Swapchain;
    class CommandList;
    class RenderGraph;
    class CommandEncoder;

    using PassExecuteCallback = TL::Function<void(CommandList& commandList)>;

    /// @brief Render Graph is Directed Acyclic Graph (DAG) that represents
    /// a set of passes, their dependencies, and their resources. It used to
    /// schedule and synchronize rendering operations
    class RHI_EXPORT RenderGraph final
    {
    public:
        RHI_INTERFACE_BOILERPLATE(RenderGraph);

        /// @brief Creates a new pass with the specified creation info.
        ///
        /// @param createInfo Information for creating the pass.
        /// @return Handle to the created pass.
        TL_NODISCARD Handle<Pass>     CreatePass(const PassCreateInfo& createInfo);

        /// @brief Resizes an existing pass to the given image size.
        ///
        /// @param pass Handle to the pass.
        /// @param size New size for the pass.
        void                          PassResize(Handle<Pass> pass, ImageSize2D size);

        /// @brief Returns the pass size.
        ///
        /// @param pass Handle to the pass.
        /// @return Size of the pass.
        ImageSize2D                   PassGetSize(Handle<Pass> pass) const;

        /// @brief Imports a swapchain image into the render graph.
        ///
        /// @param name Name of the swapchain.
        /// @param swapchain Reference to the swapchain.
        /// @return Handle to the imported image attachment.
        TL_NODISCARD Handle<RGImage>  ImportSwapchain(const char* name, Swapchain& swapchain);

        /// @brief Imports an existing image into the render graph.
        ///
        /// @param name Name of the image.
        /// @param image Handle to the image.
        /// @return Handle to the imported image attachment.
        TL_NODISCARD Handle<RGImage>  ImportImage(const char* name, Handle<Image> image);

        /// @brief Imports an existing buffer into the render graph.
        ///
        /// @param name Name of the buffer.
        /// @param buffer Handle to the buffer.
        /// @return Handle to the imported buffer attachment.
        TL_NODISCARD Handle<RGBuffer> ImportBuffer(const char* name, Handle<Buffer> buffer);

        /// @brief Creates a new image with the specified creation info.
        ///
        /// @param createInfo Information for creating the image.
        /// @return Handle to the created image attachment.
        TL_NODISCARD Handle<RGImage>  CreateImage(const ImageCreateInfo& createInfo);

        /// @brief Creates a new buffer with the specified creation info.
        ///
        /// @param createInfo Information for creating the buffer.
        /// @return Handle to the created buffer attachment.
        TL_NODISCARD Handle<RGBuffer> CreateBuffer(const BufferCreateInfo& createInfo);

        /// @brief Uses an image in a pass with view info, usage, and access.
        ///
        /// @param pass Handle to the pass.
        /// @param attachment Handle to the image attachment.
        /// @param viewInfo View information for the image.
        /// @param usage Usage flags for the image.
        /// @param stage Shader stage flags.
        /// @param access Access flags.
        void PassUseImage(Handle<Pass> pass, Handle<RGImage> attachment, ImageUsage usage, TL::Flags<PipelineStage> stage, Access access);

        /// @brief Uses a buffer in a pass with view info, usage, and access.
        ///
        /// @param pass Handle to the pass.
        /// @param attachment Handle to the buffer attachment.
        /// @param viewInfo View information for the buffer.
        /// @param usage Usage flags for the buffer.
        /// @param stage Shader stage flags.
        /// @param access Access flags.
        void PassUseBuffer(
            Handle<Pass> pass, Handle<RGBuffer> attachment, BufferUsage usage, TL::Flags<PipelineStage> stage, Access access);

        /// @brief Retrieves the image from an image attachment.
        ///
        /// @param attachment Handle to the image attachment.
        /// @return Handle to the image.
        TL_NODISCARD Handle<Image>  GetImage(Handle<RGImage> attachment) const;

        /// @brief Retrieves the buffer from a buffer attachment.
        ///
        /// @param attachment Handle to the buffer attachment.
        /// @return Handle to the buffer.
        TL_NODISCARD Handle<Buffer> GetBuffer(Handle<RGBuffer> attachment) const;

        /// @brief Compiles the render graph.
        void                        Compile();

    private:
        /// @brief Cleans up resources used by the render graph.
        void Cleanup();

    public:
        Device*                      m_device;

        // current frame counter, incremented after graph execution
        uint64_t                     m_frameCounter;

        // graph resource's pool
        HandlePool<Pass>             m_passPool;
        HandlePool<RGImage>          m_rgImagesPool;
        HandlePool<RGBuffer>         m_rgBufferPool;

        // list of all passes in the graph
        TL::Vector<Handle<Pass>>     m_passList;

        // list of all imported swapchain images in the graph
        TL::Vector<Handle<RGImage>>  m_swapchainImages;

        // list of all imported images in the graph
        TL::Vector<Handle<RGImage>>  m_importedImages;

        // list of all imported buffers in the graph
        TL::Vector<Handle<RGBuffer>> m_importedBuffers;

        // list of all transient image in the graph
        TL::Vector<Handle<RGImage>>  m_transientImages;

        // list of all transient buffer in the graph
        TL::Vector<Handle<RGBuffer>> m_transientBuffers;

        // list of all image attachments in the graph
        TL::Vector<Handle<RGImage>>  m_images;

        // list of all buffer attachments in the graph
        TL::Vector<Handle<RGBuffer>> m_buffers;

    private:
        TL::Arena m_arena;
    };
} // namespace RHI