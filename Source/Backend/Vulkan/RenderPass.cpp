#include "RHI/Pch.hpp"

#include "Backend/Vulkan/Common.hpp"

#include "Backend/Vulkan/RenderPass.hpp"

#include "Backend/Vulkan/Commands.hpp"
#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/Framebuffer.hpp"

namespace RHI
{
namespace Vulkan
{

VkResult RenderPass::Init()
{
    m_commandsAllocator = CreateUnique<CommandAllocator>(*m_device);
    m_signalSemaphore   = Semaphore::Create(*m_device);
    return m_commandsAllocator->Init();
}

CommandBuffer& RenderPass::GetCommandBuffer(size_t key)
{
    Shared<CommandBuffer> commandBuffer = m_commandBuffers.Find(key);
    if (commandBuffer)
    {
        return *commandBuffer;
    }
    commandBuffer = m_commandsAllocator->AllocateCommandBuffer();
    m_commandBuffers.Insert(key, commandBuffer);

    return *commandBuffer;
}

}  // namespace Vulkan
}  // namespace RHI