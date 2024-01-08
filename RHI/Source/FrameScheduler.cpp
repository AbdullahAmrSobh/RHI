#include "RHI/FrameScheduler.hpp"

#include "RHI/Context.hpp"

namespace RHI
{

    //////////////////////////////////////////////////////////////////////////////////////////
    /// ImageAttachment
    //////////////////////////////////////////////////////////////////////////////////////////

    ImageAttachment::ImageAttachment(const char* name, Handle<Image> handle)
        : Attachment(name, Attachment::Lifetime::Persistent, Attachment::Type::Image)
        , swapchain(nullptr)
        , handle(handle)
        , info()
        , firstUse(nullptr)
        , lastUse(nullptr)
    {
    }

    ImageAttachment::ImageAttachment(const char* name, Swapchain* swapchain)
        : Attachment(name, Attachment::Lifetime::Persistent, Attachment::Type::Image)
        , swapchain(swapchain)
        , handle(swapchain->GetImage())
        , info()
        , firstUse(nullptr)
        , lastUse(nullptr)
    {
    }

    ImageAttachment::ImageAttachment(const char* name, const ImageCreateInfo& createInfo)
        : Attachment(name, Attachment::Lifetime::Transient, Attachment::Type::Image)
        , swapchain(nullptr)
        , handle()
        , info(createInfo)
        , firstUse(nullptr)
        , lastUse(nullptr)
    {
    }

    void ImageAttachment::PushPassAttachment(ImagePassAttachment* passAttachment)
    {
        if (firstUse && lastUse)
        {
            lastUse->next = passAttachment;
            lastUse = lastUse->next;
        }
        else
        {
            firstUse = passAttachment;
            lastUse = passAttachment;
        }
    }

    void ImageAttachment::Reset()
    {
        firstUse = nullptr;
        lastUse = nullptr;
    }

