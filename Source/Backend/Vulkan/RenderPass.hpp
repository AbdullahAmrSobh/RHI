#pragma once

#include <vulkan/vulkan_core.h>
namespace RHI
{
namespace Vulkan
{
    class RenderPass
    {
    public:
        class Builder;
    
        struct AttachmentLoadStoreOps
        {
            VkAttachmentLoadOp loadOp;
            VkAttachmentLoadOp storeOp;
        };

        struct AttachmentDesc
        {
            VkFormat               format;
            VkSampleCountFlagBits  sampleCount;
            AttachmentLoadStoreOps loadStoreOps;
            VkImageLayout          srcLayout;
            VkImageLayout          dstLayout;
        };
    };

    class RenderPass::Builder
    {
    public:
        uint32_t BeginNewSubpass(VkPipelineBindPoint bindPoint);
        void     EndNewSubpass();

        void AddColorAttachment(const AttachmentDesc& desc);
        void AddInputAttachment(const AttachmentDesc& desc);
        void AddPreserveAttachment(const AttachmentDesc& desc);
        void SetDepthAttachment(const AttachmentDesc& desc);
        void SetResolveAttachment(const AttachmentDesc& desc);
    };

} // namespace Vulkan
} // namespace RHI