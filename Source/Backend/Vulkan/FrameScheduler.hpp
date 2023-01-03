#pragma once
#include "RHI/FrameScheduler.hpp"
#include "RHI/ResourceCachedAllocator.hpp"

#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/DeviceObject.hpp"
#include "Backend/Vulkan/Framebuffer.hpp"

namespace RHI
{
namespace Vulkan
{

class CommandBuffer;

class FrameScheduler final : public IFrameScheduler
{
public:
    FrameScheduler(Device& device)
        : IFrameScheduler(static_cast<IDevice&>(device))
        , m_device(&device)
    {
    }

    ~FrameScheduler();

    VkResult Init();

    Unique<IRenderPass> CreateRenderPass(std::string name) const override;
    ICommandBuffer&     BeginCommandBuffer(IRenderPass& renderpass) override;
    void                EndCommandBuffer() override;
    void                Submit(IRenderPass& renderpass) override;

private:
    Device* m_device;

    CommandBuffer* m_currentCommandBuffer = nullptr;
};

}  // namespace Vulkan
}  // namespace RHI