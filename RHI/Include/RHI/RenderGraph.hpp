#pragma once

#include "RHI/Device.hpp"
#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"
#include "RHI/PipelineAccess.hpp"
#include "RHI/CommandList.hpp"

#include <TL/Ptr.hpp>

#include <TL/Allocator/Arena.hpp>
#include <TL/Containers/Function.hpp>
#include <TL/Containers/Set.hpp>
#include <TL/Containers/Map.hpp>
#include <TL/Containers/Vector.hpp>

namespace RHI
{
    class Device;
    class CommandList;
    class Swapchain;

    class RenderGraph;
    class RenderGraphBuilder;

    using PassExecuteCallback = TL::Function<void(CommandList& commandList)>;

    enum class RGPassType
    {
        Graphics,
        Compute,
        AsyncCompute,
        Transfer,
    };

    struct RenderGraphCreateInfo
    {
        TL::StringView name                    = {};
        TL::Arena*     tmpAllocator            = nullptr;
        bool           tmpAllocatorShouldClear = 0;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Graphics Pass
    ///////////////////////////////////////////////////////////////////////////

    struct RGImage;
    struct RGBuffer;

    struct RGColorAttachment
    {
        RGImage*              color        = nullptr;
        ImageSubresourceRange colorRange   = ImageSubresourceRange::All();
        LoadOperation         loadOp       = LoadOperation::Discard;
        StoreOperation        storeOp      = StoreOperation::Store;
        ClearValue            clearValue   = {.f32 = {0.0f, 0.0f, 0.0f, 1.0f}};
        ResolveMode           resolveMode  = ResolveMode::None;
        RGImage*              resolveView  = {};
        ImageSubresourceRange resolveRange = ImageSubresourceRange::All();
    };

    struct RGDepthStencilAttachment
    {
        RGImage*              depthStencil      = nullptr;
        ImageSubresourceRange depthStencilRange = ImageSubresourceRange::All();
        LoadOperation         depthLoadOp       = LoadOperation::Discard;
        StoreOperation        depthStoreOp      = StoreOperation::Store;
        LoadOperation         stencilLoadOp     = LoadOperation::Discard;
        StoreOperation        stencilStoreOp    = StoreOperation::Store;
        DepthStencilValue     clearValue        = {0.0f, 0};
    };

    struct RGRenderPassBeginInfo
    {
        ImageSize2D                       size                   = {};
        ImageOffset2D                     offset                 = {};
        TL::Span<const RGColorAttachment> colorAttachments       = {};
        RGDepthStencilAttachment          depthStencilAttachment = {};
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Resources
    ///////////////////////////////////////////////////////////////////////////

    class RGPass;

    struct RGResource
    {
    };

    struct RGFrameImage
    {
        TL::String            name          = {};
        bool                  isImported    = false;
        TL::Flags<ImageUsage> usageFlags    = ImageUsage::None;
        ImageType             type          = ImageType::None;
        ImageSize3D           size          = ImageSize3D();
        Format                format        = Format::Unknown;
        SampleCount           sampleCount   = SampleCount::Samples1;
        uint32_t              mipLevels     = 1;
        uint32_t              arrayCount    = 1;
        Image*                handle        = nullptr;
        RGImage*              firstProducer = nullptr;
        RGImage*              lastProducer  = nullptr;

        Fence*                fence;
    };

    struct RGFrameBuffer
    {
        TL::String             name          = {};
        TL::Flags<BufferUsage> usageFlags    = BufferUsage::None;
        size_t                 size          = 0;
        bool                   isImported    = false;
        Buffer*                handle        = nullptr;
        RGBuffer*              firstProducer = nullptr;
        RGBuffer*              lastProducer  = nullptr;
    };

    struct RGImage : RGResource
    {
        ImageBarrierState m_state         = {};
        RGPass*           m_producer      = nullptr;
        RGFrameImage*     m_frameResource = nullptr;
        RGImage*          m_prevHandle    = nullptr;
        RGImage*          m_nextHandle    = nullptr;
    };

    struct RGBuffer : RGResource
    {
        // Buffer*     m_handle        = nullptr;
        BufferBarrierState m_state         = {};
        RGPass*            m_producer      = nullptr;
        RGFrameBuffer*     m_frameResource = nullptr;
        RGBuffer*          m_prevHandle    = nullptr;
        RGBuffer*          m_nextHandle    = nullptr;
    };

    struct RGImageDependency
    {
        RGImage*          image;
        uint32_t          viewID;
        ImageBarrierState state;
    };

