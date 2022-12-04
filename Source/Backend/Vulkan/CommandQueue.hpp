#pragma once
#include "Backend/Vulkan/Common.hpp"

namespace RHI
{

class Semaphore; 
class CommandBuffer; 
class Swapchain;

struct WaitPoint
{
    Semaphore*            semaphore;
    VkShaderStageFlagBits stage;
};

struct SubmitRequest
{
    std::vector<WaitPoint>      waitPoints;
    std::vector<CommandBuffer*> commandBuffers;
    std::vector<Semaphore*>     signalSemaphores;
};

class CommandQueue
{
public:
    CommandQueue(VkQueue queue)
        : m_handle(queue)
    {
    }

    VkResult WaitIdle(uint32_t timeoutMiliseconds);

    VkResult Submit(std::span<const SubmitRequest*> submitList,
                    IFence*                           signalFence = nullptr);

    VkResult Present(std::span<const WaitPoint*>,
                     std::vector<const Swapchain*> swapchains);

private:
    VkQueue m_handle; 
};

}  // namespace RHI