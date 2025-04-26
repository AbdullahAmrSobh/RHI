#pragma once

#include "RHI/Device.hpp"
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
    class Device;
    class Pass;
    class RenderGraph;
    class CommandList;
    class Swapchain;

    using PassSetupCallback   = TL::Function<void(Pass& pass)>;
    using PassCompileCallback = TL::Function<void(Pass& pass)>;
    using PassExecuteCallback = TL::Function<void(CommandList& commandList)>;

    enum class PassType
    {
        Graphics,
        Compute,
        Transfer,
    };

    struct RenderGraphCreateInfo
    {
        TL::Allocator* allocator = nullptr;
    };

    struct PassCreateInfo
    {
        const char*         name;
        PassType            type;
        ImageSize2D         size;
        PassSetupCallback   setupCallback;
        PassCompileCallback compileCallback;
        PassExecuteCallback executeCallback;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Resource Data
    ///////////////////////////////////////////////////////////////////////////

    struct RenderGraphResource
    {
    };

    struct ImagePassDependency;
    struct BufferPassDependency;

    struct RenderGraphImage : RenderGraphResource
    {
        TL::String                                  m_name;
        TL::Flags<ImageUsage>                       m_usageFlags;
        ImageType                                   m_type;
        ImageSize3D                                 m_size;
        Format                                      m_format;
        SampleCount                                 m_sampleCount;
        uint32_t                                    m_mipLevels;
        uint32_t                                    m_arrayCount;
        Handle<Image>                               m_handle;
        TL::Map<ImageViewCreateInfo, Handle<Image>> m_views;
        ImagePassDependency*                        m_begin = nullptr;
        ImagePassDependency*                        m_end   = nullptr;
    };

    struct RenderGraphBuffer : RenderGraphResource
    {
        TL::String             m_name;
        TL::Flags<BufferUsage> m_usageFlags;
        size_t                 m_size;
        Handle<Buffer>         m_handle;
        BufferPassDependency*  m_begin = nullptr;
        BufferPassDependency*  m_end   = nullptr;
    };

    struct ImagePassDependency
    {
        Pass*                    pass             = nullptr;
        Handle<RenderGraphImage> rgImage          = NullHandle;
        Handle<Image>            view             = NullHandle;
        ImageSubresourceRange    subresourceRange = ImageSubresourceRange::All();
        ImageUsage               usage            = ImageUsage::None;
        PipelineStage            stage            = PipelineStage::None;
        Access                   access           = Access::None;

        ImagePassDependency *    next, *prev;
    };

    struct BufferPassDependency
    {
        Pass*                     pass             = nullptr;
        Handle<RenderGraphBuffer> rgBuffer         = NullHandle;
        BufferSubregion           subresourceRange = {};
        BufferUsage               usage            = BufferUsage::None;
        PipelineStage             stage            = PipelineStage::None;
        Access                    access           = Access::None;
        BufferPassDependency *    next, *prev;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Bindings
    ///////////////////////////////////////////////////////////////////////////

    struct ImageUseInfo
    {
        Handle<RenderGraphImage> image;
        ImageSubresourceRange    subresourcesRange;
    };

    struct BufferUseInfo
    {
        Handle<RenderGraphBuffer> buffer;
        BufferSubregion           subregion;
    };

    struct RGColorAttachment
    {
        ImageUseInfo   view        = {};
        LoadOperation  loadOp      = LoadOperation::Discard;
        StoreOperation storeOp     = StoreOperation::Store;
        ClearValue     clearValue  = {.f32 = {0.0f, 0.0f, 0.0f, 1.0f}};
        ResolveMode    resolveMode = ResolveMode::None;
        ImageUseInfo   resolveView = {};
    };

    struct RGDepthStencilAttachment
    {
        ImageUseInfo      view           = {};
        LoadOperation     depthLoadOp    = LoadOperation::Discard;
        StoreOperation    depthStoreOp   = StoreOperation::Store;
        LoadOperation     stencilLoadOp  = LoadOperation::Discard;
        StoreOperation    stencilStoreOp = StoreOperation::Store;
        DepthStencilValue clearValue     = {0.0f, 0};
    };

    struct RGRenderPassBeginInfo
    {
        ImageSize2D                            size;
        ImageOffset2D                          offset;
        TL::Span<const RGColorAttachment>      colorAttachments;
        TL::Optional<RGDepthStencilAttachment> depthStencilAttachment;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Common and helper types
    ///////////////////////////////////////////////////////////////////////////

    class RHI_EXPORT Pass
    {
        friend RenderGraph;

    public:
        RHI_INTERFACE_BOILERPLATE(Pass);

        /// @brief Enum representing different barrier slots for a pass.
        enum BarrierSlot
        {
            Prilogue,
            Epilogue,
            Resolve,
            Count,
        };

        ResultCode  Init(RenderGraph* rg, const PassCreateInfo& ci);
        void        Shutdown();

        const char* GetName() const
        {
            return m_name.c_str();
        }

        ImageSize2D GetImageSize() const
        {
            TL_ASSERT(m_isCompiled == true);
            return m_imageSize;
        }

        PassType GetType() const
        {
            return m_type;
        }

        void UseImage(const ImageUseInfo& useInfo, ImageUsage usage, PipelineStage stage, Access access);
        void UseBuffer(const BufferUseInfo& useInfo, ImageUsage usage, PipelineStage stage, Access access);
        void AddColorAttachment(RGColorAttachment colorAttachment);
        void SetDepthStencil(RGDepthStencilAttachment depthStencilAttachment);

    private:
        RenderGraph*                              m_renderGraph;
        TL::String                                m_name;
        PassType                                  m_type;
        PassSetupCallback                         m_setupCallback;
        PassCompileCallback                       m_compileCallback;
        PassExecuteCallback                       m_executeCallback;

        TL::Vector<TL::Ptr<ImagePassDependency>>  m_imageDependencies;
        TL::Vector<TL::Ptr<BufferPassDependency>> m_bufferDependencies;

        uint64_t                                  m_globalExecutionIndex                 = 0;
        uint64_t                                  m_dependencyLevelIndex                 = 0;
        uint64_t                                  m_localToDependencyLevelExecutionIndex = 0;
        uint64_t                                  m_localToQueueExecutionIndex           = 0;
        uint64_t                                  m_indexInUnorderedList                 = 0;

        ImageSize2D                               m_imageSize;
        TL::Vector<RGColorAttachment>             m_colorAttachments;
        TL::Optional<RGDepthStencilAttachment>    m_depthStencilAttachment;

        struct Barriers
        {
            TL::Vector<BarrierInfo>       memoryBarriers;
            TL::Vector<ImageBarrierInfo>  imageBarriers;
            TL::Vector<BufferBarrierInfo> bufferBarriers;
        };

        Barriers m_barriers[BarrierSlot::Count];
        bool     m_isActive   : 1 = false;
        bool     m_isCompiled : 1 = false;
    };

    class DependencyLevel
    {
    public:
        using Node         = Pass; // Assuming Node refers to Pass for this context
        using NodeList     = TL::Vector<Node*>;
        using NodeIterator = typename NodeList::iterator;

        friend RenderGraph;

        DependencyLevel(uint64_t levelIndex = 0)
            : m_levelIndex(levelIndex)
        {
        }

        void AddNode(Node* node)
        {
            m_nodes.push_back(node);
        }

        const NodeList&                            Nodes() const { return m_nodes; }

        const TL::Vector<TL::Vector<const Node*>>& NodesForQueue() const { return m_nodesPerQueue; }

        const TL::Set<uint32_t>&                   QueuesInvolvedInCrossQueueResourceReads() const { return m_queuesInvolvedInCrossQueueResourceReads; }

        const TL::Set<uint64_t>&                   SubresourcesReadByMultipleQueues() const { return m_subresourcesReadByMultipleQueues; }

        uint64_t                                   LevelIndex() const { return m_levelIndex; }

        void                                       AddNodeForQueue(uint32_t queueIndex, const Node* node)
        {
            if (queueIndex >= m_nodesPerQueue.size())
                m_nodesPerQueue.resize(queueIndex + 1);
            m_nodesPerQueue[queueIndex].push_back(node);
        }

        void AddQueueInvolved(uint32_t queueIndex)
        {
            m_queuesInvolvedInCrossQueueResourceReads.insert(queueIndex);
        }

        void AddSubresourceRead(uint64_t subresourceName)
        {
            m_subresourcesReadByMultipleQueues.insert(subresourceName);
        }

    private:
        uint64_t                            m_levelIndex = 0;
        NodeList                            m_nodes;
        TL::Vector<TL::Vector<const Node*>> m_nodesPerQueue;
        TL::Set<uint32_t>                   m_queuesInvolvedInCrossQueueResourceReads;
        TL::Set<uint64_t>                   m_subresourcesReadByMultipleQueues;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    class RHI_EXPORT RenderGraph
    {
        friend Device;
        friend Pass;

    public:
        RHI_INTERFACE_BOILERPLATE(RenderGraph);

        ResultCode                             Init(const RenderGraphCreateInfo& ci);
        void                                   Shutdown();

        // Import existing resources into the render graph.

        TL_NODISCARD Handle<RenderGraphImage>  ImportSwapchain(const char* name, Swapchain& swapchain, Format format);
        TL_NODISCARD Handle<RenderGraphImage>  ImportImage(const char* name, Handle<Image> image, Format format);
        TL_NODISCARD Handle<RenderGraphBuffer> ImportBuffer(const char* name, Handle<Buffer> buffer);

        TL_NODISCARD Handle<RenderGraphImage>  CreateImage(const char* name, ImageType type, ImageSize3D size, Format format, uint32_t mipLevels = 1, uint32_t arrayCount = 1, SampleCount samples = SampleCount::Samples1);
        TL_NODISCARD Handle<RenderGraphBuffer> CreateBuffer(const char* name, size_t size);

        TL_NODISCARD Handle<RenderGraphImage>  FindImportedImage(Handle<Image> image) const;
        TL_NODISCARD Handle<RenderGraphBuffer> FindImportedBuffer(Handle<Buffer> buffer) const;

        void                                   DestroyImage(Handle<RenderGraphImage> handle);
        void                                   DestroyBuffer(Handle<RenderGraphBuffer> handle);

        // Frame management.
        void                                   BeginFrame(ImageSize2D frameSize);
        void                                   EndFrame();

        /////////////////////////////////////////////////////////////////////////////////
        /// Graph execution: The following methods are only valid during graph setup
        /// And must never be called outside of Begin/End Scopes
        /////////////////////////////////////////////////////////////////////////////////

        TL_NODISCARD ImageSize2D               GetFrameSize() const;
        TL_NODISCARD const RenderGraphImage*   GetImage(Handle<RenderGraphImage> handle) const;
        TL_NODISCARD const RenderGraphBuffer*  GetBuffer(Handle<RenderGraphBuffer> handle) const;
        TL_NODISCARD Handle<Image>             GetImageHandle(Handle<RenderGraphImage> handle) const;
        TL_NODISCARD Handle<Buffer>            GetBufferHandle(Handle<RenderGraphBuffer> handle) const;

        Pass*                                  AddPass(const PassCreateInfo& createInfo);

        // void                                   QueueBufferRead(Handle<RenderGraphBuffer> buffer, uint32_t offset, TL::Block data);
        // void                                   QueueBufferWrite(Handle<RenderGraphBuffer> buffer, uint32_t offset, TL::Block data);
        // void                                   QueueImageRead(Handle<RenderGraphImage> image, ImageOffset3D offset, ImageSize3D size, ImageSubresourceLayers dstLayers, TL::Block block);
        // void                                   QueueImageWrite(Handle<RenderGraphImage> image, ImageOffset3D offset, ImageSize3D size, ImageSubresourceLayers dstLayers, TL::Block block);
        // template<typename ElementType> void    QueueBufferUpload(Handle<RenderGraphBuffer> buffer, TL::Span<const ElementType> elements);

    private:
        void ExtendImageUsageFlags(Handle<RenderGraphImage> resource, ImageUsage usage);
        void ExtendBufferUsageFlags(Handle<RenderGraphBuffer> resource, BufferUsage usage);

    private:
        // Utility functions
        bool      CheckDependency(const Pass* src, const Pass* dst) const;
        bool      CheckDependency(const struct Node* src, const struct Node* dst) const;
        Access    GetResourcePassAccess(Pass* pass, const RenderGraphResource* resource) const;
        QueueType GetPassAssignedQueue(const Pass* pass) const;

    private:
        void TransientResourcesInit(bool enableAliasing = false);
        void TransientResourcesShutdown();

        void Compile();
        void Execute();

    protected:
        TL::IAllocator*                                    m_allocator;
        TL::Arena                                          m_tempAllocator;
        Device*                                            m_device;
        uint64_t                                           m_frameIndex;

        HandlePool<RenderGraphImage>                       m_imageOwner;
        HandlePool<RenderGraphBuffer>                      m_bufferOwner;

        TL::Map<Handle<Image>, Handle<RenderGraphImage>>   m_graphImportedImagesLookup;
        TL::Set<Handle<RenderGraphImage>>                  m_graphTransientImagesLookup;
        TL::Map<Handle<Buffer>, Handle<RenderGraphBuffer>> m_graphImportedBuffersLookup;
        TL::Set<Handle<RenderGraphBuffer>>                 m_graphTransientBuffersLookup;
        TL::Map<Swapchain*, Handle<RenderGraphImage>>      m_graphImportedSwapchainsLookup;

        /// List of passes in the graph.
        TL::Vector<TL::Ptr<Pass>>                          m_graphPasses;

        /// Input size of the graph (matches swapchain sizes).
        ImageSize2D                                        m_frameSize;

        // Some internal states

        bool                                               m_isExecuting : 1;
        bool                                               m_isRecording : 1;
        bool                                               m_isCompiled  : 1;
    };
} // namespace RHI

// #include "RHI/RenderGraph.inl" // IWYU pragma: export