    struct RGBufferDependency
    {
        RGBuffer*          buffer;
        BufferSubregion    subregion;
        BufferBarrierState state;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Pass
    ///////////////////////////////////////////////////////////////////////////

    enum class RGBufferUsage : uint64_t
    {
        None = 0,

        ConstantGeometry                  = 1 << 0,
        ConstantPixel                     = 1 << 1,
        ConstantCompute                   = 1 << 2,
        ConstantTraceRays                 = 1 << 3,
        SrvGeometry                       = 1 << 4,
        SrvPixel                          = 1 << 5,
        SrvCompute                        = 1 << 6,
        SrvTraceRays                      = 1 << 7,
        UavGeometry                       = 1 << 8,
        UavPixel                          = 1 << 9,
        UavCompute                        = 1 << 10,
        UavTraceRays                      = 1 << 11,
        VertexOrIndex                     = 1 << 12,
        IndirectCompute                   = 1 << 14,
        IndirectDraw                      = 1 << 15,
        IndirectTraceRays                 = 1 << 16,
        CopySource                        = 1 << 17,
        CopyDestination                   = 1 << 18,
        AccelerationStructureBuild        = 1 << 19, ///< Will be used as a position or index buffer in a BLAS build.
        ShaderBindingTable                = 1 << 20, ///< Will be used as SBT in a traceRays() command.
        AccelerationStructureBuildScratch = 1 << 21, ///< Used in buildAccelerationStructureXXX commands.

        // Derived
        AllConstant = ConstantGeometry | ConstantPixel | ConstantCompute | ConstantTraceRays,
        AllSrv      = SrvGeometry | SrvPixel | SrvCompute | SrvTraceRays,
        AllUav      = UavGeometry | UavPixel | UavCompute | UavTraceRays,
        AllIndirect = IndirectCompute | IndirectDraw | IndirectTraceRays,
        AllCopy     = CopySource | CopyDestination,

        AllGeometry  = ConstantGeometry | SrvGeometry | UavGeometry | VertexOrIndex,
        AllPixel     = ConstantPixel | SrvPixel | UavPixel,
        AllGraphics  = AllGeometry | AllPixel | IndirectDraw,
        AllCompute   = ConstantCompute | SrvCompute | UavCompute | IndirectCompute,
        AllTraceRays = ConstantTraceRays | SrvTraceRays | UavTraceRays | IndirectTraceRays | ShaderBindingTable,

        AllRayTracing = AllTraceRays | AccelerationStructureBuild | AccelerationStructureBuildScratch,
        AllRead       = AllConstant | AllSrv | AllUav | VertexOrIndex | AllIndirect | CopySource | AccelerationStructureBuild | ShaderBindingTable,
        AllWrite      = AllUav | CopyDestination | AccelerationStructureBuildScratch,

        AllShaderResource = AllConstant | AllSrv | AllUav,

        All = AllRead | AllWrite,
    };

    TL_DEFINE_FLAG_OPERATORS(RGBufferUsage);

    enum class RGImageUsage : uint32_t
    {
        None = 0,

        SrvGeometry     = 1 << 0,
        SrvPixel        = 1 << 1,
        SrvCompute      = 1 << 2,
        SrvTraceRays    = 1 << 3,
        UavGeometry     = 1 << 4,
        UavPixel        = 1 << 5,
        UavCompute      = 1 << 6,
        UavTraceRays    = 1 << 7,
        RtvDsvRead      = 1 << 8,
        RtvDsvWrite     = 1 << 9,
        ShadingRate     = 1 << 10,
        CopyDestination = 1 << 11,
        CopySource      = 1 << 13,
        Present         = 1 << 12,

        // Derived
        AllSrv    = SrvGeometry | SrvPixel | SrvCompute | SrvTraceRays,
        AllUav    = UavGeometry | UavPixel | UavCompute | UavTraceRays,
        AllRtvDsv = RtvDsvRead | RtvDsvWrite,

        AllGeometry = SrvGeometry | UavGeometry,
        AllPixel    = SrvPixel | UavPixel,
        AllGraphics = AllGeometry | AllPixel | RtvDsvRead | RtvDsvWrite | ShadingRate,
        AllCompute  = SrvCompute | UavCompute,
        AllCopy     = CopySource | CopyDestination,

        AllRead           = AllSrv | AllUav | RtvDsvRead | ShadingRate | Present,
        AllWrite          = AllUav | RtvDsvWrite | CopyDestination,
        All               = AllRead | AllWrite,
        AllShaderResource = AllSrv | AllUav,
    };

