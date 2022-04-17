#include "RHI/Backend/Vulkan/RenderPass.hpp"
#include "RHI/Backend/Vulkan/Utils.hpp"

#include "RHI/Backend/Vulkan/CommandList.hpp"
#include "RHI/Backend/Vulkan/Queue.hpp"

namespace RHI
{
namespace Vulkan
{

    RenderTargetLayout RenderPass::GetRenderTargetLayout() const
    {
        RenderTargetLayout layout;

        for (auto& usedImage : m_usedImageResources)
            layout.colorFormats.push_back(usedImage.format);

        if (m_isDepthStencilEnabled)
            layout.depthStencilFormat = m_usedDepthStencilAttachment.format;

        return layout;
    }

    void RenderPass::Invalidate()
    {
        m_isDepthStencilEnabled = false;
        m_isSwapChainWriter     = false;
        m_usedImageResources.clear();
        m_usedBufferResources.clear();
    }

    void RenderPass::UseSwapChainAttachment(SwapChainAttachmentId id, EAttachmentUsage usage, EAttachmentAccess access) 
	{
		m_isSwapChainWriter = true;
	}
    
    void RenderPass::UseImageAttachments(Span<ImageAttachmentId> ids, EAttachmentUsage usage, EAttachmentAccess access) 
	{
		if (usage == EAttachmentUsage::DepthStencil)
		{
			m_isDepthStencilEnabled = true;
		}
	}
    
    void RenderPass::UseBufferAttachments(Span<BufferAttachmentId> ids, EAttachmentUsage usage, EAttachmentAccess access) 
	{
	}
    
    SubmitInfo RenderPass::CompileSubmitInfo()
    {
        SubmitInfo submitInfo;
        
        for (auto& commandList : m_executeContext.GetCommandLists())
            submitInfo.commandBuffers.push_back(static_cast<CommandList*>(commandList)->GetHandle());
        
        submitInfo.signalSemaphores.push_back(m_renderPassFinishedSemaphore->GetHandle());
        
		// Images
        for (auto& usedResource : m_usedImageResources)
        {
            if (usedResource.access == EAttachmentAccess::Write && usedResource.isFirstUse)
            {
                submitInfo.waitSemaphores.push_back(usedResource.writeFinishedSemaphore->GetHandle());
                submitInfo.waitStages.push_back(usedResource.accessStage);
            }
        }
        
		// Buffers
        for (auto& usedResource : m_usedBufferResources)
        {
            if (usedResource.access == EAttachmentAccess::Write && usedResource.isFirstUse)
            {
                submitInfo.waitSemaphores.push_back(usedResource.writeFinishedSemaphore->GetHandle());
                submitInfo.waitStages.push_back(usedResource.accessStage);
            }
        }

        if (m_isDepthStencilEnabled && m_usedDepthStencilAttachment.access == EAttachmentAccess::Write && m_usedDepthStencilAttachment.isFirstUse)
        {
            submitInfo.waitSemaphores.push_back(m_usedDepthStencilAttachment.writeFinishedSemaphore->GetHandle());
            submitInfo.waitStages.push_back(m_usedDepthStencilAttachment.accessStage);
        }

        if (m_isSwapChainWriter)
        {
            submitInfo.waitSemaphores.push_back(m_usedSwapChainAttachment.writeFinishedSemaphore->GetHandle());
            submitInfo.waitStages.push_back(m_usedSwapChainAttachment.accessStage);
        }

        return submitInfo;
    }

	PresentInfo RenderPass::CompilePresentInfo()
	{
		PresentInfo presentInfo;
		return presentInfo;
	}
		
	void RenderPass::BuildResourceBarriers() 
	{

	}


} // namespace Vulkan
} // namespace RHI
