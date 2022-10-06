#include "Backend/Vulkan//RenderPass.hpp"
#include "Backend/Vulkan//Device.hpp"
#include "Backend/Vulkan/Common.hpp"

namespace RHI
{
namespace Vulkan
{

    Result<Unique<Framebuffer>> Framebuffer::Create(Device& device, VkExtent2D extent, const AttachmentsDesc& attachments, const RenderPass& renderPass)
    {
        Unique<Framebuffer> framebuffer = CreateUnique<Framebuffer>(device);
        VkResult            result      = framebuffer->Init(extent, attachments, renderPass);
        if (RHI_SUCCESS(result))
        {
            return std::move(framebuffer);
        }
        return ResultError(result);
    }

    VkResult Framebuffer::Init(VkExtent2D extent, const AttachmentsDesc& attachmentsDesc, const RenderPass& renderPass)
    {
        std::vector<VkImageView> attachments;

        for (uint32_t index = 0; index < attachmentsDesc.colorAttachmentsCount; index++)
        {
            const ImageView& attachment = attachmentsDesc.pColorAttachments[index];
            attachments.push_back(attachment.GetHandle());
        }

        if (attachmentsDesc.pDepthStencilAttachment != nullptr)
        {
            attachments.push_back(attachmentsDesc.pDepthStencilAttachment->GetHandle());
        }

        VkFramebufferCreateInfo createInfo{};
        createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext           = nullptr;
        createInfo.flags           = 0;
        createInfo.renderPass      = m_pRenderPass->GetHandle();
        createInfo.attachmentCount = CountElements(attachments);
        createInfo.pAttachments    = attachments.data();
        createInfo.width           = extent.width;
        createInfo.height          = extent.height;
        createInfo.layers          = 1;

        return vkCreateFramebuffer(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

} // namespace Vulkan
} // namespace RHI