    TL_DEFINE_FLAG_OPERATORS(RGImageUsage);

    struct RGImageCreateInfo
    {
        TL::StringView name        = {};
        ImageType      type        = ImageType::None;
        ImageSize3D    size        = ImageSize3D();
        Format         format      = Format::Unknown;
        SampleCount    sampleCount = SampleCount::Samples1;
        uint32_t       mipLevels   = 1;
        uint32_t       arrayCount  = 1;
    };

    struct RGAttachmentCreateInfo
    {
        TL::StringView name        = {};
        Format         format      = Format::Unknown;
        SampleCount    sampleCount = SampleCount::Samples1;
        StoreOperation storeOp     = StoreOperation::Store;
        ClearValue     clearValue  = {.f32 = {0.0f, 0.0f, 0.0f, 1.0f}};
        ResolveMode    resolveMode = ResolveMode::None;
    };

    struct RGBufferCreateInfo
    {
        TL::StringView name = {};
        size_t         size = 0;
    };

    class RHI_EXPORT RGPass
    {
        friend RenderGraph;

    public:
        RGPass(RenderGraph* rg, TL::StringView name, RGPassType type, ImageSize2D size2D);
        ~RGPass();

        TL::StringView name() const;

        /// @brief Creates a attachment for this pass.
        // TODO: should have different overloads for depth-stencil
        RGImage*       createRenderTarget(const RGAttachmentCreateInfo& ci);
        RGImage*       useRenderTarget(RGImage* image, const ImageSubresourceRange& range, LoadOperation loadOp, StoreOperation storeOp, ClearValue clearValue, ResolveMode resolveMode = ResolveMode::None);
        RGImage*       useRenderTarget(RGImage* image, LoadOperation loadOp = LoadOperation::Load, StoreOperation storeOp = StoreOperation::Store, ClearValue clearValue = {});

        /// @brief creates an image for this pass.
        RGImage*       createImage(const RGImageCreateInfo& ci, RGImageUsage usage);

        RGImage*       writeImage(RGImage* image, const ImageSubresourceRange& range, RGImageUsage usage);
        RGImage*       writeImage(RGImage* image, RGImageUsage usage);

        RGImage*       resolveImage(RGImage* src, const ImageSubresourceRange& range);
        RGImage*       resolveImage(RGImage* src);

        void           readImage(RGImage* image, const ImageSubresourceRange& range, RGImageUsage usage);
        void           readImage(RGImage* image, RGImageUsage usage);

        RGBuffer*      createBuffer(const RGBufferCreateInfo& ci, RGBufferUsage usage);

        RGBuffer*      readBuffer(RGBuffer* buffer, const BufferSubregion& subregion, RGBufferUsage usage);
        RGBuffer*      readBuffer(RGBuffer* buffer, RGBufferUsage usage);

        RGBuffer*      writeBuffer(RGBuffer* buffer, const BufferSubregion& subregion, RGBufferUsage usage);
        RGBuffer*      writeBuffer(RGBuffer* buffer, RGBufferUsage usage);

    private:
        void Execute(CommandList& commandList);

    private:
        TL::String                     m_name;
        RGPassType                     m_type;
        RenderGraph*                   m_renderGraph;
        PassExecuteCallback            m_executeCallback;

        uint32_t                       m_dependencyLevelIndex;
        uint32_t                       m_indexInUnorderedList;
        uint32_t                       m_executionQueueIndex;
        TL::Set<uint32_t>              m_producers;

        TL::Vector<RGImageDependency>  m_imageDependencies;
        TL::Vector<RGBufferDependency> m_bufferDependencies;

        // Graphics pass specific
        struct GfxPassInfo
        {
            GfxPassInfo(TL::IAllocator& allocator)
                : m_colorAttachments(allocator)
            {
            }

            ImageOffset2D                 m_offset;
            ImageSize2D                   m_size;
            TL::Vector<RGColorAttachment> m_colorAttachments;
            RGDepthStencilAttachment      m_depthStencilAttachment;
        };

        GfxPassInfo m_gfxPassInfo;

        struct Barriers
        {
            Barriers(TL::IAllocator& allocator)
                : memoryBarriers(allocator)
                , imageBarriers(allocator)
                , bufferBarriers(allocator)
            {
            }

            TL::Vector<BarrierInfo>       memoryBarriers;
            TL::Vector<ImageBarrierInfo>  imageBarriers;
            TL::Vector<BufferBarrierInfo> bufferBarriers;
        };

