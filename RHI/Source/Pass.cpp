#include "RHI/Pass.hpp"
#include "RHI/FrameScheduler.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Swapchain.hpp"

namespace RHI
{
    Pass::Pass(FrameScheduler* scheduler, const char* name, QueueType type)
        : m_scheduler(scheduler)
        , m_name(name)
        , m_queueType(type)
    {
    }

    void Pass::SetSize(ImageSize2D size)
    {
        m_frameSize = size;
    }

    ImagePassAttachment* Pass::UseImageAttachment(ImageAttachment* attachment, const ImageAttachmentUseInfo& useInfo)
    {
        auto passAttachment = EmplacePtr<ImagePassAttachment>(m_imagePassAttachments, attachment, this);
        attachment->Insert(passAttachment);

        passAttachment->m_clearValue = useInfo.clearValue;
        passAttachment->m_loadStoreOperations = useInfo.loadStoreOperations;
        passAttachment->m_usage = useInfo.usage;
        passAttachment->m_viewInfo.subresource = useInfo.subresourceRange;
        passAttachment->m_viewInfo.components = useInfo.componentMapping;

        auto& viewType = passAttachment->m_viewInfo.viewType;
        auto arrayView = passAttachment->m_viewInfo.subresource.arrayCount > 1;
        // NOTE: for imported resources, we don't know the type of the image, so pass attachments must store the use info instead of the image view create info
        switch (attachment->As<ImageAttachment>()->GetCreateInfo().type)
        {
        case ImageType::Image1D: viewType = arrayView ? ImageViewType::View2DArray : ImageViewType::View1D; break;
        case ImageType::Image2D: viewType = arrayView ? ImageViewType::View2DArray : ImageViewType::View2D; break;
        case ImageType::Image3D: viewType = ImageViewType::View3D;
        default:                 RHI_UNREACHABLE(); break;
        }

        auto formatInfo = GetFormatInfo(attachment->GetCreateInfo().format);
        if (formatInfo.hasDepth)
        {
            passAttachment->m_usage = ImageUsage::Depth;
            passAttachment->m_viewInfo.subresource.imageAspects = ImageAspect::Depth;
            m_depthStencilAttachment = passAttachment;
        }
        else
        {
            passAttachment->m_usage = ImageUsage::Color;
            m_colorAttachments.push_back(passAttachment);
        }

        return passAttachment;
    }

    BufferPassAttachment* Pass::UseBufferAttachment(BufferAttachment* attachment, const BufferAttachmentUseInfo& useInfo)
    {
        auto passAttachment = EmplacePtr<BufferPassAttachment>(m_bufferPassAttachments, attachment, this);
        attachment->Insert(passAttachment);
        passAttachment->m_usage = useInfo.usage;
        passAttachment->m_viewInfo.byteOffset = useInfo.offset;
        passAttachment->m_viewInfo.byteSize = useInfo.size;
        passAttachment->m_viewInfo.format = useInfo.format;
        return passAttachment;
    }

    void Pass::Submit(TL::Span<CommandList*> commandList)
    {
        m_commandLists.insert(m_commandLists.end(), commandList.begin(), commandList.end());
    }

} // namespace RHI