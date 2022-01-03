#pragma once
#include "RHI/RenderTarget.hpp"
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/RenderPass.hpp"

namespace RHI {
namespace Vulkan {

	class RenderTarget final : public IRenderTarget, public DeviceObject<VkFramebuffer>
	{
	public:
		RenderTarget(Device& device)
			: DeviceObject(device)
		{
		}
		
		~RenderTarget();
		
		VkResult Init(const RenderTargetDesc& desc);
	}; 

}
}
