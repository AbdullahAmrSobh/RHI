#pragma once
#include "RHI/ObjectCache.hpp"
#include "RHI/RenderPass.hpp"

#include "Backend/Vulkan/CommandQueue.hpp"
#include "Backend/Vulkan/Commands.hpp"
#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{
class Device;
class CommandAllocator;
class CommandBuffer;
class RenderPassLayout;
class Semaphore;

class RenderPass final : public IRenderPass
{
public:
    RenderPass(Device& device, std::string name)
        : IRenderPass(std::move(name))
        , m_device(&device)
    {
    }

    VkResult Init();

    std::span<WaitPoint> GetWaitPoints()
    {
        return m_waitPoints;
    }

    Semaphore& GetSignalSemaphore()
    {
        return *m_signalSemaphore;
    }

    const RenderPassLayout& GetLayout() const
    {
        return *m_layout;
    }

    Framebuffer& GetRenderTarget()
    {
        return *m_renderTargert;
    }

    CommandBuffer& GetCommandBuffer(size_t key);

    void Reset()
    {
        IRenderPass::Reset();
        m_layout = nullptr;
        m_renderTargert = nullptr;
        m_waitPoints.clear();
    }

private:
    friend class FrameScheduler;

    Device* m_device;

    Unique<CommandAllocator> m_commandsAllocator;

    ObjectCache<CommandBuffer> m_commandBuffers;

    const RenderPassLayout* m_layout;

    Shared<Framebuffer> m_renderTargert;

    Unique<Semaphore> m_signalSemaphore;

    std::vector<WaitPoint> m_waitPoints;

};

}  // namespace Vulkan
}  // namespace RHI