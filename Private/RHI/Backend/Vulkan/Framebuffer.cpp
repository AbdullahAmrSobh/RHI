#pragma once
#include "RHI/Backend/Vulkan/FrameBuffer.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"

namespace RHI
{
namespace Vulkan
{

    Expected<RenderTargetPtr> Factory::CreateRenderTarget(const RenderTargetDesc& desc)
    {
        auto renderTarget = CreateUnique<FrameBuffer>(desc);
        VkResult result = renderTarget->Init(desc);
        if (result != VK_SUCCESS)
            return Unexpected(ToResultCode(result));
        return renderTarget;
    }
    
    FrameBuffer::~FrameBuffer() { vkDestroyFramebuffer(m_pDevice->GetHandle(), m_handle, nullptr); }

    VkResult FrameBuffer::Init(const RenderTargetDesc& desc)
    {
        std::vector<VkImageView> attachments;

		for(auto& attachmentDesc : desc.attachments)
		{
			m_attachments.push_back(static_cast<TextureView*>(attachmentDesc.pView));
			attachments.push_back(m_attachments.back()->GetHandle());
		}
        
        VkFramebufferCreateInfo createInfo = {};
        createInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext                   = nullptr;
        createInfo.flags                   = 0;
        createInfo.renderPass              = desc.pRenderPass->GetHandle();
        createInfo.attachmentCount         = static_cast<uint32_t>(attachments.size());
        createInfo.pAttachments            = attachments.data();
        createInfo.width                   = desc.extent.sizeX;
        createInfo.height                  = desc.extent.sizeY;
        createInfo.layers                  = 0;

        return vkCreateFramebuffer(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    const ArrayView<const ITextureView*> GetAttachments()  {}

} // namespace Vulkan
} // namespace RHI
