#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/Texture.hpp"
namespace RHI {
	
	
	enum class ERenderPassTextureAttachmentUsageFlagBits { };
	
	class RenderPassAttachment {};
	class RenderPassTextureAttachment {};
	class RenderPassBufferAttachment {};
	
	class RenderPassDescriptor
	{
	public:
	
		void UseAttachment(const RenderPassTextureAttachment& );
		void UseAttachment(const RenderPassBufferAttachment& );

		void ProduceAttachment(const RenderPassTextureAttachment& );
		void ProduceAttachment(const RenderPassBufferAttachment& );
	
	};

	class RenderPass;
	class RenderPassDependency;
	
	class IRenderGraph{};
	class IRenderGraphBuilder;
	class IRenderGraphExecutor; 

} // namespace RHI
