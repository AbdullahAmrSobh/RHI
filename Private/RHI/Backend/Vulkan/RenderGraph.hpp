#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/RenderPass.hpp"
#include "RHI/Backend/Vulkan/RenderPassBuilder.hpp"
#include "RHI/RenderGraph.hpp"

namespace RHI
{
namespace Vulkan
{
    
    class RenderGraph
        : public IRenderGraph
        , public DeviceObject<void>
    {
    public:
        RenderGraph(Device& device)
            : DeviceObject(device)
        {
        }
        ~RenderGraph();
        
        VkResult Init(const RenderGraphBuilder& builder);
        
        virtual uint32_t GetFrameIndex() const override;
        virtual uint32_t GetFrameCount() const override;
        
        virtual ICommandContext& GetPassCommandContext(const PassId id) override;
        
        virtual void BeginFrame() override;
        virtual void EndFrame() override;
    
	private:
		void BuildPass(const Pass&);
		
		void CreateFramebuffer(const Pass& pass);
		void CreateRenderPass(const Pass& pass);
    
	
    };

} // namespace Vulkan
} // namespace RHI
