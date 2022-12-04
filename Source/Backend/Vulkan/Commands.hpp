#pragma once
#include "Backend/Vulkan/Resource.hpp"
#include "RHI/Commands.hpp"

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
    CommandBuffer(const Device&     device,
                  CommandAllocator* pParantAllocator,
                  VkCommandBuffer   handle)
        : DeviceObject(&device, handle)
        , m_pParantAllocator(pParantAllocator)
    {
    }

    ~CommandBuffer();

    ///////////////////////////////////////////////////////////////////////////////
    // Interface.
    ///////////////////////////////////////////////////////////////////////////////
    void Begin() override;
    void End() override;

    void SetViewports(const std::vector<Viewport>& viewports) override;
    void SetScissors(const std::vector<Rect>& scissors) override;

    void Submit(const DrawCommand& drawCommand) override;
    ///////////////////////////////////////////////////////////////////////////////

private:
    CommandAllocator*  m_pParantAllocator;
};

enum class ECommandPrimaryTask
{
    Graphics,
    Compute,
    Transfer,
};

class CommandAllocator final : public DeviceObject<VkCommandPool>
{
public:
    CommandAllocator(const Device& device)
        : DeviceObject(&device)
    {
    }

    ~CommandAllocator();

    VkResult Init(ECommandPrimaryTask task);

    VkResult Reset();

    Unique<CommandBuffer> AllocateCommandBuffer();

    std::vector<CommandBuffer> AllocateCommandBuffers(uint32_t count);
};

}  // namespace Vulkan
}  // namespace RHI