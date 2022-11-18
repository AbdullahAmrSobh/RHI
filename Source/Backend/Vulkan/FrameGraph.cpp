#include "Backend/Vulkan/FrameGraph.hpp"
#include <vector>
#include "RHI/Common.hpp"
#include "RHI/FrameGraphAttachment.hpp"
#include "Backend/Vulkan/Commands.hpp"
#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/FrameGraphPass.hpp"
#include <vulkan/vulkan_core.h>

namespace RHI
{
namespace Vulkan
{
    Expected<Unique<IFrameGraph>> Device::CreateFrameGraph()
    {
        Unique<FrameGraph> frameGraph = CreateUnique<FrameGraph>(*this);

        return std::move(frameGraph);
    }

    EResultCode FrameGraph::SubmitPass(IPassProducer& producer)
    {
        const Pass&  pass  = static_cast<const Pass&>(producer.GetPass());
        const Queue& queue = m_pDevice->GetGraphicsQueue();

        Queue::SubmitRequest submitRequest{};

        VkCommandBufferSubmitInfo submitInfo{};
        submitInfo.commandBuffer = static_cast<const CommandBuffer*>(pass.GetCurrentCommandBuffer())->GetHandle();
        submitInfo.sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        submitInfo.pNext         = nullptr;
        submitInfo.deviceMask    = 0;

        submitRequest.commandBuffers.push_back(submitInfo);
        
        VkSemaphoreSubmitInfo signalSemaphore = {};
        signalSemaphore.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
        signalSemaphore.pNext                 = nullptr;
        signalSemaphore.semaphore             = pass.GetPassFinishedSemaphore().GetHandle();
        signalSemaphore.stageMask             = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
        signalSemaphore.deviceIndex           = 0;
        signalSemaphore.value                 = 0;

        submitRequest.signalSemaphores.push_back(signalSemaphore);

        for (auto passAttachment : pass.GetImageAttachments())
        {
            // if used in a different pass, wait for it.

            auto prev = passAttachment->GetPerv();

            if (prev)
            {
                // Wait for pass's semapho
                re
            }
        }

        VkResult result = queue.Submit(std::vector<Queue::SubmitRequest>{submitRequest}, static_cast<const Fence&>(pass.GetSignalFence()));

        if (pass.HasSwapchainTarget())
        {
            Queue::PresentRequest presentRequest{};
            presentRequest.swapchains;
            presentRequest.waitSemaphores;
            result = queue.Present(presentRequest);
        }

        return EResultCode::Success;
    }

} // namespace Vulkan
} // namespace RHI