#include "RHI/FrameScheduler.hpp"

#include "RHI/Context.hpp"
#include "RHI/Common/Hash.hpp"

namespace std
{
    template<>
    class hash<RHI::ImageViewCreateInfo>
    {
    public:
        inline size_t operator()(const RHI::ImageViewCreateInfo& createInfo) const
        {
            return RHI::HashAny(createInfo);
        }
    };

    template<>
    class hash<RHI::BufferViewCreateInfo>
    {
    public:
        inline size_t operator()(const RHI::BufferViewCreateInfo& createInfo) const
        {
            return RHI::HashAny(createInfo);
        }
    };
} // namespace std

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
        for (auto passAttachment : m_imagePassAttachments)
            delete passAttachment;
        for (auto passAttachment : m_bufferPassAttachments)
            delete passAttachment;
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
        : m_frameCount(0)
        , m_currentFrameIndex(0)
        , m_frameNumber(0)
        , m_context(context)
        , m_passList()
        , m_attachmentsRegistry(std::make_unique<AttachmentsRegistry>())
        , m_transientResourceAllocator(nullptr)
        , m_frameSize(0, 0)
        , m_swapchainImage(nullptr)
        , m_frameReadyFence()
    {
    }

    void FrameScheduler::SetBufferedFramesCount(uint32_t count)
    {
        m_frameCount = count;
    }

    uint32_t FrameScheduler::GetBufferedFramesCount() const
    {
        return m_frameCount;
    }

    uint32_t FrameScheduler::GetCurrentFrameIndex()
    {
        return m_currentFrameIndex;
    }

    void FrameScheduler::Begin()
    {
        auto& fence = GetFrameCurrentFence();
        fence.Wait();
        fence.Reset();

        m_swapchainImage = m_attachmentsRegistry->FindImage(m_attachmentsRegistry->m_swapchainAttachments.front());

        // prepare pass attachments
        for (auto _passAttachment = m_swapchainImage->firstUse;
             _passAttachment != nullptr;
             _passAttachment = _passAttachment->next)
        {
            auto swapchain = m_swapchainImage->swapchain;
            auto passAttachment = (SwapchainImagePassAttachment*)_passAttachment;
            auto imageIndex = swapchain->GetCurrentImageIndex();
            passAttachment->view = passAttachment->views[imageIndex];
        }

        for (auto pass : m_passList)
        {
            pass->m_commandLists.clear();
        }
    }

    void FrameScheduler::End()
    {
        auto& fence = GetFrameCurrentFence();
        for (auto pass : m_passList)
        {
            QueuePassSubmit(pass, &fence);
        }

        m_frameNumber++;
        m_currentFrameIndex = m_frameNumber % m_frameCount;
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
            pass->m_size = m_frameSize;

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

        std::unordered_map<ImageViewCreateInfo, Handle<ImageView>> imageViewsLut;
        std::unordered_map<BufferViewCreateInfo, Handle<ImageView>> bufferViewsLut;

        auto findOrCreateImageView = [&](const ImageViewCreateInfo& createInfo)
        {
            if (auto it = imageViewsLut.find(createInfo); it != imageViewsLut.end())
                return it->second;
            return imageViewsLut[createInfo] = m_context->CreateImageView(createInfo);
        };

        auto findOrCreateBufferView = [&](const BufferViewCreateInfo& createInfo)
        {
            if (auto it = bufferViewsLut.find(createInfo); it != bufferViewsLut.end())
                return it->second;
            return bufferViewsLut[createInfo] = m_context->CreateBufferView(createInfo);
        };

        for (auto pass : m_passList)
        {
            for (auto passAttachment : pass->m_imagePassAttachments)
            {
                auto swapchain = passAttachment->attachment->swapchain;
                if (swapchain == nullptr)
                {
                    if (passAttachment->view)
                        continue;

                    auto image = passAttachment->attachment->GetImage();
                    passAttachment->viewInfo.image = image;
                    passAttachment->view = findOrCreateImageView(passAttachment->viewInfo);
                    continue;
                }

                auto swapchainPassAttachment = (SwapchainImagePassAttachment*)passAttachment;
                for (uint32_t i = 0; i < swapchain->GetImagesCount(); i++)
                {
                    if (swapchainPassAttachment->views[i])
                        continue;
                    passAttachment->viewInfo.image = swapchain->GetImage(i);
                    swapchainPassAttachment->views[i] = findOrCreateImageView(passAttachment->viewInfo);
                }

                swapchainPassAttachment->view = swapchainPassAttachment->views[swapchain->GetCurrentImageIndex()];
            }

            for (auto passAttachment : pass->m_bufferPassAttachments)
            {
                auto buffer = passAttachment->attachment->GetBuffer();
                if (passAttachment->view)
                    continue;
                passAttachment->viewInfo.buffer = buffer;
                passAttachment->view = findOrCreateBufferView(passAttachment->viewInfo);
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
        return *m_frameReadyFence[GetCurrentFrameIndex()];
    }

} // namespace RHI