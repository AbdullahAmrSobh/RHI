#pragma once
#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Pass.hpp" // todo remove

#include "RHI/Common/Ptr.h"
#include "RHI/Common/Containers.h"
#include "RHI/Common/Result.hpp"

#include "RHI/TransientAllocator.hpp"
#include "RHI/StagingBuffer.hpp"

namespace RHI
{
    struct ImagePassAttachment;
    struct BufferPassAttachment;
    struct TempBuffer;

    class Context;
    class Swapchain;
    class CommandList;
    class TransientAllocator;
    class AttachmentsPool;
    class StagingBuffer;
    class CommandListAllocator;

    /// @brief A frame scheduler is a frame-graph system breaks down the final frame
    /// into a set of passes, each pass represent a GPU workload. Passes share resources
    /// as Attachments. The frame scheduler tracks every attachment state accross passe.
    class RHI_EXPORT FrameScheduler
    {
        friend class Pass;

    public:
        FrameScheduler(Context* context);
        virtual ~FrameScheduler() = default;

        void        Reset();
        void        Begin();
        void        End();

        Ptr<Pass>   CreatePass(const char* name, QueueType queueType);
        ResultCode  DestroyPass(Pass* pass);

        void        WriteImageContent(Handle<Image>           handle,
                                      ImageOffset             offset,
                                      ImageSize3D             size,
                                      ImageSubresourceLayers  subresource,
                                      TL::Span<const uint8_t> content);

        ResultCode  Compile();

    private:
        Fence& GetFrameCurrentFence();

        void   CompileTransientResources();
        void   CompileResourceViews();

        void   CleanupTransientResourcesViews();
        void   CleanupTransientResources();
        void   CleanupResourceViews();

    protected:
        virtual void PassSubmit(Pass* pass, Fence* fence)                          = 0;
        virtual void StageImageWrite(const struct BufferToImageCopyInfo& copyInfo) = 0;

    protected:
        Context*                  m_context;
        Ptr<StagingBuffer>        m_stagingBuffer;
        Ptr<TransientAllocator>   m_transientAllocator;
        Ptr<AttachmentsPool>      m_attachmentsPool;
        Ptr<CommandListAllocator> m_commandListAllocator;

        TL::Vector<Pass*>         m_passList;

        TL::Vector<Ptr<Fence>>    m_frameReadyFence;
        uint32_t                  m_frameCount;
        uint32_t                  m_currentFrameIndex;
        uint64_t                  m_frameNumber;
    };

} // namespace RHI