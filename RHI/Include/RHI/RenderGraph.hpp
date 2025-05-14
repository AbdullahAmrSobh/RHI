#pragma once

#include "RHI/Device.hpp"
#include "RHI/Handle.hpp"
#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"
#include "RHI/PipelineAccess.hpp"
#include "RHI/CommandList.hpp"

#include <TL/Allocator/Mimalloc.hpp>
#include <TL/Containers.hpp>
#include <TL/UniquePtr.hpp>

namespace RHI
{
    class Device;
    class CommandList;
    class Swapchain;

    class RenderGraph;
    class RenderGraphBuilder;
    class RenderGraphContext;

    using PassSetupCallback   = TL::Function<void(RenderGraphBuilder& builder)>;
    using PassCompileCallback = TL::Function<void(RenderGraphContext& context)>;
    using PassExecuteCallback = TL::Function<void(CommandList& commandList)>;

    enum class PassType
    {
        Graphics,
        Compute,
        AsyncCompute,
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
    /// Render Graph Graphics Pass
    ///////////////////////////////////////////////////////////////////////////

    // Forward declarations for specialization
    struct RGImage;
    struct RGBuffer;

    // // Specialization for RGImage and RGBuffer
    // template<>
    // class Handle<RGImage>
    // {
    // public:
    //     Handle()
    //         : m_index(UINT32_MAX)
    //         , m_version(UINT32_MAX)
    //     {
    //     }

    //     Handle(NullHandle_T)
    //         : m_index(UINT32_MAX)
    //         , m_version(UINT32_MAX)
    //     {
    //     }

    //     inline bool operator==(Handle other) const { return m_index == other.m_index && m_version == other.m_version; }

    //     inline bool operator!=(Handle other) const { return !(*this == other); }

    //     inline bool operator==(const NullHandle_T&) const { return m_index == UINT32_MAX; }

    //     inline      operator bool() const { return *this != NullHandle; }

    // private:
    //     friend class HandlePool<RGImage>;

    //     Handle(uint32_t index, uint32_t version)
    //         : m_index(index)
    //         , m_version(version)
    //     {
    //     }

    //     uint32_t m_index;
    //     uint32_t m_version;
    // };

    // template<>
    // class Handle<RGBuffer>
    // {
    // public:
    //     Handle()
    //         : m_index(UINT32_MAX)
    //         , m_version(UINT32_MAX)
    //     {
    //     }

    //     Handle(NullHandle_T)
    //         : m_index(UINT32_MAX)
    //         , m_version(UINT32_MAX)
    //     {
    //     }

    //     inline bool operator==(Handle other) const { return m_index == other.m_index && m_version == other.m_version; }

    //     inline bool operator!=(Handle other) const { return !(*this == other); }

    //     inline bool operator==(const NullHandle_T&) const { return m_index == UINT32_MAX; }

    //     inline      operator bool() const { return *this != NullHandle; }

    // private:
    //     friend class HandlePool<RGBuffer>;

    //     Handle(uint32_t index, uint32_t version)
    //         : m_index(index)
    //         , m_version(version)
    //     {
    //     }

    //     uint32_t m_index;
    //     uint32_t m_version;
    // };

    struct RGImage;
    struct RGBuffer;

    struct RGColorAttachment
    {
        Handle<RGImage>       color        = {};
        ImageSubresourceRange colorRange   = ImageSubresourceRange::All();
        LoadOperation         loadOp       = LoadOperation::Discard;
        StoreOperation        storeOp      = StoreOperation::Store;
        ClearValue            clearValue   = {.f32 = {0.0f, 0.0f, 0.0f, 1.0f}};
        ResolveMode           resolveMode  = ResolveMode::None;
        Handle<RGImage>       resolveView  = {};
        ImageSubresourceRange resolveRange = ImageSubresourceRange::All();
    };

    struct RGDepthStencilAttachment
    {
        Handle<RGImage>       depthStencil      = {};
        ImageSubresourceRange depthStencilRange = ImageSubresourceRange::All();
        LoadOperation         depthLoadOp       = LoadOperation::Discard;
        StoreOperation        depthStoreOp      = StoreOperation::Store;
        LoadOperation         stencilLoadOp     = LoadOperation::Discard;
        StoreOperation        stencilStoreOp    = StoreOperation::Store;
        DepthStencilValue     clearValue        = {0.0f, 0};
    };

