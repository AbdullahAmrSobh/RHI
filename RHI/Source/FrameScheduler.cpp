#include "RHI/FrameScheduler.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Context.hpp"
#include "RHI/Attachments.hpp"
#include "RHI/Swapchain.hpp"

#include <tracy/Tracy.hpp>

namespace RHI
{
    FrameScheduler::FrameScheduler(Context* context)
        : m_context(context)
    {
    }

    ImageAttachment* FrameScheduler::CreateImage(const ImageCreateInfo& createInfo)
    {
        auto attachment = (ImageAttachment*)m_attachmentsLut.insert(std::make_pair(createInfo.name, CreatePtr<ImageAttachment>(createInfo.name, createInfo))).first->second.get();
        m_attachments.push_back(attachment);
        m_imageAttachments.push_back(attachment);
        m_transientAttachments.push_back(attachment);
        return attachment;
    }

    BufferAttachment* FrameScheduler::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        auto attachment = (BufferAttachment*)m_attachmentsLut.insert(std::make_pair(createInfo.name, CreatePtr<BufferAttachment>(createInfo.name, createInfo.byteSize))).first->second.get();
        m_attachments.push_back(attachment);
        m_transientAttachments.push_back(attachment);
        m_bufferAttachments.push_back(attachment);
        return attachment;
    }

    ImageAttachment* FrameScheduler::ImportSwapchain(const char* name, Swapchain& swapchain)
    {
        auto attachment = ImportImage(name, swapchain.GetImage());
        attachment->m_swapchain = &swapchain;
        attachment->m_asImage.info.type = ImageType::Image2D;
        return attachment;
    }

    ImageAttachment* FrameScheduler::ImportImage(const char* name, Handle<Image> image)
    {
        auto attachment = (ImageAttachment*)m_attachmentsLut.insert(std::make_pair(name, CreatePtr<ImageAttachment>(name, image))).first->second.get();
        m_attachments.push_back(attachment);
        m_imageAttachments.push_back(attachment);
        return attachment;
    }

    BufferAttachment* FrameScheduler::ImportBuffer(const char* name, Handle<Buffer> buffer)
    {
        auto attachment = (BufferAttachment*)m_attachmentsLut.insert(std::make_pair(name, CreatePtr<BufferAttachment>(name, buffer))).first->second.get();
        m_attachments.push_back(attachment);
        m_bufferAttachments.push_back(attachment);
        return attachment;
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
        for (auto& attachment : m_transientAttachments)
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
        for (auto attachment : m_transientAttachments)
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

    void FrameScheduler::DestroyAttachment(Attachment* attachment)
    {
        for (auto passAttachment = attachment->GetFirstPassAttachment(); passAttachment != nullptr; passAttachment = passAttachment->GetNext())
        {
            // ShutdownPassAttachment(passAttachment);
        }

        switch (attachment->m_type)
        {
        case Attachment::Type::Image:
            {
                auto imageAttachment = (ImageAttachment*)attachment;
                m_context->DestroyImage(imageAttachment->GetHandle());
                break;
            }
        case Attachment::Type::Buffer:
            {
                auto bufferAttachment = (BufferAttachment*)attachment;
                m_context->DestroyBuffer(bufferAttachment->GetHandle());
                break;
            }
        default:
            {
                RHI_UNREACHABLE();
                break;
            }
        }
    }

    Handle<ImageView> FrameScheduler::GetImageView(ImagePassAttachment* passAttachment)
    {
        passAttachment->m_viewInfo.image = passAttachment->GetAttachment()->GetHandle();

        if (auto swapchain = passAttachment->GetAttachment()->m_swapchain)
        {
            return swapchain->GetImageView(m_context, passAttachment->m_viewInfo);
        }

        return passAttachment->m_view;
    }

    Handle<BufferView> FrameScheduler::GetBufferView(BufferPassAttachment* passAttachment)
    {
        return passAttachment->m_view;
    }
} // namespace RHI