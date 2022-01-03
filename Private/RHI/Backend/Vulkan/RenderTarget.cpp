#include "RHI/Backend/Vulkan/RenderTarget.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"

namespace RHI
{
namespace Vulkan
{

    RenderTarget::~RenderTarget() {}

    VkResult RenderTarget::Init(const RenderTargetDesc& desc)
    {
        RenderPass*              pRenderPass;
        std::vector<VkImageView> attachments;

        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.pNext                   = nullptr;
        framebufferCreateInfo.flags                   = 0;
        framebufferCreateInfo.renderPass              = pRenderPass->GetHandle();
        framebufferCreateInfo.attachmentCount         = desc.colorAttachmentCount + (desc.pDepthAttachment ? 1 : 0);
        framebufferCreateInfo.pAttachments            = attachments.data();
        framebufferCreateInfo.width                   = desc.extent.sizeX;
        framebufferCreateInfo.height                  = desc.extent.sizeY;
        framebufferCreateInfo.layers                  = 1;

        return vkCreateFramebuffer(m_pDevice->GetHandle(), &framebufferCreateInfo, nullptr, &m_handle);
    }

} // namespace Vulkan
} // namespace RHI
