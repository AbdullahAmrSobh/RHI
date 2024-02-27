#pragma once
#include "RHI/Export.hpp"
#include "RHI/Attachments.hpp"
#include "RHI/Resources.hpp"

#include <string>

namespace RHI
{
    class CommandList;

    // FIXME: fwd decl
    namespace Vulkan
    {
        class IFrameScheduler;
        class ICommandList;
    } // namespace Vulkan

    enum class QueueType
    {
        Graphics,
        Compute,
        Transfer,
    };

    class RHI_EXPORT Pass
    {
    public:
        Pass(const char* name, QueueType type);
        ~Pass();

        struct TransientRenderTargetInfo
        {
            Format              format; // usage deduced from format
            ClearValue          clearValue;
            LoadStoreOperations loadStoreOperations;
            uint32_t            mipLevelsCount;
            uint32_t            arrayLayerCount;
        };

        struct TransientShaderResource;
        struct TransientCopyResource;

        ImagePassAttachment*  CreateTransientImage(const char* name, TransientRenderTargetInfo info);
        ImagePassAttachment*  UseImage(ImagePassAttachment* attachment);

        ImagePassAttachment*  ImportSwapchainImage(const char* name, Swapchain* swapchain);
        ImagePassAttachment*  ImportImage(const char* name, Handle<Image> handle);
        BufferPassAttachment* ImportBuffer(const char* name, Handle<Buffer> handle);
        ImagePassAttachment*  CreateTransientImage(const char* name, const ImageCreateInfo& createInfo);
        BufferPassAttachment* CreateTransientBuffer(const char* name, const BufferCreateInfo& createInfo);
        ImagePassAttachment*  UseColorAttachment(ImageAttachment* attachment, ColorValue value, LoadStoreOperations loadStoreOperations = LoadStoreOperations{});
        ImagePassAttachment*  UseDepthAttachment(ImageAttachment* attachment, DepthStencilValue value, LoadStoreOperations loadStoreOperations = LoadStoreOperations{});
        ImagePassAttachment*  UseStencilAttachment(ImageAttachment* attachment, DepthStencilValue value, LoadStoreOperations loadStoreOperations = LoadStoreOperations{});
        ImagePassAttachment*  UseDepthStencilAttachment(ImageAttachment* attachment, DepthStencilValue value, LoadStoreOperations loadStoreOperations = LoadStoreOperations{});
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
        friend class Vulkan::IFrameScheduler;
        friend class Vulkan::ICommandList;

        std::string                        m_name;
        QueueType                          m_queueType;    // The type of the Hardware Queue needed to execute this pass.
        ImageSize2D                        m_size;         // The size of the rendering area in the render pass.
        std::vector<Pass*>                 m_producers;    // A list of all passes that this pass will wait for.
        std::vector<CommandList*>          m_commandLists; // A list of command lists that executes this pass.

        RHI::SwapchainImagePassAttachment* m_swapchainImageAttachment;
        // RHI::ImagePassAttachment*                   m_depthStencilAttachment;

        std::vector<ImagePassAttachment*>  m_imagePassAttachments;  // A list of all image pass attachment used by this pass.
        std::vector<BufferPassAttachment*> m_bufferPassAttachments; // A list of all buffer pass attachment used by this pass.
    };

    inline ImagePassAttachment* Pass::UseColorAttachment(ImageAttachment* attachment, ColorValue value, LoadStoreOperations loadStoreOperations)
    {
        return UseColorAttachment(attachment, ImageViewCreateInfo{ ImageAspect::Color }, value, loadStoreOperations);
    }

    inline ImagePassAttachment* Pass::UseDepthAttachment(ImageAttachment* attachment, DepthStencilValue value, LoadStoreOperations loadStoreOperations)
    {
        return UseDepthAttachment(attachment, ImageViewCreateInfo{ ImageAspect::Depth }, value, loadStoreOperations);
    }

    inline ImagePassAttachment* Pass::UseStencilAttachment(ImageAttachment* attachment, DepthStencilValue value, LoadStoreOperations loadStoreOperations)
    {
        return UseStencilAttachment(attachment, ImageViewCreateInfo{ ImageAspect::Stencil }, value, loadStoreOperations);
    }

    inline ImagePassAttachment* Pass::UseDepthStencilAttachment(ImageAttachment* attachment, DepthStencilValue value, LoadStoreOperations loadStoreOperations)
    {
        return UseDepthStencilAttachment(attachment, ImageViewCreateInfo{ ImageAspect::DepthStencil }, value, loadStoreOperations);
    }

} // namespace RHI