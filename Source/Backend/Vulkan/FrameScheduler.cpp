#include "RHI/Pch.hpp"

#include "Backend/Vulkan/Common.hpp"

#include "Backend/Vulkan/FrameScheduler.hpp"

#include "Backend/Vulkan/CommandQueue.hpp"
#include "Backend/Vulkan/Commands.hpp"
#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/Framebuffer.hpp"
#include "Backend/Vulkan/RenderPass.hpp"
#include "Backend/Vulkan/Swapchain.hpp"

namespace RHI
{
namespace Vulkan
{

static std::optional<WaitPoint> GetImageAttachmentWaitPoint(UsedImageAttachment& attachment)
{
    if (ISwapchain* swapchain = attachment.GetAttachment().GetSwapchain())
    {
        Semaphore* semaphore = &static_cast<Swapchain*>(swapchain)->GetImageReadySemaphore();
        return WaitPoint(semaphore, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    }

    return std::nullopt;
}

Expected<std::unique_ptr<IFrameScheduler>> Device::CreateFrameScheduler()
{
    std::unique_ptr<FrameScheduler> scheduler = std::make_unique<FrameScheduler>(*this);
    VkResult               result    = scheduler->Init();
    if (result != VK_SUCCESS)
    {
        return Unexpected(ConvertResult(result));
    }
    return scheduler;
}

FrameScheduler::~FrameScheduler()
{
    m_device->GetGraphicsQueue().WaitIdle();
}

VkResult FrameScheduler::Init()
{
    return VK_SUCCESS;
}

std::unique_ptr<IRenderPass> FrameScheduler::CreateRenderPass(std::string name) const
{
    auto renderpass = std::make_unique<RenderPass>(*m_device, std::move(name));
    Utils::AssertSuccess(renderpass->Init());
    return renderpass;
}

ICommandBuffer& FrameScheduler::BeginCommandBuffer(IRenderPass& _renderpass)
{
    RenderPass& renderpass = static_cast<RenderPass&>(_renderpass);

    std::shared_ptr<Framebuffer> framebuffer = m_device->CreateCachedFramebuffer(renderpass.GetUsedAttachments());
    renderpass.m_layout             = &framebuffer->GetLayout();
    renderpass.m_renderTargert      = framebuffer;

    m_currentCommandBuffer = &renderpass.GetCommandBuffer(framebuffer->GetHash());
    m_currentCommandBuffer->Begin();
    m_currentCommandBuffer->BeginRenderPass(*framebuffer);
    return *m_currentCommandBuffer;
}

void FrameScheduler::EndCommandBuffer()
{
    m_currentCommandBuffer->EndRenderPass();
    m_currentCommandBuffer->End();
    m_currentCommandBuffer = nullptr;
}

void FrameScheduler::Submit(IRenderPass& _renderpass)
{
    RenderPass& renderpass = static_cast<RenderPass&>(_renderpass);

    CommandQueue& queue = m_device->GetGraphicsQueue();
    SubmitInfo    submitInfo;
    auto          waitPoints = renderpass.GetWaitPoints();
    submitInfo.waitPoints.assign(waitPoints.begin(), waitPoints.end());

    if (UsedImageAttachment* attachment = renderpass.GetUsedSwapchainAttachment())
    {
        submitInfo.waitPoints.push_back(GetImageAttachmentWaitPoint(*attachment).value());
    }

    submitInfo.commandBuffers.push_back(static_cast<CommandBuffer*>(&renderpass.GetCommandBuffer(renderpass.GetRenderTarget().GetHash())));
    submitInfo.signalSemaphores.push_back(&renderpass.GetSignalSemaphore());

    if (Swapchain* swapchain = static_cast<Swapchain*>(renderpass.GetSwapchain()))
    {
        Fence& fence = swapchain->GetCurrentFence();
        fence.Reset();
        queue.Submit(submitInfo, &fence);
        queue.Present(submitInfo.signalSemaphores, *swapchain);
        assert(fence.Wait() == ResultCode::Success);
    }
    else
    {
        queue.Submit(submitInfo);
    }

    renderpass.Reset();
}

}  // namespace Vulkan
}  // namespace RHI