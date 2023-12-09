#pragma once

#include "RHI/CommandList.hpp"
#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"

#include <deque>
#include <memory>
#include <string>
#include <vector>

namespace RHI
{

    // Forward declerations
    class Context;
    class Swapchain;
    class CommandList;
    class Pass;
    class FrameScheduler;

    enum class PassQueueState
    {
        NotSubmitted,
        Pending,
        Executing,
        Finished,
        Building,
        Compiling,
        Failed,
    };

    enum class QueueType
    {
        Graphics,
        Compute,
        Transfer,
    };

    /// @brief Enumerates the different types of attachments that can be used.
    enum class AttachmentType
    {
        Image,
        Buffer,
        Swapchain,
        Resolve,
    };

    /// @brief Enumerates the different types the attachment can be used as.
    enum class AttachmentUsage
    {
        None,
        VertexInputBuffer,
        ShaderStorage,
        RenderTarget,
        Depth,
        ShaderResource,
        Copy,
        Resolve,
    };

    /// @brief Enumerates how an attachment is access
    enum class AttachmentAccess
    {
        /// @brief Invalid option.
        None,
        /// @brief Attachment is read as a shader resource.
        Read,
        /// @brief Attachment is renderTargetOutput.
        Write,
        /// @brief Attachment is available for read and write as a shader resource.
        ReadWrite,
    };

    /// @brief Enumerates the different types of attachment's lifetime.
    enum class AttachmentLifetime
    {
        // Attachment resource is created outside of the frame
        Persistent,
        // Attachment resource is only valid for the duration of the current frame
        Transient,
    };

    /// @brief Enumerates ...
    enum class ImageLoadOperation
    {
        /// @brief The attachment load operation undefined.

        DontCare,
        /// @brief Load attachment content.

        Load,
        /// @brief Discard attachment content.

        Discard,
    };

    /// @brief Enumerates ...
    enum class ImageStoreOperation
    {
        // Attachment Store operation is undefined
        DontCare,
        // Writes to the attachment are stored
        Store,
        // Writes to the attachment are discarded
        Discard,
    };

    struct PassCreateInfo
    {
        std::string name;
        QueueType   type;
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

    struct ClearValue
    {
        ColorValue        color;
        DepthStencilValue depth;

        inline bool       operator==(const ClearValue& other) const
        {
            return color == other.color && depth == other.depth;
        }