    struct RGRenderPassBeginInfo
    {
        ImageSize2D                            size                   = {};
        ImageOffset2D                          offset                 = {};
        TL::Span<const RGColorAttachment>      colorAttachments       = {};
        TL::Optional<RGDepthStencilAttachment> depthStencilAttachment = {};
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Resources
    ///////////////////////////////////////////////////////////////////////////

    class RGPass;

#if 0

    struct RGFrameImage
    {
        struct Desc
        {
            TL::Flags<ImageUsage> usageFlags;
            ImageType             type;
            ImageSize3D           size;
            Format                format;
            SampleCount           sampleCount;
            uint32_t              mipLevels;
            uint32_t              arrayCount;
        };

        TL::String                       m_name;
        Handle<Image>                    m_handle;
        Desc                             m_desc;
        bool                             m_isImported = false;

        // Cache used to allocate image views
        TL::Map<uint64_t, Handle<Image>> m_views;

        RGFrameImage(const char* name, const Desc& desc)
            : m_name(name)
            , m_handle(NullHandle)
            , m_desc(desc)
            , m_isImported(false)
        {
        }

        RGFrameImage(const char* name, Handle<Image> handle, const Desc& desc)
            : m_name(name)
            , m_handle(handle)
            , m_desc(desc)
            , m_isImported(true)
        {
        }

        const char*   GetName() const { return m_name.c_str(); }

        Handle<Image> GetHandle() const { return m_handle; }

        Handle<Image> GetView(const ImageSubresourceRange& subresource);
    };

    struct RGFrameBuffer
    {
        struct Desc
        {
            TL::Flags<BufferUsage> usageFlags;
            size_t                 size;
        };

        TL::String     m_name;
        Handle<Buffer> m_handle;
        Desc           m_desc;
        bool           m_isImported = false;

        RGFrameBuffer(const char* name, size_t size)
            : m_name(name)
            , m_handle(NullHandle)
            , m_desc({.size = size})
            , m_isImported(false)
        {
        }

        RGFrameBuffer(const char* name, Handle<Buffer> handle, const Desc& desc)
            : m_name(name)
            , m_handle(handle)
            , m_desc(desc)
            , m_isImported(true)
        {
        }

        const char*    GetName() const { return m_name.c_str(); }

        Handle<Buffer> GetHandle() const { return m_handle; }
    };

#endif

    struct LifetimeInterval
    {
        uint32_t begin;
        uint32_t end;

        // return true if the lifetime is valid for this dep level
        bool     IsValid(uint32_t level) const;
    };

    struct RGResource
    {
        TL::String       m_name;
        RGPass*          m_producer;
        LifetimeInterval m_lifetime;
        bool             m_isValid;
        bool             m_isImported;

        RGResource(const char* name)
            : m_name(name)
            , m_producer(nullptr)
            , m_lifetime{}
            , m_isValid(true)
            , m_isImported(false)
        {
        }

        RGResource(const RGResource&)            = default;
        RGResource& operator=(const RGResource&) = default;
        virtual ~RGResource()                    = default;
    };

    struct RGImage : RGResource
    {
        RGImage(const char* name, ImageType type, ImageSize3D size, Format format, uint32_t mipLevels, uint32_t arrayCount, SampleCount samples)
            : RGResource(name)
        {
            m_desc.usageFlags  = {};
            m_desc.type        = type;
            m_desc.size        = size;
            m_desc.format      = format;
            m_desc.sampleCount = samples;
            m_desc.mipLevels   = mipLevels;
            m_desc.arrayCount  = arrayCount;
            m_handle           = NullHandle;
            m_state            = {};
            m_prevHandle       = {};
            m_nextHandle       = {};
            m_activeState      = {};
        }

        RGImage(const char* name, Handle<Image> handle, Format format)
            : RGResource(name)
        {
            m_desc.usageFlags  = {};
            m_desc.type        = ImageType::Image2D;
            m_desc.size        = {};
            m_desc.format      = format;
            m_desc.sampleCount = SampleCount::Samples1;
            m_desc.mipLevels   = 1;
            m_desc.arrayCount  = 1;
            m_handle           = handle;
            m_state            = {};
            m_prevHandle       = {};
            m_nextHandle       = {};
            m_activeState      = {};
        }

