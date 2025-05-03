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
    class CommandList;
    class Swapchain;

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
        Handle<RGImage>       depthStencil   = {};
        ImageSubresourceRange depthStencilRange     = ImageSubresourceRange::All();
        LoadOperation         depthLoadOp    = LoadOperation::Discard;
        StoreOperation        depthStoreOp   = StoreOperation::Store;
        LoadOperation         stencilLoadOp  = LoadOperation::Discard;
        StoreOperation        stencilStoreOp = StoreOperation::Store;
        DepthStencilValue     clearValue     = {0.0f, 0};
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
    struct PassDependency;
    struct ImagePassDependency;
    struct BufferPassDependency;
    class RGPass;

    struct RGResource
    {

    };

    struct RGImage : RGResource
    {
        RGImage(const char* name, ImageType type, ImageSize3D size, Format format, uint32_t mipLevels, uint32_t arrayCount, SampleCount samples);
        RGImage(const char* name, Handle<Image> handle, Format format);
        ~RGImage() = default;

        TL::String m_name     = "";
        RGPass*    m_pass     = nullptr;
        bool       m_valid    = true;
        bool       m_imported = false;

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

        Handle<RGImage> m_prevHandle;
        Handle<RGImage> m_nextHandle;
    };

    struct RGBuffer : RGResource
    {
        RGBuffer(const char* name, size_t size);
        RGBuffer(const char* name, Handle<Buffer> handle);
        ~RGBuffer() = default;

        TL::String m_name     = "";
        RGPass*    m_pass     = nullptr;
        bool       m_valid    = true;
        bool       m_imported = false;

        struct
        {
            TL::Flags<BufferUsage> usageFlags;
            size_t                 size;
        } m_desc;

        Handle<Buffer>     m_handle = NullHandle;
        BufferBarrierState m_state  = {}; // initial state


        Handle<RGBuffer> m_prevHandle;
        Handle<RGBuffer> m_nextHandle;
    };

    struct RGImageDependency
    {
        Handle<RGImage>       image;
        ImageSubresourceRange subresources;
        ImageUsage            usage;
        PipelineStage         stage;
        Access                access;
    };

    struct RGBufferDependency
    {
        Handle<RGBuffer> buffer;
        BufferSubregion  subregion;
        BufferUsage      usage;
        PipelineStage    stage;
        Access           access;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Builder Interface
    ///////////////////////////////////////////////////////////////////////////

    class RGPass;

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

        size_t      GetHash() const;

    private:
        void Setup(RenderGraphBuilder& builder);
        void Compile(RenderGraphContext& context);
        void Execute(CommandList& commandList);

        void AddProducer(TL_MAYBE_UNUSED RGPass* pass) {}

    private:
        RenderGraph*                   m_renderGraph;
        TL::String                     m_name;
        PassType                       m_type;
        PassSetupCallback              m_setupCallback;
        PassCompileCallback            m_compileCallback;
        PassExecuteCallback            m_executeCallback;
        uint32_t                       m_globalExecutionIndex;
        uint32_t                       m_dependencyLevelIndex;
        uint32_t                       m_localToDependencyLevelExecutionIndex;
        uint32_t                       m_localToQueueExecutionIndex;
        uint32_t                       m_indexInUnorderedList;
        uint32_t                       m_executionQueueIndex;
        TL::Vector<RGPass*>            m_producers;
        TL::Vector<Handle<RGImage>>    m_imageWrites;
        TL::Vector<Handle<RGBuffer>>   m_bufferWrites;

        TL::Vector<RGImageDependency>  m_imageDependencies;
        TL::Vector<RGBufferDependency> m_bufferDependencies;

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

        struct
        {
            bool culled;
            bool shouldCompile;
        } m_state;
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

        class TransientResourceAllocator;
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

        void                          QueueBufferRead(Handle<RGBuffer> buffer, uint32_t offset, TL::Block data);
        Handle<RGBuffer>              QueueBufferWrite(Handle<RGBuffer> buffer, uint32_t offset, TL::Block data);
        void                          QueueImageRead(Handle<RGImage> image, ImageOffset3D offset, ImageSize3D size, ImageSubresourceLayers dstLayers, TL::Block block);
        Handle<RGImage>               QueueImageWrite(Handle<RGImage> image, ImageOffset3D offset, ImageSize3D size, ImageSubresourceLayers dstLayers, TL::Block block);

    private:
        TL_NODISCARD ImageSize2D     GetFrameSize() const;
        TL_NODISCARD const RGImage*  GetImage(Handle<RGImage> handle) const;
        TL_NODISCARD RGImage*        GetImage(Handle<RGImage> handle);
        TL_NODISCARD const RGBuffer* GetBuffer(Handle<RGBuffer> handle) const;
        TL_NODISCARD RGBuffer*       GetBuffer(Handle<RGBuffer> handle);
        TL_NODISCARD Handle<Image>   GetImageHandle(Handle<RGImage> handle) const;
        TL_NODISCARD Handle<Buffer>  GetBufferHandle(Handle<RGBuffer> handle) const;

    private:
        // return true if src depends on
        bool IsNameValid(const char* name) const;
        bool CheckHandleIsValid(Handle<RGImage> image) const;
        bool CheckHandleIsValid(Handle<RGBuffer> buffer) const;
        bool CheckDependency(const RGPass* producer, const RGPass* consumer) const;
        void AddDependency(const RGPass* producer, RGPass* consumer);

        void Compile();

        void BuildAdjacencyLists(TL::Vector<TL::Vector<uint32_t>>& adjacencyLists);
        void DepthFirstSearch(
            uint32_t                                nodeIndex,
            TL::Vector<bool>&                       visited,
            TL::Vector<bool>&                       onStack,
            bool&                                   isCyclic,
            const TL::Vector<TL::Vector<uint32_t>>& adjacencyLists,
            TL::Vector<uint32_t>&                   sortedPasses);
        void TopologicalSort(const TL::Vector<TL::Vector<uint32_t>>& adjacencyLists, TL::Vector<uint32_t>& sortedPasses);
        void BuildDependencyLevels(TL::Span<const uint32_t> sortedPasses, const TL::Vector<TL::Vector<uint32_t>>& adjacencyLists, TL::Vector<DependencyLevel>& dependencyLevels, uint32_t& detectedQueueCount);

        // Execute

        void Execute();
        void ExecutePass(RGPass* pass, CommandList* commandList);
        void ExecuteSerialSingleThreaded();

    private:
        TL::IAllocator* m_allocator;
        TL::Arena*      m_tempAllocator;
        Device*         m_device;

        uint64_t        m_frameIndex;

        struct State // States that are reset every frame
        {
            bool compiled       : 1;
            bool frameRecording : 1;
            bool dumpGraphviz   : 1;
        } m_state;

        ImageSize2D                 m_frameSize;

        Swapchain*                  m_swapchain;

        TL::Vector<TL::Ptr<RGPass>> m_passPool;
        HandlePool<RGImage>         m_imagePool;
        HandlePool<RGBuffer>        m_bufferPool;

        TL::Vector<DependencyLevel> m_dependencyLevels;
        TL::Vector<bool>            m_dependencyTable;
    };
} // namespace RHI
