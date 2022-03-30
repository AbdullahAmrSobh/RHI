#include "RHI/Backend/Vulkan/Queue.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"

namespace RHI {
namespace Vulkan {

        void Submit(ICommandContext& cmdCtx, IFence& signalFence) 
        {
            CommandContext& vkCmdCtx        = static_cast<CommandContext&>(cmdCtx);
            VkCommandBuffer handle          = vkCmdCtx.GetHandle();
            VkSubmitInfo    submitInfo      = {};
            submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.pNext                = nullptr;
            submitInfo.waitSemaphoreCount   = static_cast<uint32_t>(vkCmdCtx.m_waitSemaphores.size());
            submitInfo.pWaitSemaphores      = vkCmdCtx.m_waitSemaphores.data();
            submitInfo.pWaitDstStageMask    = vkCmdCtx.m_waitStages.data();
            submitInfo.commandBufferCount   = 1;
            submitInfo.pCommandBuffers      = &handle;
            submitInfo.signalSemaphoreCount = static_cast<uint32_t>(vkCmdCtx.m_signalSemaphores.size());
            submitInfo.pSignalSemaphores    = vkCmdCtx.m_signalSemaphores.data();
            Fence& fence                    = static_cast<Fence&>(signalFence);
            VkResult result = vkQueueSubmit(m_handle, 1, &submitInfo, fence.GetHandle());
			assert(result == VK_SUCCESS);
        }

        void Present(const SwapchainPresentDesc& desc)
		{
			
			std::vector<VkSwapchainKHR> swapchainHandles;
			std::vector<uint32_t> imageIndices;
			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.pNext = nullptr;
			presentInfo.waitSemaphoreCount;
			presentInfo.pWaitSemaphores;
			presentInfo.swapchainCount = static_cast<uint32_t>(swapchainHandles.size());
			presentInfo.pSwapchains = swapchainHandles.data();
			presentInfo.pImageIndices = imageIndices.data();
			presentInfo.pResults;
			
			VkResult result = vkQueuePresentKHR(m_handle, &presentInfo);
			assert(result == VK_SUCCESS);
		}
} // namespace Vulkan
} // namespace RHI
