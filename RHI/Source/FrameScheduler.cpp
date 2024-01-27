 #include "RHI/FrameScheduler.hpp"

#include "RHI/Context.hpp"

namespace RHI
{

    template<typename T>
    inline static uint64_t HashAny(const T& data)
    {
        auto stream = std::string(reinterpret_cast<const char*>(&data), sizeof(data));
        std::hash<std::string> hasher;
        return hasher(stream);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Pass
    //////////////////////////////////////////////////////////////////////////////////////////

    Pass::Pass(const char* name, QueueType type)
        : m_name(name)
        , m_queueType(type)
    {
    }

    ImagePassAttachment* Pass::UseColorAttachment(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, ColorValue value, LoadStoreOperations loadStoreOperations)
    {
        auto passAttachment = EmplaceNewPassAttachment(attachment);
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::Color;
        passAttachment->access = AttachmentAccess::None;
        passAttachment->viewInfo = viewInfo;
        passAttachment->clearValue = value;
        passAttachment->loadStoreOperations = loadStoreOperations;
        return passAttachment;
    }

    ImagePassAttachment* Pass::UseDepthAttachment(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, DepthStencilValue value, LoadStoreOperations loadStoreOperations)
    {
        auto passAttachment = EmplaceNewPassAttachment(attachment);
        memset(passAttachment, 0, sizeof(decltype(*passAttachment)));
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::Depth;
        passAttachment->access = AttachmentAccess::None;
        passAttachment->viewInfo = viewInfo;
        passAttachment->clearValue = value;
        passAttachment->loadStoreOperations = loadStoreOperations;
        return passAttachment;
    }

    ImagePassAttachment* Pass::UseStencilAttachment(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, DepthStencilValue value, LoadStoreOperations loadStoreOperations)
    {
        auto passAttachment = EmplaceNewPassAttachment(attachment);
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::Stencil;
        passAttachment->access = AttachmentAccess::None;
        passAttachment->viewInfo = viewInfo;
        passAttachment->clearValue = value;
        passAttachment->loadStoreOperations = loadStoreOperations;
        return passAttachment;
    }

    ImagePassAttachment* Pass::UseDepthStencilAttachment(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, DepthStencilValue value, LoadStoreOperations loadStoreOperations)
    {
        auto passAttachment = EmplaceNewPassAttachment(attachment);
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::DepthStencil;
        passAttachment->access = AttachmentAccess::None;
        passAttachment->viewInfo = viewInfo;
        passAttachment->clearValue = value;
        passAttachment->loadStoreOperations = loadStoreOperations;
        return passAttachment;
    }

    ImagePassAttachment* Pass::UseShaderImageResource(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo)
    {
        auto passAttachment = EmplaceNewPassAttachment(attachment);
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::ShaderResource;
        passAttachment->access = AttachmentAccess::Read;
        passAttachment->viewInfo = viewInfo;
        return passAttachment;
    }

    BufferPassAttachment* Pass::UseShaderBufferResource(BufferAttachment* attachment, const BufferViewCreateInfo& viewInfo)
    {
        auto passAttachment = EmplaceNewPassAttachment(attachment);
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::Color;
        passAttachment->access = AttachmentAccess::None;
        passAttachment->viewInfo = viewInfo;
        return passAttachment;
    }

    ImagePassAttachment* Pass::UseShaderImageStorage(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, AttachmentAccess access)
    {
        auto passAttachment = EmplaceNewPassAttachment(attachment);
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::Color;
        passAttachment->access = access;
        passAttachment->viewInfo = viewInfo;
        return passAttachment;
    }

    BufferPassAttachment* Pass::UseShaderBufferStorage(BufferAttachment* attachment, const BufferViewCreateInfo& viewInfo, AttachmentAccess access)
    {
        auto passAttachment = EmplaceNewPassAttachment(attachment);
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::Color;
        passAttachment->access = access;
        passAttachment->viewInfo = viewInfo;
        return passAttachment;
    }

    ImagePassAttachment* Pass::UseCopyImageResource(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, AttachmentAccess access)
    {
        auto passAttachment = EmplaceNewPassAttachment(attachment);
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::Color;
        passAttachment->access = access;
        passAttachment->viewInfo = viewInfo;
        return passAttachment;
    }

    BufferPassAttachment* Pass::UseCopyBufferResource(BufferAttachment* attachment, const BufferViewCreateInfo& viewInfo, AttachmentAccess access)
    {
        auto passAttachment = EmplaceNewPassAttachment(attachment);
        passAttachment->pass = this;
        passAttachment->attachment = attachment;
        passAttachment->usage = AttachmentUsage::Color;
        passAttachment->access = access;
        passAttachment->viewInfo = viewInfo;
        return passAttachment;
    }

    ImagePassAttachment* Pass::EmplaceNewPassAttachment(ImageAttachment* attachment)
    {
        auto passAttachment = m_imagePassAttachments.emplace_back(std::make_unique<ImagePassAttachment>()).get();
        attachment->PushPassAttachment(passAttachment);
        return passAttachment;
    }

    BufferPassAttachment* Pass::EmplaceNewPassAttachment(BufferAttachment* attachment)
    {
        auto passAttachment = m_bufferPassAttachments.emplace_back(std::make_unique<BufferPassAttachment>()).get();
        attachment->PushPassAttachment(passAttachment);
        return passAttachment;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// FrameScheduler
    //////////////////////////////////////////////////////////////////////////////////////////

    FrameScheduler::FrameScheduler(Context* context)
        : m_context(context)
        , m_passList()
        , m_imageViewsLut()
        , m_bufferViewsLut()
        , m_attachmentsRegistry(std::make_unique<AttachmentsRegistry>())
        , m_transientResourceAllocator()
        , m_maxFrameBufferingCount(0)
        , m_currentFrameIndex(0)
        , m_frameSize()
    {
        m_fences.resize(3);
        for (auto i = 0; i < 3; i++)
        {
            m_fences[i] = m_context->CreateFence();
        }
    }

    void FrameScheduler::Begin()
    {
        auto fence = GetCurrentFrameFence();
        
        fence->Wait(UINT64_MAX);
        fence->Reset();

        for (auto id : m_attachmentsRegistry->m_swapchainAttachments)
        {
            auto attachment = m_attachmentsRegistry->FindImage(id);
            for (auto passAttachment = attachment->firstUse; passAttachment != nullptr; passAttachment = passAttachment->next)
            {
                passAttachment->view = FindOrCreateView(attachment->GetImage(), passAttachment->viewInfo);
            }
        }
    }

    void FrameScheduler::End()
    {
        auto fence = GetCurrentFrameFence();

        for (auto pass : m_passList)
        {
            QueuePassSubmit(pass, nullptr);
        }

        QueueImagePresent(m_swapchainAttachment, fence);
    }

    void FrameScheduler::RegisterPass(Pass& pass)
    {
        m_passList.push_back(&pass);
    }

    void FrameScheduler::ResizeFrame(ImageSize2D newSize)
    {
        DeviceWaitIdle();

        // Cleanup(); need to be called after initial compilation

        m_frameSize = newSize;

        // Resize all transient attachments
        for (auto& [_, attachment] : m_attachmentsRegistry->m_imageAttachments)
        {
            if (attachment->lifetime == Attachment::Lifetime::Transient)
            {
                attachment->info.size.width = newSize.width;
                attachment->info.size.height = newSize.height;
                attachment->info.size.depth = 1;
            }
        }

        Compile();
    }

    void FrameScheduler::Compile()
    {
        m_swapchainAttachment = m_attachmentsRegistry->FindImage(m_attachmentsRegistry->m_swapchainAttachments.front());

        // Allocate transient resources, and generate resource views
        for (auto pass : m_passList)
        {
            for (auto& passAttachment : pass->m_imagePassAttachments)
            {
                auto& attachemnt = passAttachment->attachment;

                if (attachemnt->info.type == ImageType::Image2D)
                {
                    attachemnt->info.size.width = m_frameSize.width;
                    attachemnt->info.size.height = m_frameSize.height;
                    attachemnt->info.size.depth = 1;
                }

                if (attachemnt->lifetime == Attachment::Lifetime::Transient)
                {
                    if (passAttachment->prev == nullptr)
                    {
                        m_transientResourceAllocator->Allocate(m_context, attachemnt);
                    }
                    else if (passAttachment->next == nullptr)
                    {
                        m_transientResourceAllocator->Release(m_context, attachemnt);
                    }
                }

                passAttachment->view = FindOrCreateView(attachemnt->GetImage(), passAttachment->viewInfo);
            }

            for (auto& passAttachment : pass->m_bufferPassAttachments)
            {
                auto& attachemnt = passAttachment->attachment;
                if (attachemnt->lifetime == Attachment::Lifetime::Transient)
                {
                    if (passAttachment->prev == nullptr)
                    {
                        m_transientResourceAllocator->Allocate(m_context, attachemnt);
                    }
                    else if (passAttachment->next == nullptr)
                    {
                        m_transientResourceAllocator->Release(m_context, attachemnt);
                    }
                }

                passAttachment->view = FindOrCreateView(attachemnt->GetBuffer(), passAttachment->viewInfo);
            }
        }
    }

    void FrameScheduler::ExecuteCommandList(TL::Span<CommandList*> commandLists, Fence& signalFence)
    {
        QueueCommandsSubmit(RHI::QueueType::Graphics, commandLists, &signalFence);
    }

    void FrameScheduler::Cleanup()
    {
        for (auto [_, view] : m_imageViewsLut)
        {
            m_context->DestroyImageView(view);
        }

        for (auto [_, view] : m_bufferViewsLut)
        {
            m_context->DestroyBufferView(view);
        }

        for (auto& pass : m_passList)
        {
            for (auto& passAttachment : pass->m_imagePassAttachments)
            {
                auto attachment = passAttachment->attachment;

                if (attachment->lifetime == Attachment::Lifetime::Transient)
                {
                    m_transientResourceAllocator->Destroy(m_context, attachment);
                }
            }

            for (auto& passAttachment : pass->m_bufferPassAttachments)
            {
                auto attachment = passAttachment->attachment;
                if (attachment->lifetime == Attachment::Lifetime::Transient)
                {
                    m_transientResourceAllocator->Destroy(m_context, attachment);
                }
            }
        }

        m_transientResourceAllocator->Reset(m_context);
    }

    Fence* FrameScheduler::GetCurrentFrameFence()
    {
        return m_fences[m_currentFrameIndex].get();
    }

    Handle<ImageView> FrameScheduler::FindOrCreateView(Handle<Image> handle, const ImageViewCreateInfo& createInfo)
    {
        struct ImageViewKey
        {
            Handle<Image> handle;
            ImageViewCreateInfo createInfo;
        };

        ImageViewKey lookupKey{};
        lookupKey.createInfo = createInfo;
        lookupKey.handle = handle;

        auto key = HashAny(lookupKey);

        if (auto it = m_imageViewsLut.find(key); it != m_imageViewsLut.end())
        {
            return it->second;
        }

        return m_imageViewsLut[key] = m_context->CreateImageView(handle, createInfo);
    }

    Handle<BufferView> FrameScheduler::FindOrCreateView(Handle<Buffer> handle, const BufferViewCreateInfo& createInfo)
    {
        struct BufferViewKey
        {
            Handle<Image> handle;
            BufferViewCreateInfo createInfo;
        };

        BufferViewKey lookupKey{};
        lookupKey.createInfo = createInfo;
        lookupKey.handle = handle;

        auto key = HashAny(lookupKey);

        if (auto it = m_bufferViewsLut.find(key); it != m_bufferViewsLut.end())
        {
            return it->second;
        }

        return m_bufferViewsLut[key] = m_context->CreateBufferView(handle, createInfo);
    }

} // namespace RHI