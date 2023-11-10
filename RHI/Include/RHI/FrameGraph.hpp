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
        Stencil,
        DepthStencil,
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

    enum class PipelineAccessStage
    {
        None,
        Vertex,
        Pixel,
        Compute,
        Graphics,
    };

    struct QueueInfo
    {
        QueueType type;
        uint32_t  id;
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
        // Name of the attachment
        const char*        name;

        // Lifetime of the attachment
        AttachmentLifetime lifetime;

        // Type of the attachment
        AttachmentType     type;

        // Union for holding either ImageAttachment or BufferAttachment
        union
        {
            // Structure for ImageAttachment
            struct ImageAttachment
            {
                // Handle to the image
                Handle<Image>        handle;

                // Information about the image
                ImageCreateInfo      info;

                // Pointer to the first usage in an image pass
                ImagePassAttachment* firstUse;

                // Pointer to the last usage in an image pass
                ImagePassAttachment* lastUse;
            } asImage;

            // Structure for BufferAttachment
            struct BufferAttachment
            {
                // Handle to the buffer
                Handle<Buffer>        handle;

                // Information about the buffer
                BufferCreateInfo      info;

                // Pointer to the first usage in a buffer pass
                BufferPassAttachment* firstUse;

                // Pointer to the last usage in a buffer pass
                BufferPassAttachment* lastUse;
            } asBuffer;
        };
    };

    struct ImagePassAttachment
    {
        Handle<Attachment>         attachment;

        Pass*                      pass;

        ImageAttachmentUseInfo     info;

        Handle<ImageView>          view;

        Flags<PipelineAccessStage> stages;

        ImagePassAttachment*       next;
        ImagePassAttachment*       prev;
    };

    struct BufferPassAttachment
    {
        Handle<Attachment>         attachment;

        Pass*                      pass;

        BufferAttachmentUseInfo    info;

        Handle<BufferView>         view;

        Flags<PipelineAccessStage> stages;

        BufferPassAttachment*      next;
        BufferPassAttachment*      prev;
    };

    struct RenderTargetInfo
    {
        ImageSize           size;
        std::vector<Format> colorFormats;
        Format              depthFormat;
    };

    struct GraphResourceID
    {
        uint32_t resourceIndex;
        uint32_t resourceBaseIndex;
    };

    /// @brief Represents a pass, which encapsulates a GPU task.
    class Pass
    {
        friend class FrameScheduler;

    public:
        Pass(Context* context)
            : m_context(context)
        {
        }
        virtual ~Pass() = default;

        /// @brief Called at the beginning of this pass building phase.
        void                  Begin();

        /// @brief Called at the end of this pass building phase.
        void                  End();

        /// @brief Used to inspect the current state of this pass.
        PassQueueState        GetPassQueueState() const;

        /// @brief Adds a pass to the wait list.
        void                  ExecuteAfter(Pass& pass);

        /// @brief Adds a pass to the signal list.
        void                  ExecuteBefore(Pass& pass);

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

        /// @brief Begins the command list associated with this pass.
        /// @param commandsCount Number of commands to be submitted.
        /// @return reference to the command lists
        virtual CommandList&  BeginCommandList(uint32_t commandsCount = 1) = 0;

        /// @brief Ends the command list of assoicated with this pass.
        virtual void          EndCommandList() = 0;

        /// @brief Returns the queue info, which this pass is executed on
        QueueInfo             GetQueueInfo() const
        {
            return m_queuInfo;
        }

    private:
        ImagePassAttachment*  UseAttachment(Handle<Attachment> handle, const ImageAttachmentUseInfo& useInfo);
        BufferPassAttachment* UseAttachment(Handle<Attachment> handle, const BufferAttachmentUseInfo& useInfo);

    protected:
        virtual void           OnBegin() = 0;
        virtual void           OnEnd()   = 0;

        /// @brief Used to inspect the current state of this pass in the command queue.
        virtual PassQueueState GetPassQueueStateInternal() const = 0;

    protected:
        Context*                         m_context;

        FrameScheduler*                  m_scheduler;

        /// @brief A pointer to swapchain which would be presented into.
        Swapchain*                       m_swapchain;

        /// @brief Information about the queue this pass is executed on
        QueueInfo                        m_queuInfo;

        /// @brief The size of the rendering area in the render pass.
        ImageSize                        m_size;

        /// @brief A list of all passes that this pass depends on.
        std::vector<Pass*>               m_producers;

        /// @brief A list of all passes that depends on this pass.
        std::vector<Pass*>               m_consumers;

        /// @brief The command list used to execute this pass.
        std::unique_ptr<CommandList>     m_commandlist;

        /// NOTE: PassAttachment pointers, must not be invalidated during the lifetime Frame.
        /// Therefore, a deque is used instead of a vector.

        /// @brief A list of all image pass attachment used by this pass.
        std::deque<ImagePassAttachment>  m_imagePassAttachments;

        /// @brief A list of all buffer pass attachment used by this pass.
        std::deque<BufferPassAttachment> m_bufferPassAttachment;

        /// @brief Index to last attachment used by this pass.
        uint32_t                         m_lastImageAttachmentIndex;
        uint32_t                         m_lastBufferAttachmentIndex;
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

        /// @brief Compiles the frame graph, no new passes are allowed to be submitted
        /// in the current frame after a this function is called.
        void                          Compile();

        /// @brief Returns a pointer to the attachment resource.
        Attachment*                   GetAttachment(Handle<Attachment> attachmentHandle);

        virtual std::unique_ptr<Pass> CreatePass(const PassCreateInfo& createInfo) = 0;

    private:
        void TopologicalSort();

        void ResetPasses();

        void InitTransientResources();

        void CreateAttachmentViews();

        void CleanupTransientResources();

    protected:
        virtual void               BeginInternal()                                                                  = 0;

        virtual void               EndInternal()                                                                    = 0;

        virtual void               ExecutePass(Pass* pass)                                                          = 0;

        virtual void               Allocate(Handle<Attachment> handle)                                              = 0;

        virtual void               Release(Handle<Attachment> handle)                                               = 0;

        virtual Handle<Image>      CreateTransientImageResource(const ImageCreateInfo& createInfo)                  = 0;

        virtual Handle<Buffer>     CreateTransientBufferResource(const BufferCreateInfo& createInfo)                = 0;

        virtual Handle<ImageView>  CreateImageView(Attachment* attachment, const ImageAttachmentUseInfo& useInfo)   = 0;

        virtual Handle<BufferView> CreateBufferView(Attachment* attachment, const BufferAttachmentUseInfo& useInfo) = 0;

        virtual void               FreeTransientBufferResource(Handle<Buffer> handle)                               = 0;

        virtual void               FreeTransientImageResource(Handle<Image> handle)                                 = 0;

        virtual void               FreeImageView(Handle<ImageView> handle)                                          = 0;

        virtual void               FreeBufferView(Handle<BufferView> handle)                                        = 0;

    protected:
        Context* m_context;

        struct FrameGraphNode
        {
            Pass*                 pass;

            bool                  enabled;

            std::vector<uint32_t> edges;
        };

        std::vector<FrameGraphNode>                   m_frameGraph;

        std::vector<Pass*>                            m_passList;

        std::vector<Swapchain*>                       m_swapchainsToPresent;

        HandlePool<Attachment>                        m_attachments;

        std::vector<Handle<Attachment>>               m_transientAttachment;

        std::unordered_map<size_t, Handle<ImageView>> m_cachedImageViews;

        std::unordered_map<size_t, Handle<ImageView>> m_cachedBufferView;
    };

} // namespace RHI