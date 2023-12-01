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

    enum class QueueType
    {
        Graphics,
        Compute,
        Transfer,
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

        ImageUsage               usage  = ImageUsage::None;
        ShaderAccess             access = ShaderAccess::None;

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
        Format       format;
        size_t       byteOffset;
        size_t       byteSize;

        BufferUsage  usage  = BufferUsage::None;
        ShaderAccess access = ShaderAccess::None;

        inline bool  operator==(const BufferAttachmentUseInfo& other) const
        {
            return format == other.format && byteOffset == other.byteOffset && byteSize == other.byteSize && usage == other.usage && access == other.access;
        }

        inline bool operator!=(const BufferAttachmentUseInfo& other) const
        {
            return !(*this == other);
        }
    };

    struct ImagePassAttachment
    {
        ImagePassAttachment()                                 = default;
        ImagePassAttachment(const ImagePassAttachment& other) = delete;
        ImagePassAttachment(ImagePassAttachment&& other)      = default;

        Handle<Image>          resourceHandle;
        Handle<ImageView>      viewHandle;

        Pass*                  pass;

        ImageAttachmentUseInfo info;

        ShaderStage            stages;

        ImagePassAttachment*   next;
        ImagePassAttachment*   prev;
    };

    struct BufferPassAttachment
    {
        BufferPassAttachment()                                  = default;
        BufferPassAttachment(const BufferPassAttachment& other) = delete;
        BufferPassAttachment(BufferPassAttachment&& other)      = default;

        Handle<Buffer>          resourceHandle;
        Handle<BufferView>      viewHandle;

        Pass*                   pass;

        BufferAttachmentUseInfo info;

        ShaderStage             stages;

        BufferPassAttachment*   next;
        BufferPassAttachment*   prev;
    };

    class TransientAttachmentAllocator
    {
    public:
        virtual ~TransientAttachmentAllocator()                                          = default;

        virtual void           Begin()                                                   = 0;
        virtual void           End()                                                     = 0;

        virtual void           Activate(Image* resource)                                 = 0;

        virtual void           Deactivate(Image* resource)                               = 0;

        virtual void           Activate(Buffer* resource)                                = 0;

        virtual void           Deactivate(Buffer* resource)                              = 0;

        virtual Handle<Image>  CreateTransientImage(const ImageCreateInfo& createInfo)   = 0;

        virtual Handle<Buffer> CreateTransientBuffer(const BufferCreateInfo& createInfo) = 0;
    };

    /// @brief Represents a pass, which encapsulates a GPU task.
    class RHI_EXPORT Pass
    {
        friend class FrameScheduler;

    public:
        Pass(Context* context)
            : m_context(context)
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
        ImagePassAttachment*  ImportSwapchainImageResource(Swapchain* swapchain, const ImageAttachmentUseInfo& useInfo);

        /// @brief Imports an external image resource to be used in this pass.
        /// @param image handle to the image resource.
        /// @param useInfo resource use information.
        /// @return Handle to an image view into the used resource.
        ImagePassAttachment*  ImportImageResource(Handle<Image> image, const ImageAttachmentUseInfo& useInfo);

        /// @brief Imports an external buffer resource to be used in this pass.
        /// @param buffer handle to the buffer resource.
        /// @param useInfo resource use information.
        /// @return Handle to an buffer view into the used resource.
        BufferPassAttachment* ImportBufferResource(Handle<Buffer> buffer, const BufferAttachmentUseInfo& useInfo);

        /// @brief Creates a new transient image resource, and use it in this pass.
        /// @param createInfo transient image create info.
        /// @return Handle to an image view into the used resource.
        ImagePassAttachment*  CreateTransientImageResource(const ImageCreateInfo& createInfo, const ImageAttachmentUseInfo& useInfo);

        /// @brief Creates a new transient buffer resource, and use it in this pass.
        /// @param createInfo transient buffer create info.
        /// @return Handle to an buffer view into the used resource.
        BufferPassAttachment* CreateTransientBufferResource(const BufferCreateInfo& createInfo, const BufferAttachmentUseInfo& useInfo);

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
        virtual void          EndCommandList()                             = 0;

    private:
        ImagePassAttachment*  UseAttachment(Handle<Image> image, const ImageAttachmentUseInfo& useInfo);

        BufferPassAttachment* UseAttachment(Handle<Buffer> buffer, const BufferAttachmentUseInfo& useInfo);

    protected:
        Context*                         m_context;

        FrameScheduler*                  m_scheduler;

        std::string                      m_name;

        /// @brief A pointer to swapchain which would be presented into.
        Swapchain*                       m_swapchain;

        /// @brief Pointer to the current command list executing this pass.
        CommandList*                     m_commandList;

        /// @brief The type of the Hardware Queue needed to execute this pass.
        QueueType                        m_queueType;

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

        virtual std::unique_ptr<Pass> CreatePass(const PassCreateInfo& createInfo) = 0;

    protected:
        enum FrameGraphState
        {
            Invalid,
            Ready,
        };

        virtual void                                           ExecutePass(Pass& pass)             = 0;

        virtual void                                           ResetPass(Pass& pass)               = 0;

        virtual CommandList*                                   GetCommandList(uint32_t frameIndex) = 0;

        virtual void                                           OnFrameEnd()                        = 0;

        Context*                                               m_context;

        std::vector<Pass*>                                     m_passList;

        std::unique_ptr<TransientAttachmentAllocator>          m_transientAttachmentAllocator;

        FrameGraphState                                        m_state;

        std::unordered_map<Handle<Image>, Handle<ImageView>>   m_imageViewsLut;

        std::unordered_map<Handle<Buffer>, Handle<BufferView>> m_bufferViewLut;

    private:
        void Compile();

        void CompileTransientAttachments();

        void CompileAttachmentsViews();

        void CompileSwapchainViews();

        void Execute();
    };

} // namespace RHI