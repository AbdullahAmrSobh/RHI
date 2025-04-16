#pragma once

#include "RHI/Handle.hpp"
#include "RHI/Export.hpp"
#include "RHI/Common.hpp"
#include "RHI/Resources.hpp"
#include "RHI/PipelineAccess.hpp"
#include "RHI/CommandList.hpp"

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

    struct RenderGraphCreateInfo
    {
        TL::Allocator* allocator = nullptr; ///< Allocator to use for memory management.
    };

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
        BarrierSlot_Prologue, // Fixed typo
        BarrierSlot_Resolve,
        BarrierSlot_Count,
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    struct GraphTransition
    {
        Pass*                pass     = nullptr;
        GraphTransition*     next     = nullptr;
        GraphTransition*     prev     = nullptr;
        RenderGraphResource* resource = nullptr;
    };

    struct ImageGraphTransition : public GraphTransition
    {
        ImageUsage               usage;
        TL::Flags<PipelineStage> stage;
        TL::Flags<Access>        access;
        ImageSubresourceRange    subresourceRange;
    };

    struct BufferGraphTransition : public GraphTransition
    {
        BufferUsage              usage;
        TL::Flags<PipelineStage> stage;
        TL::Flags<Access>        access;
        BufferSubregion          subregion;
    };

    class RenderGraphResource
    {
    public:
        enum class Type : uint8_t
        {
            Image,
            Buffer
        };

        Type                   GetType() const { return m_type; }

        const char*            GetName() const { return m_name.c_str(); }

        const GraphTransition* GetFirstAccess() const { return m_first; }

        GraphTransition*       GetFirstAccess() { return m_first; }

        const GraphTransition* GetLastAccess() const { return m_last; }

        GraphTransition*       GetLastAccess() { return m_last; }

        const Pass*            GetFirstPass() const { return m_first->pass; }

        Pass*                  GetFirstPass() { return m_first->pass; }

        const Pass*            GetLastPass() const { return m_last->pass; }

        Pass*                  GetLastPass() { return m_last->pass; }

        void                   PushAccess(GraphTransition* access);

    protected:
        friend class RenderGraph;

        RenderGraphResource(const char* name, Type type);

        TL::String       m_name;
        GraphTransition* m_first;
        GraphTransition* m_last;
        Type             m_type;
        Format           m_format = Format::Unknown;

        union
        {
            Handle<Image>  asImage;
            Handle<Buffer> asBuffer;
        } m_handle;

        union
        {
            TL::Flags<ImageUsage>  asImage;
            TL::Flags<BufferUsage> asBuffer;
        } m_usage;

        bool isImported = false;
    };

    class RenderGraphImage final : public RenderGraphResource
    {
        friend RenderGraph;
        friend class Pass;

    public:
        RenderGraphImage(const char* name, Handle<Image> image, Format format);
        RenderGraphImage(const char* name, Format format);

        Handle<Image>         GetImage() const { return m_handle.asImage; }

        Format                GetFormat() const { return m_format; }

        TL::Flags<ImageUsage> GetImageUsage() const { return m_usage.asImage; }
    };

    class RenderGraphBuffer final : public RenderGraphResource
    {
        friend class Pass;

    public:
        RenderGraphBuffer(const char* name, Handle<Buffer> buffer);
        RenderGraphBuffer(const char* name);

        Handle<Buffer>         GetBuffer() const { return m_handle.asBuffer; }

        TL::Flags<BufferUsage> GetBufferUsage() const { return m_usage.asBuffer; }
    };

    struct ColorRGAttachment
    {
        RenderGraphImage* view        = nullptr;
        LoadOperation     loadOp      = LoadOperation::Discard;
        StoreOperation    storeOp     = StoreOperation::Store;
        ClearValue        clearValue  = {.f32 = {0.0f, 0.0f, 0.0f, 1.0f}};
        ResolveMode       resolveMode = ResolveMode::None;
        RenderGraphImage* resolveView = nullptr;
    };

    struct DepthStencilRGAttachment
    {
        RenderGraphImage* view           = nullptr;
        LoadOperation     depthLoadOp    = LoadOperation::Discard;
        StoreOperation    depthStoreOp   = StoreOperation::Store;
        LoadOperation     stencilLoadOp  = LoadOperation::Discard;
        StoreOperation    stencilStoreOp = StoreOperation::Store;
        DepthStencilValue clearValue     = {0.0f, 0};
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    class RHI_EXPORT Pass
    {
        friend class RenderGraph;

    public:
        Pass(const PassCreateInfo& createInfo, TL::IAllocator* allocator);
        ~Pass();

        // Accessors
        TL::Span<GraphTransition* const>       GetTransitions() const;
        TL::Span<ImageGraphTransition* const>  GetImageTransitions() const;
        TL::Span<BufferGraphTransition* const> GetBufferTransitions() const;

        // Execution
        void                                   Execute(CommandList& commandList);

        // Resource usage
        void                                   UseResource(RenderGraphImage& resource, ImageSubresourceRange subresourceRange, ImageUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access);
        void                                   UseResource(RenderGraphBuffer& resource, BufferSubregion subregion, BufferUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access);
        void                                   PresentSwapchain(RenderGraphImage& resource);

        // Render targets
        void                                   AddRenderTarget(const ColorRGAttachment& attachment);
        void                                   AddRenderTarget(const DepthStencilRGAttachment& attachment);

    private:
        // Barrier management
        TL::Span<const BarrierInfo>       GetMemoryBarriers(BarrierSlot slot) const;
        TL::Span<const ImageBarrierInfo>  GetImageBarriers(BarrierSlot slot) const;
        TL::Span<const BufferBarrierInfo> GetBufferBarriers(BarrierSlot slot) const;
        void                              PrepareBarriers();

        // Execution group
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

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    struct AsyncQueueSyncInfo
    {
        uint64_t                 timelineValue;
        TL::Flags<PipelineStage> waitStage;
    };

    struct SwapchainSyncInfo
    {
        Swapchain*               swapchain;
        TL::Flags<PipelineStage> stage;
    };

    /// @class RenderGraphExecuteGroup
    /// @brief Interface for managing the execution of passes in a render graph.
    ///
    /// The RenderGraphExecuteGroup class handles the scheduling and synchronization of render passes
    /// and their dependencies, ensuring proper execution order and resource usage within the rendering pipeline.
    class RHI_EXPORT RenderGraphExecuteGroup
    {
    public:
        /// @brief Constructs a new render graph execution group.
        RenderGraphExecuteGroup(TL::IAllocator& allocator)
            : m_passList(allocator)
        {
        }

        /// @brief Adds a render pass to the execution group.
        ///
        /// @param pass   The render pass to be added.
        void AddPass(Pass& pass)
        {
            m_passList.push_back(&pass);
            // m_queueSignalInfo = pass
        }

        /// @brief Specifies a queue dependency that the group must wait for before proceeding.
        ///
        /// @param type          The type of queue to wait for.
        /// @param timelineValue The timeline semaphore value to wait until.
        /// @param waitStage     The pipeline stages where the wait operation should occur.
        void WaitForQueue(QueueType type, uint64_t timelineValue, TL::Flags<PipelineStage> waitStage)
        {
            m_queueWaitInfos[(uint32_t)type] = {timelineValue, waitStage};
        }

        /// @brief Specifies a swapchain dependency that the group must wait for before proceeding.
        ///
        /// @param swapchain   The swapchain to wait for.
        /// @param waitStage   The pipeline stages where the wait operation should occur.
        void WaitForSwapchain(Swapchain& swapchain, TL::Flags<PipelineStage> waitStage) { m_swapchainToWait = {&swapchain, waitStage}; }

        /// @brief Signals that the swapchain is ready for presentation.
        ///
        /// @param swapchain   The swapchain to signal for presentation.
        /// @param signalStage The pipeline stages where the signal operation should occur.
        void SignalSwapchainPresent(Swapchain& swapchain, TL::Flags<PipelineStage> signalStage)
        {
            m_swapchainToSignal = {&swapchain, signalStage};
        }

        TL::Span<Pass* const> GetPassList() const { return m_passList; }

        AsyncQueueSyncInfo    GetQueueSignalInfo() const { return m_queueSignalInfo; }

        AsyncQueueSyncInfo    GetQueueWaitInfo(QueueType type) const { return m_queueWaitInfos[(uint32_t)type]; }

        SwapchainSyncInfo     GetSwapchainToWait() const { return m_swapchainToWait; }

        SwapchainSyncInfo     GetSwapchainToSignal() const { return m_swapchainToSignal; }

    protected:
        TL::Vector<Pass*, TL::IAllocator> m_passList;
        AsyncQueueSyncInfo                m_queueSignalInfo;
        AsyncQueueSyncInfo                m_queueWaitInfos[AsyncQueuesCount];
        SwapchainSyncInfo                 m_swapchainToWait;
        SwapchainSyncInfo                 m_swapchainToSignal;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    class RHI_EXPORT RenderGraph
    {
        friend Device;

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
        void                            UseImage(Pass& pass, RenderGraphImage* image, ImageUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access);

        /// @brief Declares a buffer to be used by a pass.
        /// @param pass The pass that will use the buffer.
        /// @param buffer The buffer resource to be used.
        /// @param usage The intended usage of the buffer.
        /// @param stage The pipeline stages where the buffer will be accessed.
        /// @param access The types of access to the buffer.
        void                            UseBuffer(Pass& pass, RenderGraphBuffer* buffer, BufferUsage usage, TL::Flags<PipelineStage> stage, TL::Flags<Access> access);

        /// @brief Declares a color attachment to be used by a pass.
        /// @param pass The pass that will use the render target.
        /// @param renderTargetInfo Information about the render target configuration.
        void                            UseColorAttachment(Pass& pass, const ColorRGAttachment& colorAttachment);

        /// @brief Declares a color attachment to be used by a pass.
        /// @param pass The pass that will use the render target.
        /// @param renderTargetInfo Information about the render target configuration.
        void                            UseDepthStencilAttachment(Pass& pass, const DepthStencilRGAttachment& depthStencilAttachment);

        // Frame management.

        /// @brief Begins a new frame in the render graph.
        void                            BeginFrame(ImageSize2D frameSize);

        /// @brief Ends the current frame in the render graph.
        void                            EndFrame();

        ColorAttachment                 GetColorAttachment(const ColorRGAttachment& attachment) const;
        DepthStencilAttachment          GetDepthStencilAttachment(const DepthStencilRGAttachment& attachment) const;

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
        void     ExecutePassCallback(Pass& pass, CommandList& commandList) { pass.m_onExecuteCallback(commandList); }

        void     OnGraphExecutionBegin();

        void     OnGraphExecutionEnd();

        uint64_t ExecutePassGroup(const RenderGraphExecuteGroup& group, QueueType queueType);

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