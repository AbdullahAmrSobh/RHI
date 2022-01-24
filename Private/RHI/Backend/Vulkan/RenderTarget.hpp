#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/Semaphore.hpp"
#include "RHI/RenderTarget.hpp"

namespace RHI
{
namespace Vulkan
{
    
    class RenderTarget final
        : public IRenderTarget
        , public DeviceObject<VkFramebuffer>
    {
    public:
        RenderTarget(Device& device)
            : DeviceObject(device)
			, m_renderIsFinishedSemaphore(device)
		{
        }
        ~RenderTarget();
        
        VkResult Init(const RenderTargetDesc& desc);
	
		inline VkSemaphore GetRenderIsFinishedSemaphore() const { return m_renderIsFinishedSemaphore.GetHandle(); }
	
	private:
		Semaphore m_renderIsFinishedSemaphore;
    
    };

} // namespace Vulkan
} // namespace RHI