        inline bool operator!=(const ClearValue& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Structure specifying the load and store opertions for image attachment.
    struct ImageLoadStoreOperations
    {
        ImageLoadOperation  loadOperation  = ImageLoadOperation::Load;
        ImageStoreOperation storeOperation = ImageStoreOperation::Store;

        inline bool         operator==(const ImageLoadStoreOperations& other) const
        {
            return loadOperation == other.loadOperation && storeOperation == other.storeOperation;
        }

        inline bool operator!=(const ImageLoadStoreOperations& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Structure specifying the parameters of an image attachment.
    struct ImageAttachmentUseInfo
    {
        ComponentMapping         components;
        ImageSubresource         subresource;
        ImageLoadStoreOperations loadStoreOperations;
        ClearValue               clearValue;

        AttachmentUsage          usage  = AttachmentUsage::None;
        AttachmentAccess         access = AttachmentAccess::None;

        inline bool              operator==(const ImageAttachmentUseInfo& other) const
        {
            return components == other.components && subresource == other.subresource && loadStoreOperations == other.loadStoreOperations && clearValue == other.clearValue && usage == other.usage && access == other.access;
        }

        inline bool operator!=(const ImageAttachmentUseInfo& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Structure specifying the parameters of an buffer attachment.
    struct BufferAttachmentUseInfo
    {
        Format           format;
        size_t           byteOffset;
        size_t           byteSize;

        AttachmentUsage  usage  = AttachmentUsage::None;
        AttachmentAccess access = AttachmentAccess::None;

        inline bool      operator==(const BufferAttachmentUseInfo& other) const
        {
            return format == other.format && byteOffset == other.byteOffset && byteSize == other.byteSize && usage == other.usage && access == other.access;
        }

        inline bool operator!=(const BufferAttachmentUseInfo& other) const
        {
            return !(*this == other);
        }
    };

    struct Attachment
    {
        Attachment(const char* name, AttachmentLifetime lifetime, AttachmentType type)
            : name(name)
            , lifetime(lifetime)
            , type(type)
        {
        }

        // Name of the attachment
        const char*              name;

        // Lifetime of the attachment
        const AttachmentLifetime lifetime;

        // Type of the attachment
        const AttachmentType     type;

        virtual void             Reset() = 0;
    };

    // Structure for ImageAttachment
    struct ImageAttachment final : Attachment
    {
        ImageAttachment(const char* name, Swapchain* swapchain)
            : Attachment(name, AttachmentLifetime::Persistent, AttachmentType::Swapchain)
            , swapchain(swapchain)
            , firstUse(nullptr)
            , lastUse(nullptr)
        {
        }

        ImageAttachment(const char* name, Handle<Image> handle)
            : Attachment(name, AttachmentLifetime::Persistent, AttachmentType::Image)
            , swapchain(nullptr)
            , handle(handle)
            , firstUse(nullptr)
            , lastUse(nullptr)
        {
        }

        ImageAttachment(const char* name, const ImageCreateInfo& createInfo)
            : Attachment(name, AttachmentLifetime::Transient, AttachmentType::Image)
            , info(createInfo)
            , firstUse(nullptr)
            , lastUse(nullptr)
        {
        }

        ImageAttachment(const ImageAttachment& other) = delete;

        void Reset() override
        {
            firstUse = nullptr;
            lastUse  = nullptr;
        }

        Swapchain*           swapchain = nullptr;

        // Handle to the image
        Handle<Image>        handle    = {};

        // Information about the image
        ImageCreateInfo      info      = {};

        // Pointer to the first usage in an image pass
        ImagePassAttachment* firstUse  = nullptr;

        // Pointer to the last usage in an image pass
        ImagePassAttachment* lastUse   = nullptr;
    };

    // Structure for BufferAttachment
    struct BufferAttachment final : Attachment
    {
        BufferAttachment(const char* name, Handle<Buffer> handle)
            : Attachment(name, AttachmentLifetime::Persistent, AttachmentType::Buffer)
            , handle(handle)
            , firstUse(nullptr)
            , lastUse(nullptr)
        {
        }

        BufferAttachment(const char* name, const BufferCreateInfo& createInfo)
            : Attachment(name, AttachmentLifetime::Transient, AttachmentType::Buffer)
            , info(createInfo)
            , firstUse(nullptr)
            , lastUse(nullptr)
        {
        }

        BufferAttachment(const BufferAttachment& other) = delete;

        void Reset() override
        {
            firstUse = nullptr;
            lastUse  = nullptr;
        }

        // Handle to the buffer
        Handle<Buffer>        handle   = {};

        // Information about the buffer
        BufferCreateInfo      info     = {};

        // Pointer to the first usage in a buffer pass
        BufferPassAttachment* firstUse = nullptr;

        // Pointer to the last usage in a buffer pass
        BufferPassAttachment* lastUse  = nullptr;
    };

    struct ImagePassAttachment
    {
        ImagePassAttachment()                                 = default;
        ImagePassAttachment(const ImagePassAttachment& other) = delete;
        ImagePassAttachment(ImagePassAttachment&& other)      = default;

        ImageAttachment*       attachment;

        Pass*                  pass;

        ImageAttachmentUseInfo info;

        Handle<ImageView>      view;

        ShaderStage            stage;

        ImagePassAttachment*   next;
        ImagePassAttachment*   prev;
    };

    struct BufferPassAttachment
    {
        BufferPassAttachment()                                  = default;
        BufferPassAttachment(const BufferPassAttachment& other) = delete;
        BufferPassAttachment(BufferPassAttachment&& other)      = default;

        BufferAttachment*       attachment;

        Pass*                   pass;

        BufferAttachmentUseInfo info;

        Handle<BufferView>      view;

        ShaderStage             stage;

        BufferPassAttachment*   next;
        BufferPassAttachment*   prev;
    };

    class TransientAttachmentAllocator
    {
    public:
        virtual ~TransientAttachmentAllocator()             = default;

        virtual void Begin()                                = 0;
        virtual void End()                                  = 0;

        /// @brief bind the given resource to a memory allocation (may alias).
        virtual void Allocate(Attachment* attachment)  = 0;

        /// @brief returns the memory used by this resource to allocator, to be reused.
        /// @note this means that this resource wont be used in any subsequent operations.
        virtual void Free(Attachment* attachment)      = 0;
    };

    class RHI_EXPORT AttachmentsRegistry
    {
    public:
        virtual ~AttachmentsRegistry() = default;

        void                                           Reset();

        ImageAttachment*                               ImportSwapchainImageAttachment(const char* name, Swapchain* swapchain);

        ImageAttachment*                               ImportImageAttachment(const char* name, Handle<Image> handle);

        BufferAttachment*                              ImportBufferAttachment(const char* name, Handle<Buffer> handle);

        ImageAttachment*                               CreateTransientImageAttachment(const char* name, const ImageCreateInfo& createInfo);

        BufferAttachment*                              CreateTransientBufferAttachment(const char* name, const BufferCreateInfo& createInfo);

        std::vector<std::unique_ptr<ImageAttachment>>  m_imageAttachments;

        std::vector<std::unique_ptr<BufferAttachment>> m_bufferAttachments;

        std::vector<Attachment*>                       m_attachments;

        std::vector<ImageAttachment*>                  m_swapchainAttachments;

        std::vector<ImageAttachment*>                  m_importedImageAttachments;

        std::vector<BufferAttachment*>                 m_importedBufferAttachments;

        std::vector<ImageAttachment*>                  m_transientImageAttachments;

        std::vector<BufferAttachment*>                 m_transientBufferAttachments;
    };

    /// @brief Represents a pass, which encapsulates a GPU task.
    class RHI_EXPORT Pass
    {
        friend class FrameScheduler;

    public:
        Pass(Context* context)
            : m_context(context)
            , m_size({ 0, 0, 0 })
        {
        }

        Pass(const Pass& other) = delete;

        virtual ~Pass()         = default;

        /// @brief Called at the beginning of this pass building phase.
        void                  Begin();

        /// @brief Called at the end of this pass building phase.
        void                  End();

        /// @brief Adds a pass to the wait list.
        void                  ExecuteAfter(Pass& pass);

        /// @brief Adds a pass to the signal list.
        void                  ExecuteBefore(Pass& pass);

        /// @brief Imports an external image resource to be used in this pass.
        /// @param image handle to the image resource.
        /// @param useInfo resource use information.
        /// @return Handle to an image view into the used resource.
        ImagePassAttachment*  ImportSwapchainImageResource(const char* name, Swapchain* swapchain, const ImageAttachmentUseInfo& useInfo);

        /// @brief Imports an external image resource to be used in this pass.
        /// @param image handle to the image resource.
        /// @param useInfo resource use information.
        /// @return Handle to an image view into the used resource.
        ImagePassAttachment*  ImportImageResource(const char* name, Handle<Image> image, const ImageAttachmentUseInfo& useInfo);

        /// @brief Imports an external buffer resource to be used in this pass.
        /// @param buffer handle to the buffer resource.
        /// @param useInfo resource use information.
        /// @return Handle to an buffer view into the used resource.
        BufferPassAttachment* ImportBufferResource(const char* name, Handle<Buffer> buffer, const BufferAttachmentUseInfo& useInfo);

        /// @brief Creates a new transient image resource, and use it in this pass.
        /// @param createInfo transient image create info.
        /// @return Handle to an image view into the used resource.
        ImagePassAttachment*  CreateTransientImageResource(const char* name, const ImageCreateInfo& createInfo, const ImageAttachmentUseInfo& useInfo);

        /// @brief Creates a new transient buffer resource, and use it in this pass.
        /// @param createInfo transient buffer create info.
        /// @return Handle to an buffer view into the used resource.
        BufferPassAttachment* CreateTransientBufferResource(const char* name, const BufferCreateInfo& createInfo, const BufferAttachmentUseInfo& useInfo);

        /// @brief Use an existing image resource in this pass.
        /// @param view Handle to the used resource.
        /// @param useInfo image resource use information.
        /// @return Handle to an image resource.
        ImagePassAttachment*  UseImageResource(ImagePassAttachment* attachment, const ImageAttachmentUseInfo& useInfo);

        /// @brief Use an existing buffer resource in this pass.
        /// @param view Handle to the used resource.
        /// @param useInfo buffer resource use information.
        /// @return Handle to an buffer resource.
        BufferPassAttachment* UseBufferResource(BufferPassAttachment* attachment, const BufferAttachmentUseInfo& useInfo);

        /// @brief Submits a list of command lists for execution on this pass.
        void                  Execute(TL::Span<CommandList*> commandLists);

    private:
        ImagePassAttachment*  UseAttachment(ImageAttachment*& attachment, const ImageAttachmentUseInfo& useInfo);

        BufferPassAttachment* UseAttachment(BufferAttachment*& attachment, const BufferAttachmentUseInfo& useInfo);

    protected:
        Context*                         m_context;

        FrameScheduler*                  m_scheduler;

        std::string                      m_name;

        /// @brief A pointer to swapchain which would be presented into.
        Swapchain*                       m_swapchain;

        /// @brief The type of the Hardware Queue needed to execute this pass.
        QueueType                        m_queueType;

        /// @brief A list of command lists that executes on this pass.
        std::vector<CommandList*>        m_commandLists;

        /// @brief The size of the rendering area in the render pass.
        ImageSize                        m_size;

        /// @brief A list of all passes that this pass depends on.
        std::vector<Pass*>               m_producers;

        /// @brief A list of all passes that depends on this pass.
        std::vector<Pass*>               m_consumers;

        /// NOTE: PassAttachment pointers, must not be invalidated during the lifetime Frame.
        /// Therefore, a deque is used instead of a vector.

        /// @brief A list of all image pass attachment used by this pass.
        std::deque<ImagePassAttachment>  m_imagePassAttachments;

        /// @brief A list of all buffer pass attachment used by this pass.
        std::deque<BufferPassAttachment> m_bufferPassAttachment;
    };

    /// @brief A frame scheduler is a frame-graph system breaks down the final frame
    /// into a set of passes, each pass represent a GPU workload. Passes share resources
    /// as Attachments. The frame scheduler tracks every attachment state accross passe.
    class RHI_EXPORT FrameScheduler
    {
        friend class Pass;

    public:
        FrameScheduler(Context* context)
            : m_context(context)
            , m_attachmentsRegistry(std::make_unique<AttachmentsRegistry>())
        {
        }

        virtual ~FrameScheduler() = default;

        /// @brief Called at the beginning of the render-loop.
        /// This marks the begining of a graphics frame.
        void                          Begin();

        /// @brief Called at the ending of the render-loop.
        /// This marks the ending of a graphics frame.
        void                          End();

        /// @brief Register a pass producer, to be called this frame.
        void                          Submit(Pass& pass);

        /// @brief Creates a pass resource
        virtual std::unique_ptr<Pass> CreatePass(const PassCreateInfo& createInfo) = 0;

        virtual bool                  WaitIdle(uint64_t waitTimeNano)              = 0;

    protected:
        virtual void                                           ExecutePass(Pass& pass) = 0;

        virtual void                                           OnFrameBegin()          = 0;
        virtual void                                           OnFrameEnd()            = 0;

        Context*                                               m_context;

        std::vector<Pass*>                                     m_passList;

        std::unique_ptr<TransientAttachmentAllocator>          m_transientAttachmentAllocator;

        std::unique_ptr<AttachmentsRegistry>                   m_attachmentsRegistry;

        std::unordered_map<Handle<Image>, Handle<ImageView>>   m_imageViewsLut;

        std::unordered_map<Handle<Buffer>, Handle<BufferView>> m_bufferViewLut;

        uint64_t                                               m_frameIndex;

        const uint64_t                                         m_frameBufferingMaxCount = 3;

    private:
        void Reset();

        void Compile();

        void CompileTransientAttachments();

        void CompileAttachmentsViews();

        void CompileSwapchainViews();

        void Execute();
    };

} // namespace RHI