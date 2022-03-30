#include "RHI/Backend/Vulkan/RenderGraph.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"

namespace RHI
{
namespace Vulkan
{

    Expected<RenderGraphPtr> Factory::CreateRenderGraph(const RenderGraphBuilder& builder)
    {
        auto     renderGraph = CreateUnique<RenderGraph>(*m_device);
        VkResult result      = renderGraph->Init(builder);
        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));

        return renderGraph;
    }

    VkResult RenderGraph::Init(const RenderGraphBuilder& builder)
    {
        for (auto& pass : builder.GetPasses())
        {
            BuildPass(pass);
        }
    }

    void RenderGraph::BuildPass(const Pass& pass)
    {
        RenderPassBuilder renderPassBuilder;

        VkPipelineBindPoint bindpoint = VK_PIPELINE_BIND_POINT_MAX_ENUM;
        switch (pass.GetTaskClass())
        {
        case EPassTaskClass::Present:
        case EPassTaskClass::Graphics: bindpoint = VK_PIPELINE_BIND_POINT_GRAPHICS; break;
        case EPassTaskClass::Compute: bindpoint = VK_PIPELINE_BIND_POINT_COMPUTE; break;
        default: break;
        };
        
        uint32_t subpassIndex = renderPassBuilder.BeginSubpass(bindpoint);
		
		for(auto& attachment : pass.GetTextureAttachments())
		{
			VkAttachmentDescription desc;
			desc.flags;
			desc.format;
			desc.samples;
			desc.loadOp;
			desc.storeOp;
			desc.stencilLoadOp;
			desc.stencilStoreOp;
			desc.initialLayout;
			desc.finalLayout;
			
			renderPassBuilder.UseColorAttachment(desc, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}
		
		for(auto& attachment : pass.GetBufferAttachments())
		{
		
		}
    }

} // namespace Vulkan
} // namespace RHI
