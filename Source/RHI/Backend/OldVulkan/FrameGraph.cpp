#pragma once
#include "RHI/FrameGraph.hpp"
#include "RHI/Definitions.hpp"

#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/FrameGraph.hpp"

#include "RHI/Backend/Vulkan/RenderPass.hpp"

#include "RHI/Backend/Vulkan/Fence.hpp"
#include "RHI/Backend/Vulkan/Queue.hpp"

#include "RHI/Backend/Vulkan/Buffer.hpp"
#include "RHI/Backend/Vulkan/Image.hpp"

namespace RHI
{
namespace Vulkan
{

    void FrameGraph::CreateRenderPass(std::string name, ERenderPassQueueType queueType, RenderPassExecuter& executer) {}

    BufferAttachmentId FrameGraph::CreateBufferAttachment(const BufferAttachmentDesc& desc)
    {
        Buffer* bufferAttachmentResource = new Buffer(*m_pDevice);
        
        MemoryAllocationDesc allocationDesc = {};
		BufferDesc           bufferDesc     = {};

        VkResult result = bufferAttachmentResource->Init(allocationDesc, bufferDesc);
			return CreateAttachmentResource(bufferAttachmentResource);
    }
    
    ImageAttachmentId FrameGraph::CreateImageAttachment(const ImageAttachmentDesc& desc)
    {
		Image* imageAttachmentResource = new Image(*m_pDevice);
		
		MemoryAllocationDesc allocationDesc = {};
		ImageDesc           imageDesc       = {};
		
		VkResult result = imageAttachmentResource->Init(allocationDesc, imageDesc);
		return CreateAttachmentResource(imageAttachmentResource);
    }
    
    BufferAttachmentId FrameGraph::ImportBufferAsAttachment(const BufferAttachmentDesc& desc, IBuffer& buffer)
    {
        BufferAttachmentId id;
        return id;
    }
    
    ImageAttachmentId FrameGraph::ImportImageAsAttachment(const ImageAttachmentDesc& desc, IImage& image)
    {
        ImageAttachmentId id;
        return id;
    }

    ImageAttachmentId FrameGraph::ImportSwapchainAsAttachment(const ImageAttachmentDesc& desc, const ISwapChain& swapchain)
    {
        ImageAttachmentId id;
        return id;
    }

    void FrameGraph::Render() {}
	
}; // namespace Vulkan
} // namespace RHI
