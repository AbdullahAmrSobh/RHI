#include "RHI/FrameScheduler.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Context.hpp"

#include <tracy/Tracy.hpp>

namespace RHI
{
    FrameScheduler::FrameScheduler(Context* context)
        : m_context(context)
        , m_stagingBuffer()
        , m_transientAllocator(nullptr)
        , m_attachmentsPool(CreatePtr<AttachmentsPool>(m_context))
    {
    }

    void FrameScheduler::Reset()
    {
    }

    void FrameScheduler::Begin()
    {
    }

    void FrameScheduler::End()
    {
        CompileResourceViews();

        for (auto pass : m_passList)
        {
            PassSubmit(pass, nullptr);
        }
    }

    Ptr<Pass> FrameScheduler::CreatePass(const char* name, QueueType queueType)
    {
        auto pass = CreatePtr<Pass>(this, name, queueType);
        m_passList.push_back(pass.get());
        return pass;
    }

    ResultCode FrameScheduler::DestroyPass(Pass* pass)
    {
        delete pass;
        return ResultCode::Success;
    }

    void FrameScheduler::WriteImageContent(Handle<Image> handle, ImageOffset offset, ImageSize3D size, ImageSubresourceLayers subresource, TL::Span<uint8_t> data)
    {
        auto buffer = m_stagingBuffer->Allocate(data.size_bytes());
        memcpy(buffer.pData, data.data(), data.size_bytes());

        // StageImageWrite(handle);

        BufferToImageCopyInfo copyInfo{};
        copyInfo.srcBuffer = buffer.buffer;
        copyInfo.srcOffset = 0;
        copyInfo.srcSize = size;
        copyInfo.dstImage = handle;
        copyInfo.dstSubresource = subresource;
        copyInfo.dstOffset = offset;

        auto commandList = m_copyCommandListAllocator->Allocate();
        commandList->Begin();
        commandList->Copy(copyInfo);
        commandList->End();

        auto fence = m_context->CreateFence();
        ExecuteCommandLists(*commandList, fence.get());
        fence->Wait();
    }

    void FrameScheduler::WriteBufferContent(Handle<Buffer> handle, TL::Span<uint8_t> data)
    {
        auto buffer = m_stagingBuffer->Allocate(data.size());
        memcpy(buffer.pData, data.data(), data.size());

        StageBufferWrite(handle);

        BufferCopyInfo copyInfo{};

        auto commandList = m_copyCommandListAllocator->Allocate();
        commandList->Begin();
        commandList->Copy(copyInfo);
        commandList->End();
    }

    ResultCode FrameScheduler::Compile()
    {
        CompileTransientResources();
        CompileResourceViews();
        return ResultCode::Success;
    }

    std::string FrameScheduler::GetGraphviz()
    {
        // return m_renderGraph->GetGraphviz();
        return "";
    }

    Fence& FrameScheduler::GetFrameCurrentFence()
    {
        return *m_frameReadyFence[m_currentFrameIndex];
    }

    void FrameScheduler::CompileTransientResources()
    {
        m_transientAllocator->Begin();
        for (auto attachment : m_attachmentsPool->GetAttachments())
        {
            if (attachment->m_lifetime == Attachment::Lifetime::Transient)
            {
                auto imageAttachment = (ImageAttachment*)attachment;
                imageAttachment->SetSize({1600, 1200});
                m_transientAllocator->Allocate(attachment);
            }
        }
        m_transientAllocator->End();
    }

    void FrameScheduler::CompileResourceViews()
    {
        for (auto attachment : m_attachmentsPool->GetAttachments())
        {
            for (auto passAttachment = attachment->GetFirstPassAttachment(); passAttachment != nullptr; passAttachment = passAttachment->GetNext())
            {
                m_attachmentsPool->InitPassAttachment(passAttachment);
            }
        }
    }

    void FrameScheduler::CleanupTransientResources()
    {
        for (auto transientAttachment : m_attachmentsPool->GetTransientAttachments())
        {
            for (auto passAttachment = transientAttachment->GetFirstPassAttachment(); passAttachment != nullptr; passAttachment = passAttachment->GetNext())
            {
                m_attachmentsPool->ShutdownPassAttachment(passAttachment);
            }
            m_transientAllocator->Destroy(transientAttachment);
        }
    }

    void FrameScheduler::CleanupResourceViews()
    {
        for (auto attachment : m_attachmentsPool->GetAttachments())
        {
            for (auto passAttachment = attachment->GetFirstPassAttachment(); passAttachment != nullptr; passAttachment = passAttachment->GetNext())
            {
                m_attachmentsPool->ShutdownPassAttachment(passAttachment);
            }
        }
    }

} // namespace RHI