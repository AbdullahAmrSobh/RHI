#pragma once
#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Attachments.hpp"
#include "RHI/QueueType.hpp"

#include "RHI/Common/Containers.h"
#include "RHI/Common/Ptr.hpp"

#include <string>

namespace RHI
{
    namespace Vulkan
    {
        class IFrameScheduler;
        class ICommandList;
    } // namespace Vulkan

    class FrameScheduler;
    class CommandList;

    struct ImageAttachmentCreateInfo
    {
        const char*       name;
        Format            format;
        Flags<ImageUsage> usage;
        ImageType         type;
        ImageSize3D       size;
        SampleCount       sampleCount;
        uint32_t          mipLevels;
        uint32_t          arraySize;
    };

    struct BufferAttachmentCreateInfo
    {
        const char*        name;
        Flags<BufferUsage> usage;
        size_t             size;
    };

    struct ImageAttachmentUseInfo
    {
        ImageUsage            usage;               /// The usage of the image attachment.
        ImageSubresourceRange subresourceRange;    /// The subresource range of the image.
        ComponentMapping      componentMapping;    /// The component mapping for the image.
        LoadStoreOperations   loadStoreOperations; /// The load and store operations for the image when used as a render target.
        ClearValue            clearValue;          /// The clear value for the image when used as a render target.
    };

    struct BufferAttachmentUseInfo
    {
        BufferUsage usage;  /// The usage of the buffer attachment.
        size_t      size;   /// The size of the buffer attachment.
        size_t      offset; /// The offset of the buffer attachment.
        Format      format; /// The format of the buffer attachment.
    };

    struct PassCreateInfo
    {
        const char* name;
        QueueType   queueType;
    };

    class RHI_EXPORT Pass
    {
    public:
        Pass(FrameScheduler* scheduler, const char* name, QueueType type);
        ~Pass() = default;

        inline QueueType                GetQueueType() const { return m_queueType; }

        void                            SetSize(ImageSize2D size);

        ImagePassAttachment*            UseImageAttachment(ImageAttachment* attachment, const ImageAttachmentUseInfo& useInfo);

        BufferPassAttachment*           UseBufferAttachment(BufferAttachment* attachment, const BufferAttachmentUseInfo& useInfo);

        void                            Submit(TL::Span<CommandList*> commandList);

        TL::Span<ImagePassAttachment*>  GetColorAttachments() { return m_colorAttachments; }

        ImagePassAttachment*            GetDepthStencilAttachment() { return m_depthStencilAttachment; }

        TL::Span<ImagePassAttachment*>  GetImageShaderResources() { return m_imageShaderResources; }

        TL::Span<ImagePassAttachment*>  GetImageCopyResources() { return m_imageCopyResources; }

        TL::Span<BufferPassAttachment*> GetBufferShaderResources() { return m_bufferShaderResources; }

        TL::Span<BufferPassAttachment*> GetBufferCopyResources() { return m_bufferCopyResources; }

        TL::Span<PassAttachment*>       GetPassAttachments() { return m_passAttachments; } // return transient only

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
    };
} // namespace RHI