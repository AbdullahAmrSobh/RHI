#pragma once
#include "RHI/Export.hpp"
#include "RHI/Attachments.hpp"
#include "RHI/Resources.hpp"

#include <string>

namespace RHI
{
    class CommandList;

    enum class QueueType
    {
        Graphics,
        Compute,
        Transfer,
    };

    /// @brief Represents a pass, which encapsulates a GPU task.
    class RHI_EXPORT Pass
    {
    public:
        Pass(const char* name, QueueType type);
        virtual ~Pass();

        inline ImagePassAttachment* UseColorAttachment(ImageAttachment* attachment, ColorValue value, LoadStoreOperations loadStoreOperations = LoadStoreOperations{})
        {
            return UseColorAttachment(attachment, ImageViewCreateInfo{ ImageAspect::Color }, value, loadStoreOperations);
        }

        inline ImagePassAttachment* UseDepthAttachment(ImageAttachment* attachment, DepthStencilValue value, LoadStoreOperations loadStoreOperations = LoadStoreOperations{})
        {
            return UseDepthAttachment(attachment, ImageViewCreateInfo{ ImageAspect::Depth }, value, loadStoreOperations);
        }

        inline ImagePassAttachment* UseStencilAttachment(ImageAttachment* attachment, DepthStencilValue value, LoadStoreOperations loadStoreOperations = LoadStoreOperations{})
        {
            return UseStencilAttachment(attachment, ImageViewCreateInfo{ ImageAspect::Stencil }, value, loadStoreOperations);
        }

        inline ImagePassAttachment* UseDepthStencilAttachment(ImageAttachment* attachment, DepthStencilValue value, LoadStoreOperations loadStoreOperations = LoadStoreOperations{})
        {
            return UseDepthStencilAttachment(attachment, ImageViewCreateInfo{ ImageAspect::DepthStencil }, value, loadStoreOperations);
        }

        ImagePassAttachment*  UseColorAttachment(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, ColorValue value, LoadStoreOperations loadStoreOperations);

        ImagePassAttachment*  UseDepthAttachment(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, DepthStencilValue value, LoadStoreOperations loadStoreOperations);

        ImagePassAttachment*  UseStencilAttachment(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, DepthStencilValue value, LoadStoreOperations loadStoreOperations);

        ImagePassAttachment*  UseDepthStencilAttachment(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, DepthStencilValue value, LoadStoreOperations loadStoreOperations);

        ImagePassAttachment*  UseShaderImageResource(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo);

        BufferPassAttachment* UseShaderBufferResource(BufferAttachment* attachment, const BufferViewCreateInfo& viewInfo);

        ImagePassAttachment*  UseShaderImageStorage(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, AttachmentAccess access);

        BufferPassAttachment* UseShaderBufferStorage(BufferAttachment* attachment, const BufferViewCreateInfo& viewInfo, AttachmentAccess access);

        ImagePassAttachment*  UseCopyImageResource(ImageAttachment* attachment, const ImageViewCreateInfo& viewInfo, AttachmentAccess access);

        BufferPassAttachment* UseCopyBufferResource(BufferAttachment* attachment, const BufferViewCreateInfo& viewInfo, AttachmentAccess access);

    private:
        ImagePassAttachment*  EmplaceNewPassAttachment(ImageAttachment* attachment);
        BufferPassAttachment* EmplaceNewPassAttachment(BufferAttachment* attachment);

    protected:
        friend class FrameScheduler;

        std::string                                 m_name;
        QueueType                                   m_queueType;    // The type of the Hardware Queue needed to execute this pass.
        ImageSize2D                                 m_size;         // The size of the rendering area in the render pass.
        std::vector<Pass*>                          m_producers;    // A list of all passes that this pass will wait for.
        std::vector<CommandList*>                   m_commandLists; // A list of command lists that executes this pass.

        RHI::SwapchainImagePassAttachment*          m_swapchainImageAttachment;
        // RHI::ImagePassAttachment*                   m_depthStencilAttachment;

        std::vector<ImagePassAttachment*>  m_imagePassAttachments;  // A list of all image pass attachment used by this pass.
        std::vector<BufferPassAttachment*> m_bufferPassAttachments; // A list of all buffer pass attachment used by this pass.
    };

} // namespace RHI