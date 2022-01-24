#include "RHI/Backend/Vulkan/RenderTarget.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"
#include "RHI/Backend/Vulkan/RenderPass.hpp"
#include "RHI/Backend/Vulkan/Texture.hpp"

namespace RHI
{
namespace Vulkan
{

    Expected<RenderTargetPtr> Factory::CreateRenderTarget(const RenderTargetDesc& desc)
    {
        auto     renderTarget = CreateUnique<RenderTarget>(*m_device);
        VkResult result       = renderTarget->Init(desc);
        if (result != VK_SUCCESS)
            return Unexpected(ToResultCode(result));

        return renderTarget;
    }
    
    RenderTarget::~RenderTarget() { vkDestroyFramebuffer(m_pDevice->GetHandle(), m_handle, nullptr); }

    VkResult RenderTarget::Init(const RenderTargetDesc& desc)
    {
        RenderPass renderPass = RenderPass::FindOrCreate(desc);

        std::vector<VkImageView> attachmentHandles(desc.attachments.size() + (desc.hasDepthStencil ? 1 : 0));

        std::transform(desc.attachments.begin(), desc.attachments.end(), attachmentHandles.begin(),
                       [](auto& view) -> VkImageView { return static_cast<TextureView*>(view.pAttachmentView)->GetHandle(); });

        if (desc.hasDepthStencil)
            attachmentHandles.push_back(static_cast<TextureView*>(desc.depthAttachment.pAttachmentView)->GetHandle());

        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.pNext                   = nullptr;
        framebufferCreateInfo.flags                   = 0;
        framebufferCreateInfo.renderPass              = renderPass.GetHandle();
        framebufferCreateInfo.attachmentCount         = static_cast<uint32_t>(attachmentHandles.size());
        framebufferCreateInfo.pAttachments            = attachmentHandles.data();
        framebufferCreateInfo.width                   = desc.extent.sizeX;
        framebufferCreateInfo.height                  = desc.extent.sizeY;
        framebufferCreateInfo.layers                  = 1;

        return vkCreateFramebuffer(m_pDevice->GetHandle(), &framebufferCreateInfo, nullptr, &m_handle);
    }

} // namespace Vulkan
} // namespace RHI
