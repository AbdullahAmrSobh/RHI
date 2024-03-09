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

    Pass::~Pass() {}

    void Pass::SetRenderTargetSize(ImageSize2D size)
    {
        m_frameSize = size;
    }

    ImagePassAttachment* Pass::CreateRenderTarget(const char* name, Format format, ClearValue clearValue, LoadStoreOperations loadStoreOps, uint32_t mipLevelsCount, uint32_t arrayLayersCount)
    {
        auto pool = m_scheduler->m_attachmentsPool.get();

        ImageSize3D size{};
        size.width = size.width;
        size.height = size.height;
        size.depth = 1;

        ImageSubresourceRange subresourceRange{};
        subresourceRange.arrayCount = arrayLayersCount;
        subresourceRange.mipLevelCount = mipLevelsCount;
        auto attachment = pool->NewImageAttachment(name, format, ImageType::Image2D, size, SampleCount::Samples1, mipLevelsCount, arrayLayersCount);
        return UseRenderTargetInternal(attachment, clearValue, loadStoreOps, subresourceRange);
    }

    ImagePassAttachment* Pass::CreateTransientImage(const char* name, Format format, ImageUsage usage, ImageSize2D _size, ImageSubresourceRange subresource)
    {
        auto pool = m_scheduler->m_attachmentsPool.get();

        ImageSize3D size{};
        size.width = _size.width;
        size.height = _size.height;
        size.depth = 1;

        auto attachment = pool->NewImageAttachment(name, format, ImageType::Image2D, size, SampleCount::Samples1, subresource.mipLevelCount, subresource.arrayCount);
        return UseImageResourceInternal(attachment, usage, Access::ReadWrite, subresource, ComponentMapping{});
    }

    ImagePassAttachment* Pass::CreateTransientImage(const char* name, Format format, ImageUsage usage, ImageSize3D size, ImageSubresourceRange subresource)
    {
        auto pool = m_scheduler->m_attachmentsPool.get();

        auto attachment = pool->NewImageAttachment(name, format, ImageType::Image3D, size, SampleCount::Samples1, subresource.mipLevelCount, subresource.arrayCount);
        return UseImageResourceInternal(attachment, usage, Access::ReadWrite, subresource, ComponentMapping{});
    }

    BufferPassAttachment* Pass::CreateTransientBuffer(const char* name, BufferUsage usage, size_t size)
    {
        auto pool = m_scheduler->m_attachmentsPool.get();

        BufferSubregion subregion = {};
        subregion.byteSize = size;
        auto attachment = pool->NewBufferAttachment(name, size);
        return UseBufferResourceInternal(attachment, usage, Access::ReadWrite, subregion);
    }

    ImagePassAttachment* Pass::UseRenderTarget(ImagePassAttachment* attachment, ClearValue clearValue, LoadStoreOperations loadStoreOps, ImageSubresourceRange subresource)
    {
        return UseRenderTargetInternal(attachment->GetAttachment(), clearValue, loadStoreOps, subresource);
    }

    ImagePassAttachment* Pass::UseRenderTarget(const char* name, Handle<Image> handle, ClearValue clearValue, LoadStoreOperations loadStoreOps, ImageSubresourceRange subresource)
    {
        auto pool = m_scheduler->m_attachmentsPool.get();
        auto attachment = pool->NewImageAttachment(name, handle);
        return UseRenderTargetInternal(attachment, clearValue, loadStoreOps, subresource);
    }

    ImagePassAttachment* Pass::UseRenderTarget(const char* name, Swapchain* swapchain, ClearValue clearValue, LoadStoreOperations loadStoreOps, ImageSubresourceRange subresource)
    {
        auto pool = m_scheduler->m_attachmentsPool.get();
        auto attachment = pool->NewImageAttachment(name, swapchain->GetImage());
        attachment->m_swapchain = swapchain;
        return UseRenderTargetInternal(attachment, clearValue, loadStoreOps, subresource);
    }

    ImagePassAttachment* Pass::UseImageResource(ImagePassAttachment* attachment, ImageUsage usage, Access access, ImageSubresourceRange subresource, ComponentMapping mapping)
    {
        return UseImageResourceInternal(attachment->GetAttachment(), usage, access, subresource, mapping);
    }

    BufferPassAttachment* Pass::UseBufferResource(BufferPassAttachment* attachment, BufferUsage usage, Access access, BufferSubregion subregion)
    {
        return UseBufferResourceInternal(attachment->GetAttachment(), usage, access, subregion);
    }

    void Pass::SubmitCommandList(TL::Span<CommandList*> commandList)
    {
        m_commandLists.insert(m_commandLists.end(), commandList.begin(), commandList.end());
    }

    ImagePassAttachment* Pass::UseRenderTargetInternal(
        ImageAttachment* attachment,
        ClearValue clearValue,
        LoadStoreOperations loadStoreOps,
        ImageSubresourceRange subresourceRange)
    {
        auto passAttachment = m_imagePassAttachments.emplace_back(CreatePtr<ImagePassAttachment>(attachment, this)).get();
        passAttachment->m_clearValue = clearValue;
        passAttachment->m_loadStoreOperations = loadStoreOps;
        passAttachment->m_viewInfo.subresource = subresourceRange;
        passAttachment->m_access = Access::Write;

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

        attachment->Insert(passAttachment);

        UseAttachment(passAttachment);


        return passAttachment;
    }

    ImagePassAttachment* Pass::UseImageResourceInternal(
        ImageAttachment* attachment,
        ImageUsage usage,
        Access access,
        ImageSubresourceRange subresource,
        ComponentMapping mapping)
    {
        auto passAttachment = m_imagePassAttachments.emplace_back(CreatePtr<ImagePassAttachment>(attachment, this)).get();
        passAttachment->m_usage = usage;
        passAttachment->m_access = access;
        passAttachment->m_viewInfo.subresource = subresource;
        passAttachment->m_viewInfo.components = mapping;
        attachment->Insert(passAttachment);

        UseAttachment(passAttachment);

        return passAttachment;
    }

    BufferPassAttachment* Pass::UseBufferResourceInternal(
        BufferAttachment* attachment,
        BufferUsage usage,
        Access access,
        BufferSubregion subregion)
    {
        auto passAttachment = m_bufferPassAttachments.emplace_back(CreatePtr<BufferPassAttachment>(attachment, this)).get();
        passAttachment->m_usage = usage;
        passAttachment->m_access = access;
        passAttachment->m_viewInfo.byteOffset = subregion.byteOffset;
        passAttachment->m_viewInfo.byteSize = subregion.byteSize;
        attachment->Insert(passAttachment);

        UseAttachment(passAttachment);

        return passAttachment;
    }

    void Pass::UseAttachment(PassAttachment* attachment)
    {
        (void)(attachment);
        // auto producer = attachment->m_pass->m_node;
        // auto consuemr = m_node;
        // m_renderGraph->AddEdge(producer, consuemr);
    }

} // namespace RHI