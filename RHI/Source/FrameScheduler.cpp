#include "RHI/FrameScheduler.hpp"

#include "RHI/Context.hpp"
#include "RHI/Common/Hash.hpp"

namespace RHI
{
    bool IsRenderTarget(Flags<AttachmentUsage> usage)
    {
        return usage & AttachmentUsage::Color || usage & AttachmentUsage::Depth || usage & AttachmentUsage::Stencil || usage & AttachmentUsage::DepthStencil;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Pass
    //////////////////////////////////////////////////////////////////////////////////////////

    Pass::Pass(const char* name, QueueType type)
        : m_name(name)
        , m_queueType(type)
        , m_swapchain(nullptr)
        , m_size(0, 0)
        , m_producers()
        , m_commandLists()
        , m_swapchainImageAttachment(nullptr)
        , m_imagePassAttachments()
        , m_bufferPassAttachments()
    {
    }

    Pass::~Pass()
    {
        for (auto passAttachment : m_imagePassAttachments) delete passAttachment;
        for (auto passAttachment : m_bufferPassAttachments) delete passAttachment;
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
        if (auto swapchain = attachment->swapchain; swapchain != nullptr)
        {
            m_imagePassAttachments.emplace_back(new SwapchainImagePassAttachment());
            m_swapchainImageAttachment = (SwapchainImagePassAttachment*)m_imagePassAttachments.back();
        }
        else
        {
            m_imagePassAttachments.emplace_back(new ImagePassAttachment());
        }

        auto passAttachment = m_imagePassAttachments.back();
        attachment->PushPassAttachment(passAttachment);
        return passAttachment;
    }

    BufferPassAttachment* Pass::EmplaceNewPassAttachment(BufferAttachment* attachment)
    {
        auto passAttachment = m_bufferPassAttachments.emplace_back(new BufferPassAttachment());
        attachment->PushPassAttachment(passAttachment);
        return passAttachment;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// FrameScheduler
    //////////////////////////////////////////////////////////////////////////////////////////

    FrameScheduler::FrameScheduler(Context* context)
        : m_context(context)
        , m_attachmentsRegistry(std::make_unique<AttachmentsRegistry>())
        , m_transientResourceAllocator(nullptr)
        , m_frameSize(0, 0)
        , m_swapchainImageAttachment(nullptr)
    {
    }

    void FrameScheduler::Begin()
    {
        // FIXME
        m_swapchainImageAttachment = m_attachmentsRegistry->FindImage(m_attachmentsRegistry->m_swapchainAttachments.front());

        auto& fence = GetFrameCurrentFence();
        fence.Wait();
        fence.Reset();

        // prepare pass attachments
        for (auto passAttachment = (SwapchainImagePassAttachment*)m_swapchainImageAttachment->firstUse; passAttachment->next != nullptr;
             passAttachment = (SwapchainImagePassAttachment*)passAttachment->next)
        {
            passAttachment->view = passAttachment->views[m_swapchainImageAttachment->swapchain->GetCurrentImageIndex()];
        }

        for (auto pass : m_passList)
        {
            pass->m_commandLists.clear();
        }
    }

    void FrameScheduler::End()
    {
        for (auto pass : m_passList)
        {
            QueuePassSubmit(pass, nullptr);
        }

        QueueImagePresent(m_swapchainImageAttachment, GetFrameCurrentFence());
    }

    void FrameScheduler::RegisterPass(Pass& pass)
    {
        m_passList.push_back(&pass);
    }

    void FrameScheduler::Compile()
    {
        // Allocate transient resources, and generate resource views
        for (auto pass : m_passList)
        {
            for (auto& passAttachment : pass->m_imagePassAttachments)
            {
                auto& attachemnt = passAttachment->attachment;
                if (attachemnt->lifetime == Attachment::Lifetime::Transient)
                {
                    if (attachemnt->info.type == ImageType::Image2D)
                    {
                        attachemnt->info.size.width = m_frameSize.width;
                        attachemnt->info.size.height = m_frameSize.height;
                        attachemnt->info.size.depth = 1;
                    }

                    if (passAttachment->prev == nullptr)
                        m_transientResourceAllocator->Allocate(m_context, attachemnt);
                    if (passAttachment->next == nullptr)
                        m_transientResourceAllocator->Release(m_context, attachemnt);
                }
            }

            for (auto& passAttachment : pass->m_bufferPassAttachments)
            {
                auto& attachemnt = passAttachment->attachment;
                if (attachemnt->lifetime == Attachment::Lifetime::Transient)
                {
                    if (passAttachment->prev == nullptr)
                        m_transientResourceAllocator->Allocate(m_context, attachemnt);
                    if (passAttachment->next == nullptr)
                        m_transientResourceAllocator->Release(m_context, attachemnt);
                }
            }
        }

        struct ImageViewKey
        {
            Handle<Image> handle;
            ImageViewCreateInfo createInfo;
        };

        struct BufferViewKey
        {
            Handle<Image> handle;
            BufferViewCreateInfo createInfo;
        };

        std::unordered_map<size_t, Handle<ImageView>> imageViewsLut;
        std::unordered_map<size_t, Handle<ImageView>> bufferViewsLut;

        auto findOrCreateImageView = [&](Handle<Image> handle, const ImageViewCreateInfo& createInfo)
        {
            ImageViewKey lookupKey{};
            lookupKey.createInfo = createInfo;
            lookupKey.handle = handle;
            auto key = HashAny(lookupKey);
            if (auto it = imageViewsLut.find(key); it != imageViewsLut.end())
                return it->second;
            return imageViewsLut[key] = m_context->CreateImageView(handle, createInfo);
        };

        auto findOrCreateBufferView = [&](Handle<Buffer> handle, const BufferViewCreateInfo& createInfo)
        {
            BufferViewKey lookupKey{};
            lookupKey.createInfo = createInfo;
            lookupKey.handle = handle;
            auto key = HashAny(lookupKey);
            if (auto it = bufferViewsLut.find(key); it != bufferViewsLut.end())
                return it->second;
            return bufferViewsLut[key] = m_context->CreateBufferView(handle, createInfo);
        };

        for (auto pass : m_passList)
        {
            for (auto passAttachment : pass->m_imagePassAttachments)
            {
                auto swapchain = passAttachment->attachment->swapchain;
                if (swapchain == nullptr)
                {
                    auto image = passAttachment->attachment->GetImage();
                    passAttachment->view = findOrCreateImageView(image, passAttachment->viewInfo);
                    continue;
                }
        
                auto swapchainPassAttachment = (SwapchainImagePassAttachment*)passAttachment;
                for (uint32_t i = 0; i < swapchain->GetImagesCount(); i++)
                {
                    swapchainPassAttachment->views[i] = findOrCreateImageView(swapchain->GetImage(i), passAttachment->viewInfo);
                }

                swapchainPassAttachment->view = swapchainPassAttachment->views[swapchain->GetCurrentImageIndex()];
            }

            for (auto passAttachment : pass->m_bufferPassAttachments)
            {
                auto buffer = passAttachment->attachment->GetBuffer();
                passAttachment->view = findOrCreateBufferView(buffer, passAttachment->viewInfo);
            }
        }
    }

    void FrameScheduler::ResizeFrame(ImageSize2D newSize)
    {
        m_frameSize = newSize;
        Cleanup();
        Compile();
    }

    void FrameScheduler::ExecuteCommandList(TL::Span<CommandList*> commandLists, Fence& fence)
    {
        QueueCommandsSubmit(QueueType::Graphics, commandLists, fence);
    }

    void FrameScheduler::Cleanup()
    {
        DeviceWaitIdle();

        m_transientResourceAllocator->Reset(m_context);
    }

    Fence& FrameScheduler::GetFrameCurrentFence()
    {
        auto swapchain = m_swapchainImageAttachment->swapchain;
        return swapchain->GetCurrentFrameFence();
    }

} // namespace RHI