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

VkAttachmentLoadOp ConvertAttachmentLoadOperation(AttachmentLoadOperation operation)
{
    return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
}

VkAttachmentStoreOp ConvertAttachmentStoreOperation(AttachmentStoreOperation operation)
{
    return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
}

VkImageLayout GetInitialImageLayout(const UsedImageAttachment& attachment)
{
    if(const UsedImageAttachment* previous  = attachment.GetPreviousUse())
    {
        return VK_IMAGE_LAYOUT_GENERAL;
    }
    
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkImageLayout GetFinalImageLayout(const UsedImageAttachment& attachment)
{
    if(attachment.GetAttachment().IsSwapchainImage() && !attachment.GetNextUse())
    {
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    return VK_IMAGE_LAYOUT_GENERAL;
}

VkImageLayout GetOptiomalImageLayout(const UsedImageAttachment& attachment)
{
    return VK_IMAGE_LAYOUT_GENERAL;
}


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