    Handle<Image> ImageAttachment::GetImage()
    {
        if (swapchain)
        {
            return swapchain->GetImage();
        }

        return handle;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// BufferAttachment
    //////////////////////////////////////////////////////////////////////////////////////////

    BufferAttachment::BufferAttachment(const char* name, Handle<Buffer> handle)
        : Attachment(name, Attachment::Lifetime::Persistent, Attachment::Type::Buffer)
        , handle(handle)
        , info()
        , firstUse(nullptr)
        , lastUse(nullptr)
    {
    }

    BufferAttachment::BufferAttachment(const char* name, const BufferCreateInfo& createInfo)
        : Attachment(name, Attachment::Lifetime::Transient, Attachment::Type::Buffer)
        , handle()
        , info(createInfo)
        , firstUse(nullptr)
        , lastUse(nullptr)
    {
    }

    void BufferAttachment::PushPassAttachment(BufferPassAttachment* passAttachment)
    {
        if (firstUse && lastUse)
        {
            lastUse->next = passAttachment;
            lastUse = lastUse->next;
        }
        else
        {
            firstUse = passAttachment;
            lastUse = passAttachment;
        }
    }

    void BufferAttachment::Reset()
    {
        firstUse = nullptr;
        lastUse = nullptr;
    }

    Handle<Image> BufferAttachment::GetBuffer()
    {
        return handle;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// AttachmentsRegistry
    //////////////////////////////////////////////////////////////////////////////////////////

    void AttachmentsRegistry::Reset()
    {
        m_imageAttachments.clear();
        m_bufferAttachments.clear();
        m_swapchainAttachments.clear();
    }

    ImageAttachment* AttachmentsRegistry::ImportSwapchainImage(const char* name, Swapchain* swapchain)
    {
        m_imageAttachments[name] = new ImageAttachment(name, swapchain);
        auto attachment = m_imageAttachments[name];
        m_swapchainAttachments.push_back(name);
        swapchain->m_attachment = attachment;
        return attachment;
    }

    ImageAttachment* AttachmentsRegistry::ImportImage(const char* name, Handle<Image> handle)
    {
        m_imageAttachments[name] = new ImageAttachment(name, handle);
        auto attachment = m_imageAttachments[name];
        return attachment;
    }

    BufferAttachment* AttachmentsRegistry::ImportBuffer(const char* name, Handle<Buffer> handle)
    {
        m_bufferAttachments[name] = new BufferAttachment(name, handle);
        auto attachment = m_bufferAttachments[name];
        return attachment;
    }

    ImageAttachment* AttachmentsRegistry::CreateTransientImage(const char* name, const ImageCreateInfo& createInfo)
    {
        m_imageAttachments[name] = new ImageAttachment(name, createInfo);
        auto attachment = m_imageAttachments[name];
        return attachment;
    }

    BufferAttachment* AttachmentsRegistry::CreateTransientBuffer(const char* name, const BufferCreateInfo& createInfo)
    {
        m_bufferAttachments[name] = new BufferAttachment(name, createInfo);
        auto attachment = m_bufferAttachments[name];
        return attachment;
    }

    ImageAttachment* AttachmentsRegistry::FindImage(AttachmentID id)
    {
        if (auto it = m_imageAttachments.find(id); it != m_imageAttachments.end())
        {
            return it->second;
        }
        return nullptr;
    }

    BufferAttachment* AttachmentsRegistry::FindBuffer(AttachmentID id)
    {
        if (auto it = m_bufferAttachments.find(id); it != m_bufferAttachments.end())
        {
            return it->second;
        }
        return nullptr;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Pass
    //////////////////////////////////////////////////////////////////////////////////////////

    // FIXME: fix memory leaks here

    ImagePassAttachment* Pass::UseColorAttachment(ImageAttachment* attachment, ColorValue value, LoadStoreOperations loadStoreOperations, const ImageViewCreateInfo& viewInfo)
    {
        auto passAttachment = new ImagePassAttachment;
        memset(passAttachment, 0, sizeof(decltype(*passAttachment)));
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::Color;
        passAttachment->access = AttachmentAccess::None;
        passAttachment->viewInfo = viewInfo;
        passAttachment->clearValue = value;
        passAttachment->loadStoreOperations = loadStoreOperations;
        attachment->PushPassAttachment(passAttachment);
        m_imagePassAttachments.push_back(passAttachment);
        return passAttachment;
    }

    ImagePassAttachment* Pass::UseDepthAttachment(ImageAttachment* attachment, DepthStencilValue value, LoadStoreOperations loadStoreOperations, const ImageViewCreateInfo& viewInfo)
    {
        auto passAttachment = new ImagePassAttachment;
        memset(passAttachment, 0, sizeof(decltype(*passAttachment)));
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::Depth;
        passAttachment->access = AttachmentAccess::None;
        passAttachment->viewInfo = viewInfo;
        passAttachment->clearValue = value;
        passAttachment->loadStoreOperations = loadStoreOperations;
        attachment->PushPassAttachment(passAttachment);
        m_imagePassAttachments.push_back(passAttachment);
        return passAttachment;
    }

    ImagePassAttachment* Pass::UseStencilAttachment(ImageAttachment* attachment, DepthStencilValue value, LoadStoreOperations loadStoreOperations, const ImageViewCreateInfo& viewInfo)
    {
        auto passAttachment = new ImagePassAttachment;
        memset(passAttachment, 0, sizeof(decltype(*passAttachment)));
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::Stencil;
        passAttachment->access = AttachmentAccess::None;
        passAttachment->viewInfo = viewInfo;
        passAttachment->clearValue = value;
        passAttachment->loadStoreOperations = loadStoreOperations;
        attachment->PushPassAttachment(passAttachment);
        m_imagePassAttachments.push_back(passAttachment);
        return passAttachment;
    }

    ImagePassAttachment* Pass::UseDepthStencilAttachment(ImageAttachment* attachment, DepthStencilValue value, LoadStoreOperations loadStoreOperations, const ImageViewCreateInfo& viewInfo)
    {
        auto passAttachment = new ImagePassAttachment;
        memset(passAttachment, 0, sizeof(decltype(*passAttachment)));
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::DepthStencil;
        passAttachment->access = AttachmentAccess::None;
        passAttachment->viewInfo = viewInfo;
        passAttachment->clearValue = value;
        passAttachment->loadStoreOperations = loadStoreOperations;
        attachment->PushPassAttachment(passAttachment);
        m_imagePassAttachments.push_back(passAttachment);
        return passAttachment;
    }

    ImagePassAttachment* Pass::UseShaderImageResource(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo)
    {
        auto passAttachment = new ImagePassAttachment;
        memset(passAttachment, 0, sizeof(decltype(*passAttachment)));
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::ShaderResource;
        passAttachment->access = AttachmentAccess::Read;
        passAttachment->viewInfo = viewInfo;
        attachment->PushPassAttachment(passAttachment);
        m_imagePassAttachments.push_back(passAttachment);
        return passAttachment;
    }

    BufferPassAttachment* Pass::UseShaderBufferResource(BufferAttachment* attachment, const BufferViewCreateInfo& viewInfo)
    {
        auto passAttachment = new BufferPassAttachment;
        memset(passAttachment, 0, sizeof(decltype(*passAttachment)));
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::Color;
        passAttachment->access = AttachmentAccess::None;
        passAttachment->viewInfo = viewInfo;
        attachment->PushPassAttachment(passAttachment);
        m_bufferPassAttachment.push_back(passAttachment);
        return passAttachment;
    }

    ImagePassAttachment* Pass::UseShaderImageStorage(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, AttachmentAccess access)
    {
        auto passAttachment = new ImagePassAttachment;
        memset(passAttachment, 0, sizeof(decltype(*passAttachment)));
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::Color;
        passAttachment->access = access;
        passAttachment->viewInfo = viewInfo;
        attachment->PushPassAttachment(passAttachment);
        m_imagePassAttachments.push_back(passAttachment);
        return passAttachment;
    }

    BufferPassAttachment* Pass::UseShaderBufferStorage(BufferAttachment* attachment, const BufferViewCreateInfo& viewInfo, AttachmentAccess access)
    {
        auto passAttachment = new BufferPassAttachment;
        memset(passAttachment, 0, sizeof(decltype(*passAttachment)));
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::Color;
        passAttachment->access = access;
        passAttachment->viewInfo = viewInfo;
        attachment->PushPassAttachment(passAttachment);
        m_bufferPassAttachment.push_back(passAttachment);
        return passAttachment;
    }

    ImagePassAttachment* Pass::UseCopyImageResource(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, AttachmentAccess access)
    {
        auto passAttachment = new ImagePassAttachment;
        memset(passAttachment, 0, sizeof(decltype(*passAttachment)));
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::Color;
        passAttachment->access = access;
        passAttachment->viewInfo = viewInfo;
        attachment->PushPassAttachment(passAttachment);
        m_imagePassAttachments.push_back(passAttachment);
        return passAttachment;
    }

    BufferPassAttachment* Pass::UseCopyBufferResource(BufferAttachment* attachment, const BufferViewCreateInfo& viewInfo, AttachmentAccess access)
    {
        auto passAttachment = new BufferPassAttachment;
        memset(passAttachment, 0, sizeof(decltype(*passAttachment)));
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::Color;
        passAttachment->access = access;
        passAttachment->viewInfo = viewInfo;
        attachment->PushPassAttachment(passAttachment);
        m_bufferPassAttachment.push_back(passAttachment);
        return passAttachment;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// FrameScheduler
    //////////////////////////////////////////////////////////////////////////////////////////

    FrameScheduler::FrameScheduler(Context* context)
        : m_attachmentsRegistry(std::make_unique<AttachmentsRegistry>())
        , m_context(context)
    {
    }

    void FrameScheduler::Begin()
    {
        auto registry = GetRegistry();

        // FIXME: no need to iterate over all pass attachments, just update swapchain's
        for (auto pass : m_passList)
        {
            for (auto passAttachment : pass->m_imagePassAttachments)
            {
                auto attachment = passAttachment->attachment;
                passAttachment->view = FindOrCreateImageView(attachment->GetImage(), passAttachment->viewInfo);
            }
        }

        OnFrameBegin();
    }

    void FrameScheduler::End()
    {
        for (auto pass : m_passList)
        {
            ExecutePass(*pass);
            // clear the commandlists after execution.
            pass->m_commandLists.clear();
        }

        OnFrameEnd();
    }

    void FrameScheduler::RegisterPass(Pass& pass)
    {
        m_passList.push_back(&pass);
    }

    void FrameScheduler::Compile()
    {
        m_transientAttachmentAllocator->Begin();

        /// TODO: Add topological sort that minimize passes overlaping
        /// TODO: This should be replaced with more sophisticated breadth-first-search
        /// on the frame graph, to maximize the aliasing between non overlaping passes.
        for (auto pass : m_passList)
        {
            for (auto& passAttachment : pass->m_imagePassAttachments)
            {
                auto attachment = passAttachment->attachment;
                if (attachment->lifetime == Attachment::Lifetime::Transient &&
                    passAttachment->prev == nullptr)
                {
                    m_transientAttachmentAllocator->Allocate(attachment);
                }

                if (attachment->type == Attachment::Type::Buffer)
                {
                    passAttachment->view = FindOrCreateBufferView(attachment->GetImage(), passAttachment->viewInfo);
                }
                else
                {
                    passAttachment->view = FindOrCreateImageView(attachment->GetImage(), passAttachment->viewInfo);
                }
            }

            for (auto& passAttachment : pass->m_bufferPassAttachment)
            {
                auto attachment = passAttachment->attachment;
                if (attachment->lifetime == Attachment::Lifetime::Transient &&
                    passAttachment->next == nullptr)
                {
                    m_transientAttachmentAllocator->Free(attachment);
                }
            }
        }

        m_transientAttachmentAllocator->End();
    }

    Handle<ImageView> FrameScheduler::FindOrCreateImageView(Handle<Image> image, const ImageViewCreateInfo& createInfo)
    {
        if (auto it = m_imageViewsLut.find(image); it != m_imageViewsLut.end())
        {
            return it->second;
        }

        auto result = m_imageViewsLut[image] = m_context->CreateImageView(image, createInfo);
        return result;
    }

    Handle<BufferView> FrameScheduler::FindOrCreateBufferView(Handle<Buffer> buffer, const ImageViewCreateInfo& createInfo)
    {
        if (auto it = m_imageViewsLut.find(buffer); it != m_imageViewsLut.end())
        {
            return it->second;
        }

        auto result = m_imageViewsLut[buffer] = m_context->CreateImageView(buffer, createInfo);
        return result;
    }

} // namespace RHI