        Barriers m_barriers;

        struct State
        {
            bool culled;
            bool shouldCompile;
        };

        State m_state;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Resource Pool
    ///////////////////////////////////////////////////////////////////////////

    class RHI_EXPORT RenderGraphResourcePool
    {
    public:
        RenderGraphResourcePool()  = default;
        ~RenderGraphResourcePool() = default;

        ResultCode Init(Device* device);
        void       Shutdown();

        Image*     InitTransientImage(RGFrameImage* rgImage);
        Buffer*    InitTransientBuffer(RGFrameBuffer* rgBuffer);

        // Create or retrieve an image view for a given image and subresource range.
        // In this design, an image view is represented as an RGImage that aliases the parent image's resources.
        RGImage*   GetOrCreateImageView(RGFrameImage* parentImage, const ImageSubresourceRange& range);

    private:
        Device*                                                   m_device;
        TL::Map<TL::String, std::pair<ImageCreateInfo, Image*>>   m_imageCache;
        TL::Map<TL::String, std::pair<BufferCreateInfo, Buffer*>> m_bufferCache;
    };

    ///

    class StagingBuffer
    {
    public:
        static constexpr size_t DefaultCapacity = 32u * 1024u * 1024u; // 32 MiB per frame

        ResultCode              Init(Device* device, size_t capacity = DefaultCapacity);
        void                    Shutdown(Device* device);
        void                    Reset(); // call at the start of each frame

        struct Allocation
        {
            uint8_t* ptr    = nullptr;
            size_t   offset = 0;

            bool     isValid() const { return ptr != nullptr; }
        };

        Allocation Allocate(size_t size, size_t alignment = 256);

        Buffer*    GetBuffer() const { return m_buffer; }

    private:
        Buffer*  m_buffer   = nullptr;
        uint8_t* m_mapped   = nullptr;
        size_t   m_capacity = 0;
        size_t   m_offset   = 0;
    };

    ///

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    struct RGBeginInfo
    {
        bool        rdocDebugCapture   = false;
        TL::String* dumpGraphVizString = nullptr;
    };

    class RHI_EXPORT RenderGraph
    {
        friend Device;
        friend RGPass;
        friend RenderGraphBuilder;

    public:
        RenderGraph();
        ~RenderGraph();

        static RenderGraph*     create(Device* device, const RenderGraphCreateInfo& createInfo);

        static void             destroy(RenderGraph* renderGraph);

        RHI::Device*            getDevice() const { return m_device; }

        /// @brief Begins a frame recording.
        void                    BeginFrame(const RGBeginInfo& beginInfo = {});

        /// @brief Ends frame recording.
        void                    EndFrame();

        /// @brief Imports a swapchain image into the render graph.
        TL_NODISCARD RGImage*   importSwapchain(TL::StringView name, Swapchain& swapchain, Format format);

        /// @brief Imports an image into the render graph.
        /// @param initialState The barrier state the image is already in (e.g. CopyDst after streaming).
        TL_NODISCARD RGImage*   importImage(TL::StringView name, Image* image, Format format, ImageBarrierState initialState = {});

        /// @brief Imports a buffer into the render graph.
        /// @param initialState The barrier state the buffer is already in (e.g. CopyDst after streaming).
        TL_NODISCARD RGBuffer*  importBuffer(TL::StringView name, Buffer* buffer, BufferBarrierState initialState = {});

        /// @brief Adds a pass to the render graph.
        TL_MAYBE_UNUSED RGPass* addPass(TL::StringView name, RGPassType type, ImageSize2D size2D);
        void                    submitPass(RGPass* pass, PassExecuteCallback&& executeCallback);

        /// @brief Returns render graph resource's handle.
        TL_NODISCARD Image*     GetImageHandle(RGImage* handle) const;
        TL_NODISCARD Buffer*    GetBufferHandle(RGBuffer* handle) const;

        // streaming functions
        void                    streamBegin();
        void                    streamEnd();
        void                    streamBufferWrite(Buffer* buffer, size_t offset, TL::Block block);
        void                    streamImageWrite(Image* image, RHI::Format format, ImageOffset3D offset, ImageSize3D size, uint32_t mipLevel, uint32_t arrayLayer, TL::Block block);

        // Allocates a transient bind group for the current local bind group
        BindGroup*              createBindGroup(RHI::BindGroupLayout* layout);
        BufferBindingInfo*      allocateConstantBuffer(size_t size, uint8_t alignment);

