#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Queue.hpp"

#include "RHI/Backend/Vulkan/CommandList.hpp"
#include "RHI/Backend/Vulkan/Fence.hpp"

namespace RHI
{
namespace Vulkan
{

    class Queue final
        : public IQueue
        , public DeviceObject<VkQueue>
    {
    public:
        ~Queue() = default;
        
        void Init(Device& device, uint32_t queueFamilyIndex, uint32_t queueIndex)
		{
			vkGetDeviceQueue(m_pDevice->GetHandle(), queueFamilyIndex, queueIndex, &m_handle);
		}
        
        virtual void Submit(ICommandContext& cmdCtx, IFence& signalFence) override;
        virtual void Present(const SwapchainPresentDesc& desc) override;
    };
} // namespace Vulkan
} // namespace RHI
