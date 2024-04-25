#include "RHI/FrameScheduler.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Context.hpp"

#include <tracy/Tracy.hpp>

namespace RHI
{
    FrameScheduler::FrameScheduler(Context* context)
        : m_context(context)
        , m_attachmentsPool(CreatePtr<AttachmentsPool>(m_context))
    {
    }

    ImageAttachment* FrameScheduler::CreateImage(const ImageCreateInfo& createInfo)
    {
        return m_attachmentsPool->CreateAttachment(createInfo);
    }

    BufferAttachment* FrameScheduler::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        return m_attachmentsPool->CreateAttachment(createInfo);
    }

    ImageAttachment* FrameScheduler::ImportSwapchain(const char* name, Swapchain& swapchain)
    {
        return m_attachmentsPool->CreateAttachment(name, &swapchain);
    }

    ImageAttachment* FrameScheduler::ImportImage(const char* name, Handle<Image> image)
    {
        return m_attachmentsPool->CreateAttachment(name, image);
    }

    BufferAttachment* FrameScheduler::ImportBuffer(const char* name, Handle<Buffer> buffer)
    {
        return m_attachmentsPool->CreateAttachment(name, buffer);
    }

    void FrameScheduler::Begin()
    {
    }

    void FrameScheduler::End()
    {
        for (auto pass : m_passList)
        {
            PassSubmit(pass, nullptr);
        }

        m_context->DestroyResources();
    }

    Ptr<Pass> FrameScheduler::CreatePass(const char* name, QueueType queueType)
    {
        auto pass = CreatePtr<Pass>(this, name, queueType);
        m_passList.push_back(pass.get());
        return pass;
    }

    void FrameScheduler::WriteImageContent(Handle<Image> handle,
                                           ImageOffset3D offset,
                                           ImageSize3D size,
                                           ImageSubresourceLayers subresource,
                                           TL::Span<const uint8_t> content)
    {
        BufferCreateInfo tmpBufferCreateInfo{};
        tmpBufferCreateInfo.usageFlags = BufferUsage::CopySrc;
        tmpBufferCreateInfo.byteSize = content.size_bytes();
        auto buffer = m_context->CreateBuffer(tmpBufferCreateInfo).GetValue();
        memcpy(m_context->MapBuffer(buffer), content.data(), content.size_bytes());
        m_context->UnmapBuffer(buffer);
        m_context->DestroyBuffer(buffer);

        BufferToImageCopyInfo copyInfo{};
        copyInfo.srcBuffer = buffer;
        copyInfo.srcOffset = 0;
        copyInfo.dstSize = size;
        copyInfo.dstImage = handle;
        copyInfo.dstSubresource = subresource;
        copyInfo.dstOffset = offset;
        StageImageWrite(copyInfo);
    }

    void FrameScheduler::Compile()
    {
        for (auto& attachment : m_attachmentsPool->GetTransientAttachments())
        {
            if (attachment->m_type == Attachment::Type::Image)
            {
                auto imageAttachment = attachment->As<ImageAttachment>();
                if (imageAttachment->m_swapchain)
                    continue;
                auto createInfo = imageAttachment->GetCreateInfo();
                createInfo.size.width = 1600;
                createInfo.size.height = 1200;
                imageAttachment->SetHandle(m_context->CreateImage(createInfo).GetValue());
                for (auto passAttachment = imageAttachment->GetFirstPassAttachment(); passAttachment; passAttachment = passAttachment->GetNext())
                {
                    passAttachment->m_viewInfo.image = imageAttachment->GetHandle();
                    passAttachment->m_view = m_context->CreateImageView(passAttachment->m_viewInfo);
                }
            }
            else if (attachment->m_type == Attachment::Type::Buffer)
            {
                auto bufferAttachment = attachment->As<BufferAttachment>();
                bufferAttachment->SetHandle(m_context->CreateBuffer(bufferAttachment->GetCreateInfo()).GetValue());

                for (auto passAttachment = bufferAttachment->GetFirstPassAttachment(); passAttachment; passAttachment = passAttachment->GetNext())
                {
                    passAttachment->m_view = m_context->CreateBufferView(passAttachment->m_viewInfo);
                }
            }
            else
            {
                RHI_UNREACHABLE();
            }
        }
    }

    void FrameScheduler::Cleanup()
    {
        for (auto attachment : m_attachmentsPool->GetTransientAttachments())
        {
            if (attachment->m_type == Attachment::Type::Image)
            {
                auto imageAttachment = attachment->As<ImageAttachment>();
                m_context->DestroyImage(imageAttachment->GetHandle());
            }
            else if (attachment->m_type == Attachment::Type::Buffer)
            {
                auto bufferAttachment = attachment->As<BufferAttachment>();
                m_context->DestroyBuffer(bufferAttachment->GetHandle());
            }
            else
            {
                RHI_UNREACHABLE();
            }
        }

        m_context->DestroyResources();
    }
} // namespace RHI