        ~RGImage() = default;

        struct
        {
            TL::Flags<ImageUsage> usageFlags;
            ImageType             type;
            ImageSize3D           size;
            Format                format;
            SampleCount           sampleCount;
            uint32_t              mipLevels;
            uint32_t              arrayCount;
        } m_desc;

        Handle<Image>     m_handle = NullHandle;
        ImageBarrierState m_state  = {}; // initial state
        Handle<RGImage>   m_prevHandle;
        Handle<RGImage>   m_nextHandle;

        ImageBarrierState m_activeState; // Resource state used during pass build

        size_t GetHash() const;
    };

    struct RGBuffer : RGResource
    {
        RGBuffer(const char* name, size_t size)
            : RGResource(name)
        {
            m_desc.usageFlags = {};
            m_desc.size       = size;
            m_handle          = NullHandle;
            m_state           = {};
            m_prevHandle      = {};
            m_nextHandle      = {};
            m_activeState     = {};
        }

        RGBuffer(const char* name, Handle<Buffer> handle)
            : RGResource(name)
        {
            m_desc.usageFlags = {};
            m_desc.size       = 0;
            m_handle          = handle;
            m_state           = {};
            m_prevHandle      = {};
            m_nextHandle      = {};
            m_activeState     = {};
        }

        ~RGBuffer() = default;

        struct
        {
            TL::Flags<BufferUsage> usageFlags;
            size_t                 size;
        } m_desc;

        Handle<Buffer>     m_handle = NullHandle;
        BufferBarrierState m_state  = {}; // initial state
        Handle<RGBuffer>   m_prevHandle;
        Handle<RGBuffer>   m_nextHandle;

        BufferBarrierState m_activeState; // Resource state used during pass build

        size_t GetHash() const;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Builder Interface
    ///////////////////////////////////////////////////////////////////////////

    class RHI_EXPORT RenderGraphBuilder
    {
        friend RenderGraph;

        RenderGraphBuilder(RenderGraph* rg, RGPass* pass);

    public:
        /// @brief Declare pass image read dependencey.
        void             ReadImage(Handle<RGImage> image, ImageUsage usage, PipelineStage stage);
        void             ReadImage(Handle<RGImage> image, const ImageSubresourceRange& subresource, ImageUsage usage, PipelineStage stage);

        /// @brief Declare pass image write dependencey.
        Handle<RGImage>  WriteImage(Handle<RGImage> image, ImageUsage usage, PipelineStage stage);
        Handle<RGImage>  WriteImage(Handle<RGImage> image, const ImageSubresourceRange& subresource, ImageUsage usage, PipelineStage stage);

        /// @brief Declare pass buffer read dependencey.
        void             ReadBuffer(Handle<RGBuffer> buffer, BufferUsage usage, PipelineStage stage);
        void             ReadBuffer(Handle<RGBuffer> buffer, const BufferSubregion& subresource, BufferUsage usage, PipelineStage stage);

        /// @brief Declare pass buffer write dependencey.
        Handle<RGBuffer> WriteBuffer(Handle<RGBuffer> buffer, BufferUsage usage, PipelineStage stage);
        Handle<RGBuffer> WriteBuffer(Handle<RGBuffer> buffer, const BufferSubregion& subresource, BufferUsage usage, PipelineStage stage);

        /// Graphics Pass specfic
        Handle<RGImage>  AddColorAttachment(RGColorAttachment attachment);
        Handle<RGImage>  SetDepthStencil(RGDepthStencilAttachment attachment);

    private:
        Handle<RGImage>  UseImageInternal(Handle<RGImage> handle, HandlePool<RGImage>& pool, Access access, const ImageSubresourceRange& subresource, ImageUsage usage, PipelineStage stage);
        Handle<RGBuffer> UseBufferInternal(Handle<RGBuffer> handle, HandlePool<RGBuffer>& pool, Access access, const BufferSubregion& subresource, BufferUsage usage, PipelineStage stage);

    private:
        RenderGraph* m_rg;
        RGPass*      m_pass;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Context Interface
    ///////////////////////////////////////////////////////////////////////////

