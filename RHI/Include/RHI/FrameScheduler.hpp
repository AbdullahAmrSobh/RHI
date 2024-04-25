#pragma once
#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Pass.hpp" // todo remove

#include "RHI/Common/Ptr.h"
#include "RHI/Common/Containers.h"

namespace RHI
{
    struct ImagePassAttachment;
    struct BufferPassAttachment;
    struct TempBuffer;

    class Context;
    class Swapchain;
    class CommandList;
    class AttachmentsPool;
    class CommandPool;

    /// @brief A frame scheduler is a frame-graph system breaks down the final frame
    /// into a set of passes, each pass represent a GPU workload. Passes share resources
    /// as Attachments. The frame scheduler tracks every attachment state accross passe.
    class RHI_EXPORT FrameScheduler
    {
        friend class Pass;

    public:
        FrameScheduler(Context* context);
        virtual ~FrameScheduler() = default;

        ImageAttachment*  CreateImage(const ImageCreateInfo& createInfo);
        BufferAttachment* CreateBuffer(const BufferCreateInfo& useInfo);
        ImageAttachment*  ImportSwapchain(const char* name, Swapchain& swapchain);
        ImageAttachment*  ImportImage(const char* name, Handle<Image> image);
        BufferAttachment* ImportBuffer(const char* name, Handle<Buffer> buffer);

        void              Begin();
        void              End();

        Ptr<Pass>         CreatePass(const char* name, QueueType queueType);

        void              WriteImageContent(Handle<Image>           handle,
                                            ImageOffset3D           offset,
                                            ImageSize3D             size,
                                            ImageSubresourceLayers  subresource,
                                            TL::Span<const uint8_t> content);
        void              Compile();
        void              Cleanup();

        virtual void      PassSubmit(Pass* pass, Fence* fence)                          = 0;
        virtual void      StageImageWrite(const struct BufferToImageCopyInfo& copyInfo) = 0;

        Handle<ImageView>     GetImageView(ImagePassAttachment* passAttachment);
        Handle<BufferView>    GetBufferView(BufferPassAttachment* passAttachment);

    private:
        void                  DestroyAttachment(Attachment* attachment);

    protected:
        Context*                                       m_context;

        TL::UnorderedMap<std::string, Ptr<Attachment>> m_attachmentsLut;
        TL::Vector<Attachment*>                        m_attachments;
        TL::Vector<Attachment*>                        m_transientAttachments;
        TL::Vector<ImageAttachment*>                   m_imageAttachments;
        TL::Vector<BufferAttachment*>                  m_bufferAttachments;

        Ptr<CommandPool>                               m_commandPool;

        TL::Vector<Pass*>                              m_passList;
    };

} // namespace RHI