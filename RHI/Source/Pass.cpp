#include "RHI/Pass.hpp"

namespace RHI 
{
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
        passAttachment->clearValue.colorValue = value;
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
        passAttachment->clearValue.depthStencilValue = value;
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
        passAttachment->clearValue.depthStencilValue = value;
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
        passAttachment->clearValue.depthStencilValue = value;
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
}