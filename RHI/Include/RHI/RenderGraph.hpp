#pragma once

#include "RHI/Handle.hpp"
#include "RHI/Export.hpp"
#include "RHI/Common.hpp"
#include "RHI/Resources.hpp"
#include "RHI/PipelineAccess.hpp"
#include "RHI/RenderGraphPass.hpp"
#include "RHI/RenderGraphResources.hpp"
#include "RHI/RenderGraphExecuteGroup.hpp"

#include "RHI/RenderTarget.hpp"

#include <TL/Allocator/Mimalloc.hpp>
#include <TL/Containers.hpp>
#include <TL/UniquePtr.hpp>

namespace RHI
{
    class Device;
    class Swapchain;
    class CommandList;
    class RenderGraph;
    class RenderGraphExecuteGroup;
    struct RenderTargetInfo;

    struct RenderGraphCreateInfo
    {
    };

    class RHI_EXPORT RenderGraph
    {
    public:
        RHI_INTERFACE_BOILERPLATE(RenderGraph);

        // Import existing resources into the render graph.

        /// @brief Imports a swapchain into the render graph for rendering.
        /// @param name The name of the swapchain resource.
        /// @param swapchain The swapchain object to import.
        /// @param format The format of the imported swapchain image.
        /// @return A pointer to the imported RenderGraphImage.
        TL_NODISCARD RenderGraphImage*  ImportSwapchain(const char* name, Swapchain& swapchain, Format format);

        /// @brief Imports an existing image into the render graph.
        /// @param name The name of the image resource.
        /// @param image The handle to the existing image.
        /// @param format The format of the imported image.
        /// @return A pointer to the imported RenderGraphImage.
        TL_NODISCARD RenderGraphImage*  ImportImage(const char* name, Handle<Image> image, Format format);

        /// @brief Imports an existing buffer into the render graph.
        /// @param name The name of the buffer resource.
        /// @param buffer The handle to the existing buffer.
        /// @param format The format of the imported image.
        TL_NODISCARD RenderGraphBuffer* ImportBuffer(const char* name, Handle<Buffer> buffer);

        // Create new resources in the render graph.

        /// @brief Creates a new image in the render graph.
        /// @param createInfo The creation parameters for the image.
        /// @return A pointer to the created RenderGraphImage.
        TL_NODISCARD RenderGraphImage*  CreateImage(const ImageCreateInfo& createInfo);

        /// @brief Creates a new buffer in the render graph.
        /// @param createInfo The creation parameters for the buffer.
        /// @return A pointer to the created RenderGraphBuffer.
        TL_NODISCARD RenderGraphBuffer* CreateBuffer(const BufferCreateInfo& createInfo);

        /// @brief Destroys an existing image in the render graph.
        /// @param image The pointer to the RenderGraphImage to destroy.
        void                            DestroyImage(RenderGraphImage* image);

        /// @brief Destroys an existing buffer in the render graph.
        /// @param buffer The pointer to the RenderGraphBuffer to destroy.
        void                            DestroyBuffer(RenderGraphBuffer* buffer);

        // Add and configure passes in the render graph.

        /// @brief Adds a new pass to the render graph.
        /// @param createInfo The creation parameters for the pass.
        /// @return A pointer to the added Pass.
        [[maybe_unused]] Pass*          AddPass(const PassCreateInfo& createInfo);

        /// @brief Declares an image to be used by a pass.
        /// @param pass The pass that will use the image.
        /// @param image The image resource to be used.
        /// @param usage The intended usage of the image.
        /// @param stage The pipeline stages where the image will be accessed.
        /// @param access The types of access to the image.
        void UseImage(Pass& pass, RenderGraphImage* image, ImageUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access);

        /// @brief Declares a buffer to be used by a pass.
        /// @param pass The pass that will use the buffer.
        /// @param buffer The buffer resource to be used.
        /// @param usage The intended usage of the buffer.
        /// @param stage The pipeline stages where the buffer will be accessed.
        /// @param access The types of access to the buffer.
        void UseBuffer(Pass& pass, RenderGraphBuffer* buffer, BufferUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access);

