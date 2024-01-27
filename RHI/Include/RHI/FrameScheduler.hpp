#pragma once

#include "RHI/CommandList.hpp"
#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Attachments.hpp"

#include <string>

namespace RHI
{
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