    class RHI_EXPORT RenderGraphContext
    {
        friend RenderGraph;

        RenderGraphContext(RenderGraph* rg, RGPass* pass);

    public:
        Handle<Image>  GetImage(Handle<RGImage> handle) const;
        Handle<Buffer> GetBuffer(Handle<RGBuffer> handle) const;

    private:
        RenderGraph* m_rg;
        RGPass*      m_pass;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Pass
    ///////////////////////////////////////////////////////////////////////////

    struct RGImageDependency
    {
        Handle<RGImage>       image;
        uint32_t              viewID;
        ImageBarrierState     state;
    };

    struct RGBufferDependency
    {
        Handle<RGBuffer>   buffer;
        BufferSubregion    subregion;
        BufferBarrierState state;
    };

    class RHI_EXPORT RGPass
    {
        friend RenderGraph;
        friend RenderGraphBuilder;
        friend RenderGraphContext;

    public:
        RGPass();
        ~RGPass();

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

        const char* GetName() const { return m_name.c_str(); }

    private:
        void Setup(RenderGraphBuilder& builder);
        void Compile(RenderGraphContext& context);
        void Execute(CommandList& commandList);

    private:
        RenderGraph*                   m_renderGraph;
        TL::String                     m_name;
        PassType                       m_type;
        PassSetupCallback              m_setupCallback;
        PassCompileCallback            m_compileCallback;
        PassExecuteCallback            m_executeCallback;
        uint32_t                       m_dependencyLevelIndex;
        uint32_t                       m_indexInUnorderedList;
        uint32_t                       m_executionQueueIndex;

        TL::Vector<RGImageDependency>  m_imageDependencies;
        TL::Vector<RGBufferDependency> m_bufferDependencies;

        TL::Set<uint32_t>              m_producers;

        struct GfxPassInfo
        {
            ImageOffset2D                          m_offset;
            ImageSize2D                            m_size;
            TL::Vector<RGColorAttachment>          m_colorAttachments;
            TL::Optional<RGDepthStencilAttachment> m_depthStencilAttachment;
        };

        GfxPassInfo m_gfxPassInfo;

        struct Barriers
        {
            TL::Vector<BarrierInfo>       memoryBarriers;
            TL::Vector<ImageBarrierInfo>  imageBarriers;
            TL::Vector<BufferBarrierInfo> bufferBarriers;
        };

        Barriers m_barriers[BarrierSlot::Count];

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

        ResultCode      Init(Device* device);
        void            Shutdown();

        Handle<Image>   InitTransientImage(RGImage* rgImage);
        Handle<Buffer>  InitTransientBuffer(RGBuffer* rgBuffer);

        // Create or retrieve an image view for a given image and subresource range.
        // In this design, an image view is represented as an RGImage that aliases the parent image's resources.
        Handle<RGImage> GetOrCreateImageView(Handle<RGImage> parentImage, const ImageSubresourceRange& range);

    private:
        Device*                                                          m_device;
        TL::Map<TL::String, std::pair<ImageCreateInfo, Handle<Image>>>   m_imageCache;
        TL::Map<TL::String, std::pair<BufferCreateInfo, Handle<Buffer>>> m_bufferCache;

