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
    struct RenderTargetInfo;
    class Device;
    class Pass;
    class RenderGraph;
    class CommandList;
    class Swapchain;
    class RenderGraphExecuteGroup;

    // Based on (https://anki3d.org/simplified-pipeline-barriers/)

    enum class PassType
    {
        Graphics,
        Compute,
        Transfer,
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Resource Data
    ///////////////////////////////////////////////////////////////////////////

    struct RenderGraphResource
    {
        // empty for now
    };

    class RGResourcePool
    {
    public:
        Handle<Image>                               m_handle;
    };

    struct RenderGraphImage : RenderGraphResource
    {
        TL::String            m_name;
        TL::Flags<ImageUsage> m_usageFlags;
        ImageType             m_type;
        ImageSize3D           m_size;
        Format                m_format;
        SampleCount           m_sampleCount;
        uint32_t              m_mipLevels;
        uint32_t              m_arrayCount;

        Handle<Image>         m_handle;


        TL::Map<ImageViewCreateInfo, Handle<Image>> m_views;

        struct PassDependency
        {
            Handle<RenderGraphImage> rgImage          = NullHandle;
            Pass*                    pass             = nullptr;
            Handle<Image>            view             = NullHandle;
            ImageSubresourceRange    subresourceRange = ImageSubresourceRange::All();
            ImageBarrierState        state            = {};
        };

        // List of this resource's dependencies based on their access order
        TL::Vector<Dependency> m_orderedDependencies;
    };

    struct RenderGraphBuffer : RenderGraphResource
    {
        TL::String             m_name;
        TL::Flags<BufferUsage> m_usageFlags;
        size_t                 m_size;

        Handle<Buffer>         m_handle;

        struct Dependency
        {
            Pass*              pass             = nullptr;
            BufferSubregion    subresourceRange = {};
            BufferBarrierState state            = {};
        };

        // List of this resource's dependencies based on their access order
        TL::Vector<Dependency> m_orderedDependencies;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Bindings
    ///////////////////////////////////////////////////////////////////////////

    struct ColorRGAttachment
    {
        Handle<RenderGraphImage> view        = NullHandle;
        LoadOperation            loadOp      = LoadOperation::Discard;
        StoreOperation           storeOp     = StoreOperation::Store;
        ClearValue               clearValue  = {.f32 = {0.0f, 0.0f, 0.0f, 1.0f}};
        ResolveMode              resolveMode = ResolveMode::None;
        Handle<RenderGraphImage> resolveView = NullHandle;
    };

    struct DepthStencilRGAttachment
    {
        Handle<RenderGraphImage> view           = NullHandle;
        LoadOperation            depthLoadOp    = LoadOperation::Discard;
        StoreOperation           depthStoreOp   = StoreOperation::Store;
        LoadOperation            stencilLoadOp  = LoadOperation::Discard;
        StoreOperation           stencilStoreOp = StoreOperation::Store;
        DepthStencilValue        clearValue     = {0.0f, 0};
    };

    struct RGRenderPassBeginInfo
    {
        ImageSize2D                            size;
        ImageOffset2D                          offset;
        TL::Span<const ColorRGAttachment>      colorAttachments;
        TL::Optional<DepthStencilRGAttachment> depthStencilAttachment;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph Common and helper types
    ///////////////////////////////////////////////////////////////////////////

    using PassSetupCallback   = TL::Function<void(Pass& pass)>;
    using PassCompileCallback = TL::Function<void(Pass& pass)>;
    using PassExecuteCallback = TL::Function<void(CommandList& commandList)>;

    struct RenderGraphCreateInfo
    {
        TL::Allocator* allocator = nullptr; ///< Allocator to use for memory management.
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

    struct ImageUseInfo
    {
        Handle<RenderGraphImage> image;
        ImageSubresourceRange    subresourcesRange;
        ImageUsage               usage;
        PipelineStage            stage;
        Access                   access;
    };

    struct BufferUseInfo
    {
        Handle<RenderGraphBuffer> buffer;
        BufferSubregion           subregion;
        BufferUsage               usage;
        PipelineStage             stage;
        Access                    access;
    };

    using RGResourceHandle       = uint32_t;
    using RGResourceTransitionID = uint32_t;

    struct ImageDependency
    {
        Handle<RenderGraphImage> image;
        uint32_t                 usageIndex;
    };

    struct BufferDependency
    {
    };

    /// @brief Represents a render graph pass, encapsulating its setup, execution, and resource usage.
    class RHI_EXPORT Pass
    {
        friend RenderGraph;

    public:
        RHI_INTERFACE_BOILERPLATE(Pass);

        /// @brief Enum representing different barrier slots for a pass.
        enum BarrierSlot
        {
            Prilogue, ///< Barriers executed before the pass begins.
            Epilogue, ///< Barriers executed after the pass ends.
            Resolve,  ///< Barriers for resolving resources.
            Count,    ///< Total number of barrier slots.
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

        void UseImage(const ImageUseInfo& useInfo);
        void UseBuffer(const BufferUseInfo& useInfo);
        void UseColorAttachment(ColorRGAttachment colorAttachment);
        void UseDepthStencil(DepthStencilRGAttachment depthStencilAttachment);

    private:
        RenderGraph*                           m_renderGraph;            ///< Pointer to the render graph this pass belongs to.
        TL::String                             m_name;                   ///< Name of the pass.
        PassType                               m_type;                   ///< The type of the pass.
        PassSetupCallback                      m_setupCallback;          ///< Callback for setting up the pass.
        PassCompileCallback                    m_compileCallback;        ///< Callback for compiling the pass.
        PassExecuteCallback                    m_executeCallback;        ///< Callback for executing the pass.
        ImageSize2D                            m_imageSize;              ///< Size of the images used in the pass.
        TL::Vector<ColorRGAttachment>          m_colorAttachments;       ///< List of color attachments used in the pass.
        TL::Optional<DepthStencilRGAttachment> m_depthStencilAttachment; ///< Depth-stencil attachment used in the pass.
        TL::Vector<ImageDependency>            m_imageDependencies;
        TL::Vector<BufferDependency>           m_bufferDependencies;

        struct Barriers
        {
            TL::Vector<BarrierInfo>       memoryBarriers; ///< Memory barriers for synchronization.
            TL::Vector<ImageBarrierInfo>  imageBarriers;  ///< Image barriers for image transitions.
            TL::Vector<BufferBarrierInfo> bufferBarriers; ///< Buffer barriers for buffer transitions.
        };

        Barriers m_barriers[BarrierSlot::Count]; ///< Barriers for each slot in the pass.
        bool     m_isActive   : 1 = false;
        bool     m_isCompiled : 1 = false; ///< Flag indicating if the pass has been compiled.
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Render Graph
    ///////////////////////////////////////////////////////////////////////////

    class RHI_EXPORT RenderGraph
    {
        friend Pass;

    public:
        RHI_INTERFACE_BOILERPLATE(RenderGraph);

        ResultCode                             Init(const RenderGraphCreateInfo& ci);
        void                                   Shutdown();

        // Import existing resources into the render graph.

        /// @brief Imports a swapchain into the render graph for rendering.
        /// @param name The name of the swapchain resource.
        /// @param swapchain The swapchain object to import.
        /// @param format The format of the imported swapchain image.
        /// @return A handle to the imported RenderGraphImage.
        TL_NODISCARD Handle<RenderGraphImage>  ImportSwapchain(const char* name, Swapchain& swapchain, Format format);

        /// @brief Imports an existing image into the render graph.
        /// @param name The name of the image resource.
        /// @param image The handle to the existing image.
        /// @param format The format of the imported image.
        /// @return A handle to the imported RenderGraphImage.
        TL_NODISCARD Handle<RenderGraphImage>  ImportImage(const char* name, Handle<Image> image, Format format);

        /// @brief Imports an existing buffer into the render graph.
        /// @param name The name of the buffer resource.
        /// @param buffer The handle to the existing buffer.
        /// @return A handle to the imported RenderGraphBuffer.
        TL_NODISCARD Handle<RenderGraphBuffer> ImportBuffer(const char* name, Handle<Buffer> buffer);

        /// @brief Creates a new image in the render graph.
        /// @param createInfo The creation parameters for the image.
        /// @return A handle to the created RenderGraphImage.
        TL_NODISCARD Handle<RenderGraphImage>  CreateImage(const char* name, ImageType type, ImageSize3D size, Format format, uint32_t mipLevels = 1, uint32_t arrayCount = 1, SampleCount samples = SampleCount::Samples1);

        /// @brief Creates a new buffer in the render graph.
        /// @param createInfo The creation parameters for the buffer.
        /// @return A handle to the created RenderGraphBuffer.
        TL_NODISCARD Handle<RenderGraphBuffer> CreateBuffer(const char* name, size_t size);

        /// @brief Finds an imported image in the render graph.
        /// @param image The handle to the existing image.
        /// @return A handle to the imported RenderGraphImage, or NullHandle if not found.
        TL_NODISCARD Handle<RenderGraphImage>  FindImportedImage(Handle<Image> image) const;

        /// @brief Finds an imported buffer in the render graph.
        /// @param buffer The handle to the existing buffer.
        /// @return A handle to the imported RenderGraphBuffer, or NullHandle if not found.
        TL_NODISCARD Handle<RenderGraphBuffer> FindImportedBuffer(Handle<Buffer> buffer) const;

        /// @brief Destroys an existing image in the render graph.
        /// @param handle The handle to the RenderGraphImage to destroy.
        void                                   DestroyImage(Handle<RenderGraphImage> handle);

        /// @brief Destroys an existing buffer in the render graph.
        /// @param handle The handle to the RenderGraphBuffer to destroy.
        void                                   DestroyBuffer(Handle<RenderGraphBuffer> handle);

        // Frame management.

        /// @brief Begins a new frame in the render graph.
        /// @param frameSize The size of the frame (typically matches swapchain size).
        void                                   BeginFrame(ImageSize2D frameSize);

        /// @brief Ends the current frame and executes the render graph.
        void                                   EndFrame();

        /////////////////////////////////////////////////////////////////////////////////
        /// Graph execution: The following methods are only valid during graph setup
        /// And must never be called outside of Begin/End Scopes
        /////////////////////////////////////////////////////////////////////////////////

        /// @brief Returns the default render target size.
        /// @return The size of the render target.
        TL_NODISCARD ImageSize2D               GetFrameSize() const;

        /// @brief Returns the image associated with the given handle.
        /// @param handle The handle to the RenderGraphImage.
        /// @return A pointer to the RenderGraphImage, or nullptr if not found.
        TL_NODISCARD const RenderGraphImage*   GetImage(Handle<RenderGraphImage> handle) const;

        /// @brief Returns the buffer associated with the given handle.
        /// @param handle The handle to the RenderGraphBuffer.
        /// @return A pointer to the RenderGraphBuffer, or nullptr if not found.
        TL_NODISCARD const RenderGraphBuffer*  GetBuffer(Handle<RenderGraphBuffer> handle) const;

        /// @brief Returns the underlying image handle for a RenderGraphImage.
        /// @param handle The handle to the RenderGraphImage.
        /// @return The handle to the underlying Image.
        TL_NODISCARD Handle<Image>             GetImageHandle(Handle<RenderGraphImage> handle) const;

        /// @brief Returns the underlying buffer handle for a RenderGraphBuffer.
        /// @param handle The handle to the RenderGraphBuffer.
        /// @return The handle to the underlying Buffer.
        TL_NODISCARD Handle<Buffer>            GetBufferHandle(Handle<RenderGraphBuffer> handle) const;

        // // clang-format off
        // template<typename T>
        // T*                                     AllocatePassParams(const char* name);
        // // clang-format on

        [[maybe_unused]] Pass*                 AddPass(const PassCreateInfo& createInfo);

        /// @brief Queues a buffer upload operation.
        /// @param buffer The handle to the RenderGraphBuffer to upload data to.
        /// @param data The data block to upload.
        void                                   QueueBufferUpload(Handle<RenderGraphBuffer> buffer, TL::Block data);

        /// @brief Queues a buffer upload operation with an offset.
        /// @param buffer The handle to the RenderGraphBuffer to upload data to.
        /// @param offset The offset in the buffer to start the upload.
        /// @param data The data block to upload.
        void                                   QueueBufferUpload(Handle<RenderGraphBuffer> buffer, uint32_t offset, TL::Block data);

        // clang-format off
        // /// @brief Queues a buffer upload operation for a storage buffer.
        // /// @tparam ElementType The type of the elements in the storage buffer.
        // /// @param buffer The storage buffer to upload data to.
        // /// @param elements The span of elements to upload.
        // template<typename ElementType>
        // void                                   QueueBufferUpload(StorageBuffer<ElementType> buffer, TL::Span<const ElementType> elements);
        // clang-format on

    private:
        void               ExtendImageUsageFlags(Handle<RenderGraphImage> resource, ImageUsage usage);
        void               ExtendBufferUsageFlags(Handle<RenderGraphBuffer> resource, BufferUsage usage);

        ImageBarrierState  GetBarrierStateInfo(Handle<RenderGraphImage> image, int32_t accessIndex);
        BufferBarrierState GetBarrierStateInfo(Handle<RenderGraphBuffer> buffer, int32_t accessIndex);

    private:
        void TransientResourcesInit(bool enableAliasing = false);
        void TransientResourcesShutdown();

        void Compile();
        void Execute();

    protected:
        /// Main allocator for graph resources.
        TL::IAllocator*                                    m_allocator = new TL::Mimalloc();
        /// Temporary allocator for transient data.
        TL::Arena                                          m_tempAllocator;
        /// The associated device for the render graph.
        Device*                                            m_device;
        /// The current frame index.
        uint64_t                                           m_frameIndex;

        /// List of images in the graph.
        HandlePool<RenderGraphImage>                       m_imageOwner;
        /// List of buffers in the graph.
        HandlePool<RenderGraphBuffer>                      m_bufferOwner;

        /// Lookup table for imported images.
        TL::Map<Handle<Image>, Handle<RenderGraphImage>>   m_graphImportedImagesLookup;
        /// Lookup table for transient images.
        TL::Set<Handle<RenderGraphImage>>                  m_graphTransientImagesLookup;
        /// Lookup table for imported buffers.
        TL::Map<Handle<Buffer>, Handle<RenderGraphBuffer>> m_graphImportedBuffersLookup;
        /// Lookup table for transient buffers.
        TL::Set<Handle<RenderGraphBuffer>>                 m_graphTransientBuffersLookup;
        /// Lookup table for imported swapchains.
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