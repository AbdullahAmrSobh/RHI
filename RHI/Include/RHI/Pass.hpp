#pragma once
#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Attachments.hpp"

#include "RHI/Common/Containers.h"

#include <string>

namespace RHI
{
    namespace Vulkan
    {
        class IFrameScheduler;
        class ICommandList;
    } // namespace Vulkan

    class FrameScheduler;

    enum class QueueType
    {
        Graphics,
        Compute,
        Transfer,
        Count,
    };

    class RHI_EXPORT Pass
    {
    public:
        Pass(FrameScheduler* scheduler, const char* name, QueueType type);
        ~Pass() = default;

        void                  SetRenderTargetSize(ImageSize2D size);

        QueueType             GetQueueType() const { return m_queueType; }

        ImagePassAttachment*  CreateRenderTarget(const char* name, Format format, ClearValue clearValue, LoadStoreOperations loadStoreOps = {}, uint32_t mipLevelsCount = 1, uint32_t arrayLayersCount = 1);

        ImagePassAttachment*  CreateTransientImage(const char* name, Format format, ImageUsage usage, ImageSize2D size, ImageSubresourceRange subresource = {});

        ImagePassAttachment*  CreateTransientImage(const char* name, Format format, ImageUsage usage, ImageSize3D size, ImageSubresourceRange subresource = {});

        BufferPassAttachment* CreateTransientBuffer(const char* name, BufferUsage usage, size_t size);

        ImagePassAttachment*  UseRenderTarget(const char* name, Handle<Image> handle, ClearValue clearValue, LoadStoreOperations loadStoreOps = {}, ImageSubresourceRange subresource = {});

        ImagePassAttachment*  UseRenderTarget(const char* name, Swapchain* swapchain, ClearValue clearValue, LoadStoreOperations loadStoreOps = {}, ImageSubresourceRange subresource = {});

        ImagePassAttachment*  UseRenderTarget(ImagePassAttachment* attachment, ClearValue clearValue, LoadStoreOperations loadStoreOps = {}, ImageSubresourceRange subresource = {});

        ImagePassAttachment*  UseImageResource(ImagePassAttachment* attachment, ImageUsage usage, Access access, ImageSubresourceRange subresource = {}, ComponentMapping mapping = {});

        BufferPassAttachment* UseBufferResource(BufferPassAttachment* attachment, BufferUsage usage, Access access, BufferSubregion subregion);

        void                  Submit(TL::Span<class CommandList*> commandList);

    public:
        TL::Span<ImagePassAttachment*>  GetColorAttachments() { return m_colorAttachments; }

        ImagePassAttachment*            GetDepthStencilAttachment() { return m_depthStencilAttachment; }

        TL::Span<ImagePassAttachment*>  GetImageShaderResources() { return m_imageShaderResources; }

        TL::Span<ImagePassAttachment*>  GetImageCopyResources() { return m_imageCopyResources; }

        TL::Span<BufferPassAttachment*> GetBufferShaderResources() { return m_bufferShaderResources; }

        TL::Span<BufferPassAttachment*> GetBufferCopyResources() { return m_bufferCopyResources; }

        TL::Span<PassAttachment*>       GetPassAttachments() { return m_passAttachments; } // return transient only

    private:
        ImagePassAttachment* UseRenderTargetInternal(
            ImageAttachment*      attachment,
            ClearValue            clearValue,
            LoadStoreOperations   loadStoreOps,
            ImageSubresourceRange subresourceRange);

        ImagePassAttachment* UseImageResourceInternal(
            ImageAttachment*      attachment,
            ImageUsage            usage,
            Access                access,
            ImageSubresourceRange subresource,
            ComponentMapping      mapping);

        BufferPassAttachment* UseBufferResourceInternal(
            BufferAttachment* attachment,
            BufferUsage       usage,
            Access            access,
            BufferSubregion   subregion);

        void UseAttachment(PassAttachment* attachment);

    protected:
        friend class CommandList;
        friend class FrameScheduler;
        friend class Vulkan::IFrameScheduler;
        friend class Vulkan::ICommandList;

        FrameScheduler*                       m_scheduler;
        std::string                           m_name;
        QueueType                             m_queueType;
        ImageSize2D                           m_frameSize;

        TL::Vector<Ptr<ImagePassAttachment>>  m_imagePassAttachments;
        TL::Vector<Ptr<BufferPassAttachment>> m_bufferPassAttachments;
        TL::Vector<CommandList*>              m_commandLists;

        TL::Vector<PassAttachment*>           m_passAttachments;
        TL::Vector<ImagePassAttachment*>      m_colorAttachments;
        ImagePassAttachment*                  m_depthStencilAttachment;
        TL::Vector<ImagePassAttachment*>      m_imageShaderResources;
        TL::Vector<ImagePassAttachment*>      m_imageCopyResources;
        TL::Vector<BufferPassAttachment*>     m_bufferShaderResources;
        TL::Vector<BufferPassAttachment*>     m_bufferCopyResources;

        // NodeIndex                              m_node;
        // RenderGraph*                           m_renderGraph;
    };
} // namespace RHI