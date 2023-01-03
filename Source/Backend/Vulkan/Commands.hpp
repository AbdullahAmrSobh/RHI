#pragma once
#include "RHI/CommandBuffer.hpp"

#include "Backend/Vulkan/DeviceObject.hpp"

namespace RHI
{
namespace Vulkan
{

class Framebuffer;
class Semaphore;
class ShaderResourceGroup;
class CommandAllocator;

class CommandBuffer final
    : public ICommandBuffer
    , public DeviceObject<VkCommandBuffer>
{
public:
    CommandBuffer(Device& device, CommandAllocator* pParantAllocator, VkCommandBuffer handle)
        : DeviceObject(device, handle)
        , m_pParantAllocator(pParantAllocator)
    {
    }

    ~CommandBuffer();

    void Begin();
    void End();

    void BeginRenderPass(Framebuffer& framebuffer);
    void EndRenderPass();

    void SetViewports(std::span<const Viewport> viewports) override;
    void SetScissors(std::span<const Rect> scissors) override;

    void Submit(const DrawCommand& drawCommand) override;

private:
    CommandAllocator* m_pParantAllocator;
};

class CommandAllocator final : public DeviceObject<VkCommandPool>
{
public:
    CommandAllocator(Device& device)
        : DeviceObject(device)
    {
    }

    ~CommandAllocator();

    VkResult Init();

    VkResult Reset();

    Unique<CommandBuffer> AllocateCommandBuffer();

    std::vector<CommandBuffer> AllocateCommandBuffers(uint32_t count);
};

}  // namespace Vulkan
}  // namespace RHI