    private:
        // Bind group stuff

        RGFrameImage*  CreateFrameImage(TL::StringView name);
        RGFrameBuffer* CreateFrameBuffer(TL::StringView name);

        RGImage*       EmplacePassImage(RGFrameImage* frameImage, RGPass* pass, ImageBarrierState initialState);
        RGBuffer*      EmplacePassBuffer(RGFrameBuffer* frameBuffer, RGPass* pass, BufferBarrierState initialState);

        bool           CheckDependency(const RGPass* producer, const RGPass* consumer) const;
        void           AddDependency(const RGPass* producer, RGPass* consumer);

        void           Compile();
        void           TopologicalSort(const TL::Vector<TL::Vector<uint32_t>>& adjacencyLists, TL::Vector<uint32_t>& sortedPasses);
        void           DepthFirstSearch(uint32_t nodeIndex, TL::Vector<bool>& visited, TL::Vector<bool>& onStack, bool& isCyclic, const TL::Vector<TL::Vector<uint32_t>>& adjacencyLists, TL::Vector<uint32_t>& sortedPasses);

        void           CreateTransientResources();

        void           PassBuildBarriers();

        void           ExecutePass(RGPass* pass, CommandList* commandList);

        void           Execute();
        void           ExecuteSingleThreadUsingSignelQueue();
        void           ExecuteSingleThreadUsingAsyncQueues();
        void           ExecuteMultithreadUsingSingleQueue();
        void           ExecuteMultithreadUsingAsyncQueues();

        void           DumpGraphViz();

    private:
        static constexpr uint32_t FramesInFlightCount = 2;

        RGBeginInfo               m_beginInfo;

        struct State
        {
            // States that are reset every frame
            bool compiled       : 1;
            bool frameRecording : 1;
        } m_state;

        struct DependencyLevel
        {
            DependencyLevel(TL::IAllocator& allocator, uint32_t m_index = 0)
                : m_levelIndex(m_index)
                , m_passes(allocator)
            {
            }

            void AddPass(RGPass* pass)
            {
                m_passes.push_back(pass);
            }

            TL::Span<RGPass* const> GetPasses() const
            {
                return m_passes;
            }

            uint32_t            m_levelIndex;
            TL::Vector<RGPass*> m_passes;
        };

        struct SwapchainRGInfo
        {
            Swapchain*    swapchain;
            RGFrameImage* frameImage;
        };

        Device*                          m_device;
        TL::Ptr<RenderGraphResourcePool> m_resourcePool;

        TL::Arena                        m_arena;
        TL::Vector<RGPass*>              m_passPool{m_arena};
        TL::Vector<RGFrameImage*>        m_imagePool{m_arena};
        TL::Vector<RGFrameBuffer*>       m_bufferPool{m_arena};
        TL::Vector<DependencyLevel>      m_dependencyLevels{m_arena};
        TL::Vector<SwapchainRGInfo>      m_swapchains{m_arena};

        // TL::Ptr<RenderDoc> m_rdoc;

        template<typename T>
        struct FreeList
        {
            uint32_t      head = 0;
            uint32_t      lastFrame; // rests the bind group if the
            TL::Vector<T> items;
        };

        TL::Map<RHI::BindGroupLayout*, FreeList<RHI::BindGroup*>> m_bindGroupsLookup;

        struct PerFrame
        {
            CommandPool* commandPool[(int)QueueType::Count];
        };

        // Streaming: pending upload operations collected between streamBegin/streamEnd
        struct PendingBufferWrite
        {
            Buffer* dstBuffer;
            size_t  dstOffset;
            size_t  stagingOffset;
            size_t  size;
        };

        struct PendingImageWrite
        {
            Image*        dstImage;
            ImageOffset3D imageOffset;
            ImageSize3D   imageSize;
            uint32_t      mipLevel;
            uint32_t      arrayLayer;
            size_t        stagingOffset;
            uint32_t      bytesPerRow;
        };

        bool                           m_streamingActive = false;
        TL::Vector<PendingBufferWrite> m_pendingBufferWrites{m_arena};
        TL::Vector<PendingImageWrite>  m_pendingImageWrites{m_arena};

        uint64_t                       m_activeFrame = 0;
        PerFrame                       m_frame[2];
        StagingBuffer                  m_stagingBuffer[FramesInFlightCount];

        struct PerQueue
        {
            Queue*   queue;
            Fence*   fence;
            uint64_t value;
        };

        PerQueue m_perQueue[(int)QueueType::Count];
    };
} // namespace RHI