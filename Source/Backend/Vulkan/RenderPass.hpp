#pragma once
#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
    
struct RenderTargetLayout;

namespace Vulkan
{
    class RenderPass final : public DeviceObject<VkRenderPass>
    {
    public:
        struct Desc
        {
            std::vector<VkAttachmentDescription> attachmentsDesc;
            std::vector<VkSubpassDescription>    subpassesDescs;
            std::vector<VkSubpassDependency>     dependencies;
        };
        
        static Result<Unique<RenderPass>> Create(Device& device, const Desc& desc);
    };

    class RenderPassManager 
    {
    public:
        struct SubpassDesc 
        {
            VkRenderPass renderPass; 
            uint32_t     subpassIndex; 
        };

        SubpassDesc& GetPass(const RenderTargetLayout& renderTargetLayout) const;

    private:
        mutable std::unordered_map<size_t, Unique<RenderPass>> m_renderPasses;        
    };

    class Framebuffer final : public DeviceObject<VkFramebuffer>
    {
    public:
        struct AttachmentsDesc
        {
            uint32_t         colorAttachmentsCount;
            const ImageView* pColorAttachments;
            const ImageView* pDepthStencilAttachment;
        };

        static Result<Unique<Framebuffer>> Create(Device& device, VkExtent2D extent, const AttachmentsDesc& attachments, const RenderPass& renderPass);

        VkResult Init(VkExtent2D extent, const AttachmentsDesc& attachmentsDesc, const RenderPass& renderPass);

        inline const RenderPass& GetRenderPass() const
        {
            return *m_pRenderPass;
        }

    private:
        const RenderPass* m_pRenderPass;
    };

} // namespace Vulkan
} // namespace RHI