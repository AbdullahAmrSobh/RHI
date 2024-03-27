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

    void FrameScheduler::Begin()
    {
        for (auto attachment : m_attachmentsPool->GetAttachments())
        {
            for (auto passAttachment = attachment->GetFirstPassAttachment();
                 passAttachment;
                 passAttachment = passAttachment->GetNext())
            {
                if (attachment->m_type == Attachment::Type::Image)
                {
                    auto imagePassAttachment = (ImagePassAttachment*)passAttachment;
                    imagePassAttachment->m_viewInfo.image = imagePassAttachment->GetAttachment()->GetHandle();

                    if (auto it = m_imageViewLUT.find(imagePassAttachment->m_viewInfo); it != m_imageViewLUT.end())
                    {
                        imagePassAttachment->m_view = it->second;
                    }
                    else
                    {
                        imagePassAttachment->m_view = m_context->CreateImageView(imagePassAttachment->m_viewInfo);
                        m_imageViewLUT[imagePassAttachment->m_viewInfo] = imagePassAttachment->m_view;
                    }
                    
                }
                else if (attachment->m_type == Attachment::Type::Buffer)
                {
                    auto bufferPassAttachment = (BufferPassAttachment*)passAttachment;
                    bufferPassAttachment->m_viewInfo.buffer = bufferPassAttachment->GetAttachment()->GetHandle();

                    if (auto it = m_bufferViewLUT.find(bufferPassAttachment->m_viewInfo); it != m_bufferViewLUT.end())
                    {
                        bufferPassAttachment->m_view = it->second;
                    }
                    else
                    {
                        bufferPassAttachment->m_view = m_context->CreateBufferView(bufferPassAttachment->m_viewInfo);
                        m_bufferViewLUT[bufferPassAttachment->m_viewInfo] = bufferPassAttachment->m_view;
                    }
                    
                }
                else
                {
                    RHI_UNREACHABLE();
                }
            }
        }
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
                                           ImageOffset offset,
                                           ImageSize3D size,
                                           ImageSubresourceLayers subresource,
                                           TL::Span<const uint8_t> content)
    {
        BufferCreateInfo tmpBufferCreateInfo {};
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
                auto createInfo = imageAttachment->GetCreateInfo();
                createInfo.size.width = 1600;
                createInfo.size.height = 1200;
                imageAttachment->SetHandle(m_context->CreateImage(createInfo).GetValue());
            }
            else if (attachment->m_type == Attachment::Type::Buffer)
            {
                auto bufferAttachment = attachment->As<BufferAttachment>();
                bufferAttachment->SetHandle(m_context->CreateBuffer(bufferAttachment->GetCreateInfo()).GetValue());
            }
            else
            {
                RHI_UNREACHABLE();
            }
        }
    }

    void FrameScheduler::Cleanup()
    {
        for (auto [_, view] : m_imageViewLUT)
        {
            m_context->DestroyImageView(view);
        }

        
        for (auto [_, view] : m_bufferViewLUT)
        {
            m_context->DestroyBufferView(view);
        }

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