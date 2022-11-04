#pragma once
#include "RHI/FrameGraphPass.hpp"

#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/FrameGraphPass.hpp"
#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{

namespace Vulkan
{
    
    class Pass;
    
    class RenderPass final : public DeviceObject<VkRenderPass>
    {
    public:
        static Result<Unique<RenderPass>> Create(const Device& device, const Pass& pass);
        
        RenderPass(const Device& device)
            : DeviceObject(&device)
        {
        }
        
        ~RenderPass();
        
        VkResult Init(const Pass& pass);
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
        
        static Result<Unique<Framebuffer>> Create(const Device& device, VkExtent2D extent, const AttachmentsDesc& attachments, const RenderPass& renderPass);

        Framebuffer(const Device& device)
            : DeviceObject(&device)
        {
        }
        ~Framebuffer();

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