        /// @brief Declares a color attachment to be used by a pass.
        /// @param pass The pass that will use the render target.
        /// @param renderTargetInfo Information about the render target configuration.
        void UseColorAttachment(Pass& pass, const ColorRGAttachment& colorAttachment);

        /// @brief Declares a color attachment to be used by a pass.
        /// @param pass The pass that will use the render target.
        /// @param renderTargetInfo Information about the render target configuration.
        void UseDepthStencilAttachment(Pass& pass, const DepthStencilRGAttachment& depthStencilAttachment);

        // Frame management.

        /// @brief Begins a new frame in the render graph.
        void BeginFrame(ImageSize2D frameSize);

        /// @brief Ends the current frame in the render graph.
        void EndFrame();

        ColorAttachment        GetColorAttachment(const ColorRGAttachment& attachment) const;
        DepthStencilAttachment GetDepthStencilAttachment(const DepthStencilRGAttachment& attachment) const;

    private:
        /// @brief Returns passes sorted based on their graph topological order.
        TL::Span<Pass* const> GetSortedGraphPasses() const { return m_graphPasses; }

        /// @brief Compiles the render graph for the current frame.
        void                  Compile();

        void                  CleanupResources();

        /// @brief Initializes transient resources.
        void                  InitializeTransientResources();

        static void           ExtendResourceUsage(RenderGraphImage& resource, ImageUsage usage) { resource.m_usage.asImage |= usage; }

        static void           ExtendResourceUsage(RenderGraphBuffer& resource, BufferUsage usage) { resource.m_usage.asBuffer |= usage; }

    protected:
        void             ExecutePassCallback(Pass& pass, CommandList& commandList) { pass.m_onExecuteCallback(commandList); }

        virtual Pass*    CreatePass(const PassCreateInfo& createInfo)                                = 0;

        virtual void     OnGraphExecutionBegin()                                                     = 0;

        virtual void     OnGraphExecutionEnd()                                                       = 0;

        virtual uint64_t ExecutePassGroup(const RenderGraphExecuteGroup& group, QueueType queueType) = 0;

    protected:
        /// Main allocator for graph resources.
        TL::IAllocator*                               m_allocator = new TL::Mimalloc();
        /// Temporary allocator for transient data.
        TL::Arena                                     m_tempAllocator;
        /// The associated device for the render graph.
        Device*                                       m_device;
        /// The current frame index.
        uint64_t                                      m_frameIndex;
        /// List of images in the graph.
        TL::Vector<RenderGraphImage*>                 m_graphImages;
        /// List of buffers in the graph.
        TL::Vector<RenderGraphBuffer*>                m_graphBuffers;
        /// Lookup table for named resources.
        TL::Map<const char*, RenderGraphResource*>    m_graphResourcesLookup;
        /// Lookup table for imported images.
        TL::Map<Handle<Image>, RenderGraphImage*>     m_graphImportedImagesLookup;
        /// Lookup table for transient images.
        TL::Map<RenderGraphImage*, ImageCreateInfo>   m_graphTransientImagesLookup;
        /// Lookup table for imported buffers.
        TL::Map<Handle<Buffer>, RenderGraphBuffer*>   m_graphImportedBuffersLookup;
        /// Lookup table for transient buffers.
        TL::Map<RenderGraphBuffer*, BufferCreateInfo> m_graphTransientBuffersLookup;
        /// Lookup table for imported swapchains.
        TL::Map<Swapchain*, RenderGraphImage*>        m_graphImportedSwapchainsLookup;
        /// List of passes in the graph.
        TL::Vector<Pass*>                             m_graphPasses;
        /// List of passes ordered based on their execution.
        TL::Vector<RenderGraphExecuteGroup>           m_orderedPassGroups[AsyncQueuesCount];
        /// Input size of the graph (matches swapchain sizes).
        ImageSize2D                                   m_frameSize;

        enum class GraphState
        {
            Invalid,
            Compiled,
            Executing
        };

        GraphState m_state = GraphState::Invalid;
    };
} // namespace RHI