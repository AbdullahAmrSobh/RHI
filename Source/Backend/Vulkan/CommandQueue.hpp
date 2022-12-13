#pragma once
#include "Backend/Vulkan/Common.hpp"

namespace RHI
{
namespace Vulkan
{

class Semaphore;
class CommandBuffer;
class Swapchain;

struct WaitPoint
{
    Semaphore*           semaphore;
    VkPipelineStageFlags stage;
};

struct SubmitInfo
{
    std::vector<WaitPoint>      waitPoints;
    std::vector<CommandBuffer*> commandBuffers;
    std::vector<Semaphore*>     signalSemaphores;
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

    void Present(std::span<const Semaphore*> waitSemaphores,
                     const Swapchain&            swapchain);

    const uint32_t m_familyIndex;

private:
    VkQueue m_handle;
};

}  // namespace Vulkan
}  // namespace RHI