#pragma once
#include "RHI/Export.hpp"
#include "RHI/Attachments.hpp"
#include "RHI/Pass.hpp"

namespace RHI
{
    struct ImagePassAttachment;
    struct BufferPassAttachment;

    class Context;
    class Swapchain;
    class CommandList;

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

        void                        SetBufferedFramesCount(uint32_t count);

        uint32_t                    GetBufferedFramesCount() const;

        uint32_t                    GetCurrentFrameIndex();

        /// @brief Called at the beginning of the render-loop.
        /// This marks the begining of a graphics frame.
        void                        Begin();

        /// @brief Called at the ending of the render-loop.
        /// This marks the ending of a graphics frame.
        void                        End();

        Ptr<Pass>                   CreatePass(const char* name, QueueType queueType);

        /// @brief Called after all passes inside the Frame Graph are setup, to finialize the graph
        void                        Compile();

        /// @brief Called when the Render Target is resized, to recreate all graph resources, with the new sizes.
        void                        ResizeFrame(ImageSize2D newSize);

        /// @brief Executes a list of command lists, and signal the provided fence when complete
        void                        ExecuteCommandList(TL::Span<CommandList*> commandLists, Fence& fence);

    private:
        void   Cleanup();

        Fence& GetFrameCurrentFence();

    protected:
        virtual void DeviceWaitIdle()                                                                            = 0;
        virtual void QueuePassSubmit(Pass* pass, Fence* fence)                                                   = 0;
        virtual void QueueCommandsSubmit(QueueType queueType, TL::Span<CommandList*> commandLists, Fence& fence) = 0;

    private:
        uint32_t m_frameCount;
        uint32_t m_currentFrameIndex;
        uint64_t m_frameNumber;

        // A list of fences for each frame in flight

    protected:
        Context*                                    m_context;

        std::vector<Pass*>                          m_passList;

        std::unique_ptr<AttachmentsRegistry>        m_attachmentsRegistry;

        std::unique_ptr<TransientResourceAllocator> m_transientResourceAllocator;

        ImageSize2D                                 m_frameSize;

        ImageAttachment*                            m_swapchainImage;

        std::vector<std::unique_ptr<Fence>>         m_frameReadyFence;
    };

} // namespace RHI