        // // Track created image views: map of parent RGImage handle to a map of subresource range to RGImage view handle
        // // When the original RGImage is destroyed, all views are invalidated.
        // TL::Map<Handle<RGImage>, TL::Map<ImageViewCreateInfo, Handle<RGImage>>> m_imageViewCache;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    class RHI_EXPORT RenderGraph
    {
        friend Device;
        friend RGPass;
        friend RenderGraphBuilder;
        friend RenderGraphContext;

        class DependencyLevel;
        class ExecuteGroup;

    public:
        RenderGraph();
        ~RenderGraph();

        /// @brief Initializes the render graph.
        TL_MAYBE_UNUSED ResultCode    Init(Device* device, const RenderGraphCreateInfo& ci);

        /// @brief Shutdown the render graph.
        void                          Shutdown();

        void                          BeginFrame(ImageSize2D frameSize);
        void                          EndFrame();

        /// @brief Imports a swapchain image into the render graph.
        TL_NODISCARD Handle<RGImage>  ImportSwapchain(const char* name, Swapchain& swapchain, Format format);

        /// @brief Imports an image into the render graph.
        TL_NODISCARD Handle<RGImage>  ImportImage(const char* name, Handle<Image> image, Format format);

        /// @brief Imports a buffer into the render graph.
        TL_NODISCARD Handle<RGBuffer> ImportBuffer(const char* name, Handle<Buffer> buffer);

        /// @brief Creates a transient image in the render graph.
        TL_NODISCARD Handle<RGImage>  CreateImage(const char* name, ImageType type, ImageSize3D size, Format format, uint32_t mipLevels = 1, uint32_t arrayCount = 1, SampleCount samples = SampleCount::Samples1);

        /// @brief Creates a transient render target image in the render graph.
        TL_NODISCARD Handle<RGImage>  CreateRenderTarget(const char* name, ImageSize2D size, Format format, uint32_t mipLevels = 1, uint32_t arrayCount = 1, SampleCount samples = SampleCount::Samples1);

        /// @brief Creates a transient buffer in the render graph.
        TL_NODISCARD Handle<RGBuffer> CreateBuffer(const char* name, size_t size);

        /// @brief Adds a pass to the render graph.
        TL_MAYBE_UNUSED RGPass*       AddPass(const PassCreateInfo& createInfo);

        void                          Dump();

        TL_NODISCARD ImageSize2D      GetFrameSize() const;
        TL_NODISCARD Handle<Image>    GetImageHandle(Handle<RGImage> handle) const;
        TL_NODISCARD Handle<Buffer>   GetBufferHandle(Handle<RGBuffer> handle) const;

    private:
        // TODO: Remove this section
        void             ExtendImageUsage(RGImage* imageBefore, ImageUsage usage);
        void             ExtendBufferUsage(RGBuffer* bufferBefore, BufferUsage usage);
        Handle<RGImage>  CreateRGImageHandle(Handle<RGImage> imageBefore, RGPass* producer);
        Handle<RGBuffer> CreateRGBufferHandle(Handle<RGBuffer> bufferBefore, RGPass* producer);

    private:
        bool CheckHandleIsValid(Handle<RGImage> image) const;
        bool CheckHandleIsValid(Handle<RGBuffer> buffer) const;

        bool CheckDependency(const RGPass* producer, const RGPass* consumer) const;
        void AddDependency(const RGPass* producer, RGPass* consumer);

        void Compile();
        void BuildAdjacencyLists(TL::Vector<TL::Vector<uint32_t>>& adjacencyLists);
        void TopologicalSort(const TL::Vector<TL::Vector<uint32_t>>& adjacencyLists, TL::Vector<uint32_t>& sortedPasses);
        void DepthFirstSearch(uint32_t nodeIndex, TL::Vector<bool>& visited, TL::Vector<bool>& onStack, bool& isCyclic, const TL::Vector<TL::Vector<uint32_t>>& adjacencyLists, TL::Vector<uint32_t>& sortedPasses);
        void BuildDependencyLevels(TL::Span<const uint32_t> sortedPasses, const TL::Vector<TL::Vector<uint32_t>>& adjacencyLists, TL::Vector<DependencyLevel>& dependencyLevels, uint32_t& detectedQueueCount);

        void CreateTransientResources();

        void PassBuildBarriers();

        void Execute();
        void ExecutePass(RGPass* pass, CommandList* commandList);

    private:
        struct State // States that are reset every frame
        {
            bool compiled       : 1;
            bool frameRecording : 1;
            bool dumpGraphviz   : 1;
        } m_state;

        TL::IAllocator*                  m_allocator;
        TL::Arena*                       m_tempAllocator;
        Device*                          m_device;
        uint64_t                         m_frameIndex;
        TL::Ptr<RenderGraphResourcePool> m_resourcePool;
        ImageSize2D                      m_frameSize;
        Swapchain*                       m_swapchain;
        TL::Vector<TL::Ptr<RGPass>>      m_passPool;
        HandlePool<RGImage>              m_imagePool;
        TL::Vector<Handle<RGImage>>      m_imageList;
        HandlePool<RGBuffer>             m_bufferPool;
        TL::Vector<Handle<RGBuffer>>     m_bufferList;
        TL::Vector<DependencyLevel>      m_dependencyLevels;
    };
} // namespace RHI