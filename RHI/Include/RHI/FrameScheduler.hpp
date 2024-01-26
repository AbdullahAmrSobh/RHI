#pragma once

#include "RHI/CommandList.hpp"
#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace RHI
{
    // Forward declerations

    struct ImagePassAttachment;
    struct BufferPassAttachment;

    class Context;
    class Swapchain;
    class CommandList;
    class Pass;
    class FrameScheduler;

    enum class QueueType
    {
        Graphics,
        Compute,
        Transfer,
    };

    /// @brief Enumerates how an attachment is access
    enum class AttachmentAccess
    {
        None,      // Invalid option.
        Read,      // Attachment is read as a shader resource.
        Write,     // Attachment is renderTargetOutput.
        ReadWrite, // Attachment is available for read and write as a shader resource.
    };

    /// @brief Enumerates the different types the image attachment can be used as.
    enum class AttachmentUsage
    {
        Color,                 // Color attachment in a render target
        Depth,                 // Depth attachment in a render target
        Stencil,               // Stencil attachment in a render target
        DepthStencil,          // Depth and Stencil attachment in a render target
        PipelineInputAssembly, // Resource is Index or Vertex buffer attribute pipeline vertex assembler state
        ShaderResource,        // Resource will be bound and read by a shader (e.g. sampled image, or uniform buffer)
        StorageResource,       // Resource will be bound and read (or written to) by a shader (e.g. storage buffers, storage image)
        Copy,                  // Resource will be used for transfer operations
    };

    /// @brief Enumerates ...
    enum class LoadOperation
    {
        DontCare, // The attachment load operation undefined.
        Load,     // Load attachment content.
        Discard,  // Discard attachment content.
    };

    /// @brief Enumerates ...
    enum class StoreOperation
    {
        DontCare, // Attachment Store operation is undefined
        Store,    // Writes to the attachment are stored
        Discard,  // Writes to the attachment are discarded
    };

    struct ColorValue
    {
        float       r = 1.0f;
        float       g = 1.0f;
        float       b = 1.0f;
        float       a = 1.0f;

        inline bool operator==(const ColorValue& other) const
        {
            return r == other.r && g == other.g && b == other.b && a == other.a;
        }

        inline bool operator!=(const ColorValue& other) const
        {
            return !(*this == other);
        }
    };

    struct DepthStencilValue
    {
        float       depthValue   = 1.0f;
        uint8_t     stencilValue = 0xff;

        inline bool operator==(const DepthStencilValue& other) const
        {
            return depthValue == other.depthValue && stencilValue == other.stencilValue;
        }

        inline bool operator!=(const DepthStencilValue& other) const
        {
            return !(*this == other);
        }
    };

    using ClearValue = std::variant<ColorValue, DepthStencilValue>;

    /// @brief Structure specifying the load and store opertions for image attachment.
    struct LoadStoreOperations
    {
        LoadOperation  loadOperation  = LoadOperation::Load;
        StoreOperation storeOperation = StoreOperation::Store;

        inline bool    operator==(const LoadStoreOperations& other) const
        {
            return loadOperation == other.loadOperation && storeOperation == other.storeOperation;
        }

        inline bool operator!=(const LoadStoreOperations& other) const
        {
            return !(*this == other);
        }
    };

    struct Attachment
    {
        enum class Type
        {
            Image,
            SwapchainImage,
            Resolve,
            Buffer,
        };

        enum class Lifetime
        {
            Persistent,
            Transient,
        };

        Attachment(const char* name, Lifetime lifetime, Type type)
            : referenceCount(0)
            , name(name)
            , lifetime(lifetime)
            , type(type)
        {
        }

        virtual ~Attachment() = default;

        std::atomic<uint32_t> referenceCount;

        const char*           name;     // Name of the attachment
        const Lifetime        lifetime; // Lifetime of the attachment
        const Type            type;     // Type of the attachment
    };

    // Structure for ImageAttachment
    struct ImageAttachment final : public Attachment
    {
        ImageAttachment(const char* name, Handle<Image> handle);
        ImageAttachment(const char* name, Swapchain* swapchain);
        ImageAttachment(const char* name, const ImageCreateInfo& createInfo);

        Swapchain*           swapchain; // pointer to swapchain, if this is an swapchain image attachment
        Handle<Image>        handle;    // Handle to the image
        ImageCreateInfo      info;      // Information about the image
        ImagePassAttachment* firstUse;  // Pointer to the first usage in an image pass
        ImagePassAttachment* lastUse;   // Pointer to the last usage in an image pass

        void                 PushPassAttachment(ImagePassAttachment* passAttachment);
        void                 Reset();
        Handle<Image>        GetImage();
    };

    // Structure for BufferAttachment
    struct BufferAttachment final : public Attachment
    {
        BufferAttachment(const char* name, Handle<Buffer> handle);
        BufferAttachment(const char* name, const BufferCreateInfo& createInfo);

        Handle<Buffer>        handle;   // Handle to the buffer
        BufferCreateInfo      info;     // Information about the buffer
        BufferPassAttachment* firstUse; // Pointer to the first usage in a buffer pass
        BufferPassAttachment* lastUse;  // Pointer to the last usage in a buffer pass

        void                  PushPassAttachment(BufferPassAttachment* passAttachment);
        void                  Reset();
        Handle<Image>         GetBuffer();
    };

    struct ImagePassAttachment
    {
        ImagePassAttachment()                                 = default;
        ImagePassAttachment(const ImagePassAttachment& other) = delete;
        ImagePassAttachment(ImagePassAttachment&& other)      = default;

        Pass*                pass;
        ImageAttachment*     attachment;
        ImagePassAttachment* next;
        ImagePassAttachment* prev;
        AttachmentUsage      usage;
        AttachmentAccess     access;
        ImageViewCreateInfo  viewInfo;
        Handle<ImageView>    view;

        // Render target related
        ClearValue           clearValue;
        LoadStoreOperations  loadStoreOperations;

        RHI::ShaderStage     stage;
    };

    struct BufferPassAttachment
    {
        BufferPassAttachment()                                  = default;
        BufferPassAttachment(const BufferPassAttachment& other) = delete;
        BufferPassAttachment(BufferPassAttachment&& other)      = default;

        Pass*                 pass;
        BufferAttachment*     attachment;
        BufferPassAttachment* next;
        BufferPassAttachment* prev;
        AttachmentUsage       usage;
        AttachmentAccess      access;
        BufferViewCreateInfo  viewInfo;
        Handle<BufferView>    view;

        RHI::ShaderStage      stage;
    };

    struct SwapchainImagePassAttachment : ImagePassAttachment
    {
        SwapchainImagePassAttachment()                                          = default;
        SwapchainImagePassAttachment(const SwapchainImagePassAttachment& other) = delete;
        SwapchainImagePassAttachment(SwapchainImagePassAttachment&& other)      = delete;

        Handle<ImageView> views[Swapchain::c_MaxSwapchainBackBuffersCount];

        Handle<ImageView> GetView();
    };

    class RHI_EXPORT AttachmentsRegistry
    {
    public:
        using AttachmentID    = const char*;

        AttachmentsRegistry() = default;

        /// @brief Resets the registry to empty state
        void              Reset();

        /// @brief Imports an external image resource to be used in this pass.
        /// @param image handle to the image resource.
        /// @param useInfo resource use information.
        /// @return Handle to an image view into the used resource.
        ImageAttachment*  ImportSwapchainImage(const char* name, Swapchain* swapchain);

        /// @brief Imports an external image resource to be used in this pass.
        /// @param image handle to the image resource.
        /// @param useInfo resource use information.
        /// @return Handle to an image view into the used resource.
        ImageAttachment*  ImportImage(const char* name, Handle<Image> handle);

        /// @brief Imports an external buffer resource to be used in this pass.
        /// @param buffer handle to the buffer resource.
        /// @param useInfo resource use information.
        /// @return Handle to an buffer view into the used resource.
        BufferAttachment* ImportBuffer(const char* name, Handle<Buffer> handle);

        /// @brief Creates a new transient image resource, and use it in this pass.
        /// @param createInfo transient image create info.
        /// @return Handle to an image view into the used resource.
        ImageAttachment*  CreateTransientImage(const char* name, const ImageCreateInfo& createInfo);

        /// @brief Creates a new transient buffer resource, and use it in this pass.
        /// @param createInfo transient buffer create info.
        /// @return Handle to an buffer view into the used resource.
        BufferAttachment* CreateTransientBuffer(const char* name, const BufferCreateInfo& createInfo);

        /// @brief Lookup for an resource with the given name in the registery
        /// returns nullptr, if not found.
        ImageAttachment*  FindImage(AttachmentID id);

        /// @brief Lookup for an resource with the given name in the registery
        /// returns nullptr, if not found.
        BufferAttachment* FindBuffer(AttachmentID id);

    private:
        friend class FrameScheduler;
        template<typename AttachmentType>
        using AttachmentLookup = std::unordered_map<const char*, std::unique_ptr<AttachmentType>>;

        AttachmentLookup<ImageAttachment>  m_imageAttachments;
        AttachmentLookup<BufferAttachment> m_bufferAttachments;

        std::vector<AttachmentID>          m_swapchainAttachments;
    };

    /// @brief Represents a pass, which encapsulates a GPU task.
    class RHI_EXPORT Pass
    {
    public:
        Pass(const char* name, QueueType type);
        virtual ~Pass() = default;

        inline ImagePassAttachment* UseColorAttachment(ImageAttachment* attachment, ColorValue value, LoadStoreOperations loadStoreOperations = LoadStoreOperations{})
        {
            return UseColorAttachment(attachment, ImageViewCreateInfo{ ImageAspect::Color }, value, loadStoreOperations);
        }

        inline ImagePassAttachment* UseDepthAttachment(ImageAttachment* attachment, DepthStencilValue value, LoadStoreOperations loadStoreOperations = LoadStoreOperations{})
        {
            return UseDepthAttachment(attachment, ImageViewCreateInfo{ ImageAspect::Depth }, value, loadStoreOperations);
        }

        inline ImagePassAttachment* UseStencilAttachment(ImageAttachment* attachment, DepthStencilValue value, LoadStoreOperations loadStoreOperations = LoadStoreOperations{})
        {
            return UseStencilAttachment(attachment, ImageViewCreateInfo{ ImageAspect::Stencil }, value, loadStoreOperations);
        }

        inline ImagePassAttachment* UseDepthStencilAttachment(ImageAttachment* attachment, DepthStencilValue value, LoadStoreOperations loadStoreOperations = LoadStoreOperations{})
        {
            return UseDepthStencilAttachment(attachment, ImageViewCreateInfo{ ImageAspect::DepthStencil }, value, loadStoreOperations);
        }

        ImagePassAttachment*  UseColorAttachment(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, ColorValue value, LoadStoreOperations loadStoreOperations);

        ImagePassAttachment*  UseDepthAttachment(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, DepthStencilValue value, LoadStoreOperations loadStoreOperations);

        ImagePassAttachment*  UseStencilAttachment(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, DepthStencilValue value, LoadStoreOperations loadStoreOperations);

        ImagePassAttachment*  UseDepthStencilAttachment(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, DepthStencilValue value, LoadStoreOperations loadStoreOperations);

        ImagePassAttachment*  UseShaderImageResource(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo);

        BufferPassAttachment* UseShaderBufferResource(BufferAttachment* attachment, const BufferViewCreateInfo& viewInfo);

        ImagePassAttachment*  UseShaderImageStorage(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, AttachmentAccess access);

        BufferPassAttachment* UseShaderBufferStorage(BufferAttachment* attachment, const BufferViewCreateInfo& viewInfo, AttachmentAccess access);

        ImagePassAttachment*  UseCopyImageResource(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, AttachmentAccess access);

        BufferPassAttachment* UseCopyBufferResource(BufferAttachment* attachment, const BufferViewCreateInfo& viewInfo, AttachmentAccess access);

    private:
        ImagePassAttachment*  EmplaceNewPassAttachment(ImageAttachment* attachment);
        BufferPassAttachment* EmplaceNewPassAttachment(BufferAttachment* attachment);

    protected:
        friend class FrameScheduler;

        template<typename PassAttachmentType>
        using PassAttachmentStorage = std::vector<std::unique_ptr<PassAttachmentType>>;

        std::string                                 m_name;
        QueueType                                   m_queueType;             // The type of the Hardware Queue needed to execute this pass.
        Swapchain*                                  m_swapchain;             // A pointer to swapchain which would be presented into.
        ImageSize2D                                 m_size;                  // The size of the rendering area in the render pass.
        std::vector<Pass*>                          m_producers;             // A list of all passes that this pass will wait for.
        std::vector<CommandList*>                   m_commandLists;          // A list of command lists that executes this pass.
        PassAttachmentStorage<ImagePassAttachment>  m_imagePassAttachments;  // A list of all image pass attachment used by this pass.
        PassAttachmentStorage<BufferPassAttachment> m_bufferPassAttachments; // A list of all buffer pass attachment used by this pass.
    };

    class RHI_EXPORT TransientResourceAllocator
    {
    public:
        virtual ~TransientResourceAllocator()                           = default;

        virtual void Begin(Context* context)                            = 0;
        virtual void End(Context* context)                              = 0;

        virtual void Reset(Context* context)                            = 0;

        virtual void Allocate(Context* context, Attachment* attachment) = 0;
        virtual void Release(Context* context, Attachment* attachment)  = 0;
        virtual void Destroy(Context* context, Attachment* attachment)  = 0;
    };
    
    /// @brief A frame scheduler is a frame-graph system breaks down the final frame
    /// into a set of passes, each pass represent a GPU workload. Passes share resources
    /// as Attachments. The frame scheduler tracks every attachment state accross passe.
    class RHI_EXPORT FrameScheduler
    {
        friend class Pass;

    public:
        FrameScheduler(Context* context);
        virtual ~FrameScheduler() = default;

        inline AttachmentsRegistry& GetRegistry() { return *m_attachmentsRegistry; }

        inline uint32_t             GetFrameBufferingCount() const { return m_maxFrameBufferingCount; }

        inline void                 SetFrameBufferingCount(uint32_t count) { m_maxFrameBufferingCount = count; }

        inline uint32_t             GetCurrentFrameBufferIndex() const { return m_currentFrameIndex; }

        /// @brief Called at the beginning of the render-loop.
        /// This marks the begining of a graphics frame.
        void                        Begin();

        /// @brief Called at the ending of the render-loop.
        /// This marks the ending of a graphics frame.
        void                        End();

        /// @brief Register a pass producer, to be called this frame.
        void                        RegisterPass(Pass& pass);

        /// @brief Called after all passes inside the Frame Graph are setup, to finialize the graph
        void                        Compile();

        /// @brief Called when the Render Target is resized, to recreate all graph resources, with the new sizes.
        void                        ResizeFrame(ImageSize2D newSize);

        /// @brief Executes a list of command lists, and signal the provided fence when complete
        void                        ExecuteCommandList(TL::Span<CommandList*> commandLists, Fence& signalFence);

    private:
        void Cleanup();

    protected:
        Fence*             GetCurrentFrameFence();

        Handle<ImageView>  FindOrCreateView(Handle<Image> handle, const ImageViewCreateInfo& createInfo);
        Handle<BufferView> FindOrCreateView(Handle<Buffer> handle, const BufferViewCreateInfo& createInfo);

        virtual void       OnBegin()                                                                                         = 0;
        virtual void       OnEnd()                                                                                           = 0;

        virtual void       DeviceWaitIdle()                                                                                  = 0;
        virtual void       QueuePassSubmit(Pass* pass, Fence* signalFence)                                                   = 0;
        virtual void       QueueImagePresent(ImageAttachment* attachments, Fence* signalFence)                               = 0;
        virtual void       QueueCommandsSubmit(QueueType queueType, TL::Span<CommandList*> commandLists, Fence* signalFence) = 0;

    protected:
        Context*                                      m_context;

        std::vector<Pass*>                            m_passList;

        std::unordered_map<size_t, Handle<ImageView>> m_imageViewsLut;
        std::unordered_map<size_t, Handle<ImageView>> m_bufferViewsLut;

        std::unique_ptr<AttachmentsRegistry>          m_attachmentsRegistry;
        std::unique_ptr<TransientResourceAllocator>   m_transientResourceAllocator;

        uint32_t                                      m_maxFrameBufferingCount;
        uint32_t                                      m_currentFrameIndex;
        ImageSize2D                                   m_frameSize;

        ImageAttachment*                              m_swapchainAttachment;

        std::vector<std::unique_ptr<Fence>>           m_fences;
    };

} // namespace RHI