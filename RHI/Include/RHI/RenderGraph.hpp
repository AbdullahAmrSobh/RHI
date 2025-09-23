#pragma once

#include "RHI/Device.hpp"
#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"
#include "RHI/PipelineAccess.hpp"
#include "RHI/CommandList.hpp"

#include <TL/Allocator/Arena.hpp>
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
    };

    struct PassCreateInfo
    {
        const char*         name;
        PassType            type;
        ImageSize2D         size;
        PassSetupCallback   setupCallback;
        PassExecuteCallback executeCallback;
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
        // Image*     m_handle        = nullptr;
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
    /// Render Graph Builder Interface
    ///////////////////////////////////////////////////////////////////////////

    class RHI_EXPORT RenderGraphBuilder
    {
        friend RenderGraph;

        RenderGraphBuilder(RenderGraph* rg, RGPass* pass);

    public:
        /// @brief Declare pass image read dependencey.
        void      ReadImage(RGImage* image, ImageUsage usage, PipelineStage stage);
        void      ReadImage(RGImage* image, const ImageSubresourceRange& subresource, ImageUsage usage, PipelineStage stage);

        /// @brief Declare pass image write dependencey.
        RGImage*  WriteImage(RGImage* image, ImageUsage usage, PipelineStage stage);
        RGImage*  WriteImage(RGImage* image, const ImageSubresourceRange& subresource, ImageUsage usage, PipelineStage stage);

        /// @brief Declare pass buffer read dependencey.
        void      ReadBuffer(RGBuffer* buffer, BufferUsage usage, PipelineStage stage);
        void      ReadBuffer(RGBuffer* buffer, const BufferSubregion& subresource, BufferUsage usage, PipelineStage stage);

        /// @brief Declare pass buffer write dependencey.
        RGBuffer* WriteBuffer(RGBuffer* buffer, BufferUsage usage, PipelineStage stage);
        RGBuffer* WriteBuffer(RGBuffer* buffer, const BufferSubregion& subresource, BufferUsage usage, PipelineStage stage);

        /// Graphics Pass specfic
        RGImage*  AddColorAttachment(RGColorAttachment attachment);
        RGImage*  SetDepthStencil(RGDepthStencilAttachment attachment);

        RGImage*  AddColorAttachment(RGImage* target, LoadOperation loadOp, ClearValue clear = {});
        RGImage*  CreateColorTarget(const char* name, ImageSize2D size, Format format, ClearValue clear = {});

        RGBuffer*  CreateBuffer(const char* name, size_t size, BufferUsage usage, PipelineStage stage);

    private:
        RGImage*  UseImageInternal(RGImage* handle, Access access, const ImageSubresourceRange& subresource, ImageUsage usage, PipelineStage stage);
        RGBuffer* UseBufferInternal(RGBuffer* handle, Access access, const BufferSubregion& subresource, BufferUsage usage, PipelineStage stage);

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
        /// @brief Enum representing different barrier slots for a pass.
        enum BarrierSlot
        {
            Prilogue,
            Epilogue,
            Resolve,
            Count,
        };

        RGPass(RenderGraph* rg, const PassCreateInfo& ci);
        ~RGPass();

        const char* GetName() const { return m_name; }

    private:
        void Setup(RenderGraphBuilder& builder);
        void Execute(CommandList& commandList);

    public:
        /// @brief Declare pass image read dependencey.
        void      ReadImage(RGImage* image, ImageUsage usage, PipelineStage stage);
        void      ReadImage(RGImage* image, const ImageSubresourceRange& subresource, ImageUsage usage, PipelineStage stage);

        /// @brief Declare pass image write dependencey.
        RGImage*  WriteImage(RGImage* image, ImageUsage usage, PipelineStage stage);
        RGImage*  WriteImage(RGImage* image, const ImageSubresourceRange& subresource, ImageUsage usage, PipelineStage stage);

        /// @brief Declare pass buffer read dependencey.
        void      ReadBuffer(RGBuffer* buffer, BufferUsage usage, PipelineStage stage);
        void      ReadBuffer(RGBuffer* buffer, const BufferSubregion& subresource, BufferUsage usage, PipelineStage stage);

        /// @brief Declare pass buffer write dependencey.
        RGBuffer* WriteBuffer(RGBuffer* buffer, BufferUsage usage, PipelineStage stage);
        RGBuffer* WriteBuffer(RGBuffer* buffer, const BufferSubregion& subresource, BufferUsage usage, PipelineStage stage);

        /// Graphics Pass specfic
        RGImage*  AddColorAttachment(RGColorAttachment attachment);
        RGImage*  SetDepthStencil(RGDepthStencilAttachment attachment);

        RGImage*  AddColorAttachment(RGImage* target, LoadOperation loadOp, ClearValue clear = {})
        {
            RGColorAttachment attachment{
                .color      = target,
                .loadOp     = loadOp,
                .storeOp    = StoreOperation::Store,
                .clearValue = clear,
            };
            return AddColorAttachment(attachment);
        }

    private:
        const char*                    m_name;
        PassType                       m_type;
        RenderGraph*                   m_renderGraph;
        PassSetupCallback              m_setupCallback;
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

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    class RHI_EXPORT RenderGraph
    {
        friend Device;
        friend RGPass;
        friend RenderGraphBuilder;
        friend RenderGraphContext;

    public:
        RenderGraph();
        ~RenderGraph();

        /// @brief Initializes the render graph.
        TL_MAYBE_UNUSED ResultCode Init(Device* device, const RenderGraphCreateInfo& ci);

        /// @brief Shutdown the render graph.
        void                       Shutdown();

        /// @brief Capture next rendered frame.
        void                       Debug_CaptureNextFrame();

        /// @brief Prints graph debug information to console.
        void                       Dump();

        /// @brief Begins a frame recording.
        void                       BeginFrame();

        /// @brief Ends frame recording.
        void                       EndFrame();

        /// @brief Imports a swapchain image into the render graph.
        TL_NODISCARD RGImage*      ImportSwapchain(const char* name, Swapchain& swapchain, Format format);

        /// @brief Imports an image into the render graph.
        TL_NODISCARD RGImage*      ImportImage(const char* name, Image* image, Format format);

        /// @brief Imports a buffer into the render graph.
        TL_NODISCARD RGBuffer*     ImportBuffer(const char* name, Buffer* buffer);

        /// @brief Creates a transient image in the render graph.
        TL_NODISCARD RGImage*      CreateImage(const char* name, ImageType type, ImageSize3D size, Format format, uint32_t mipLevels = 1, uint32_t arrayCount = 1, SampleCount samples = SampleCount::Samples1);

        /// @brief Creates a transient render target image in the render graph.
        TL_NODISCARD RGImage*      CreateRenderTarget(const char* name, ImageSize2D size, Format format, uint32_t mipLevels = 1, uint32_t arrayCount = 1, SampleCount samples = SampleCount::Samples1);

        /// @brief Creates a transient buffer in the render graph.
        TL_NODISCARD RGBuffer*     CreateBuffer(const char* name, size_t size);

        /// @brief Adds a pass to the render graph.
        TL_MAYBE_UNUSED RGPass*    AddPass(const PassCreateInfo& createInfo);

        /// @brief Returns render graph resource's handle.
        TL_NODISCARD Image*        GetImageHandle(RGImage* handle) const;
        TL_NODISCARD Buffer*       GetBufferHandle(RGBuffer* handle) const;

    private:
        // Bind group stuff

        /// Gets or creates a new bind group with provided resources.
        BindGroup*      AllocateBindGroup(BindGroupLayout* layout, const BindGroupUpdateInfo& updateInfo);

        RGFrameImage*   CreateFrameImage(const char* name);
        RGFrameBuffer*  CreateFrameBuffer(const char* name);

        RGImage*        EmplacePassImage(RGFrameImage* frameImage, RGPass* pass, ImageBarrierState initialState);
        RGBuffer*       EmplacePassBuffer(RGFrameBuffer* frameBuffer, RGPass* pass, BufferBarrierState initialState);

        TL::IAllocator& GetFrameAllocator();

        bool            CheckDependency(const RGPass* producer, const RGPass* consumer) const;
        void            AddDependency(const RGPass* producer, RGPass* consumer);

        void            Compile();
        void            TopologicalSort(const TL::Vector<TL::Vector<uint32_t>>& adjacencyLists, TL::Vector<uint32_t>& sortedPasses);
        void            DepthFirstSearch(uint32_t nodeIndex, TL::Vector<bool>& visited, TL::Vector<bool>& onStack, bool& isCyclic, const TL::Vector<TL::Vector<uint32_t>>& adjacencyLists, TL::Vector<uint32_t>& sortedPasses);

        void            CreateTransientResources();

        void            PassBuildBarriers();

        void            Execute();
        void            ExecutePass(RGPass* pass, CommandList* commandList);

    private:
        struct State
        {
            // States that are reset every frame
            bool compiled                      : 1;
            bool frameRecording                : 1;
            bool dumpGraphviz                  : 1;
            bool debug_triggerNextFrameCapture : 1;
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

        Device*                               m_device;
        Frame*                                m_activeFrame;
        TL::Ptr<RenderGraphResourcePool>      m_resourcePool;
        TL::Arena                             m_arena;
        TL::Vector<SwapchainImageAcquireInfo> m_swapchains{m_arena};
        TL::Vector<RGPass*>                   m_passPool{m_arena};
        TL::Vector<RGFrameImage*>             m_imagePool{m_arena};
        TL::Vector<RGFrameBuffer*>            m_bufferPool{m_arena};
        TL::Vector<DependencyLevel>           m_dependencyLevels{m_arena};
    };
} // namespace RHI