#pragma once
#include "Backend/Vulkan/Swapchain.hpp"

namespace RHI
{
namespace Vulkan
{

class CommandBuffer;

struct WaitPoint
{
    Semaphore*           semaphore;
    VkPipelineStageFlags stage;
};

struct SubmitInfo
{
    std::vector<WaitPoint>      waitPoints;
    std::vector<Semaphore*>     signalSemaphores;
    std::vector<CommandBuffer*> commandBuffers;
};

class CommandQueue
{
public:
    CommandQueue(VkQueue queue, uint32_t familyIndex)
        : m_familyIndex(familyIndex)
        , m_handle(queue)
    {
    }

    void WaitIdle();

    void Submit(const SubmitInfo& submitInfo, IFence* signalFence = nullptr);

    void Present(std::span<const Semaphore* const> waitSemaphores, Swapchain& swapchain);

    const uint32_t m_familyIndex;

private:
    VkQueue m_handle;
};

}  // namespace Vulkan
}  // namespace RHI