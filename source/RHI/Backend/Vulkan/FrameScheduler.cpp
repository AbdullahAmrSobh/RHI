#pragma once

#include "RHI/Backend/Vulkan/FrameScheduler.hpp"

#include "RHI/FrameGraphAttachments.hpp"
#include "RHI/Pass.hpp"
#include "RHI/PassInterface.hpp"

#include "RHI/Backend/Vulkan/Command.hpp"
#include "RHI/Backend/Vulkan/Context.hpp"
#include "RHI/Backend/Vulkan/Conversion.inl"
#include "RHI/Backend/Vulkan/Pass.hpp"
#include "RHI/Backend/Vulkan/Resources.hpp"

namespace Vulkan
{

RHI::CommandList& FrameScheduler::PassExecuteBegin(RHI::Pass& _pass)
{
    auto& context = static_cast<Context&>(*m_context);
    auto& pass    = static_cast<Pass&>(_pass);
    auto  device  = context.GetDevice();

    vk::CommandBufferAllocateInfo allocateInfo {};
    allocateInfo.setCommandPool(m_commandPool);
    allocateInfo.setCommandBufferCount(1);
    allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);

    vk::CommandBuffer handle = device.allocateCommandBuffers(allocateInfo).value.front();
    pass.m_commandList       = std::make_unique<CommandList>(handle);

    allocateInfo.setCommandBufferCount(10);
    allocateInfo.setLevel(vk::CommandBufferLevel::eSecondary);
    auto commandBuffers = device.allocateCommandBuffers(allocateInfo).value;
    return *pass.m_commandList;
}

void FrameScheduler::PassExecuteEnd(RHI::Pass& _pass)
{
    auto& context             = static_cast<Context&>(*m_context);
    auto& pass                = static_cast<Pass&>(_pass);
    auto  commandList         = static_cast<CommandList&>(pass.GetCommandList());
    auto  device              = context.GetDevice();
    auto  commandBufferHandle = commandList.GetHandle();

    vk::CommandBufferSubmitInfo commandInfo {};
    commandInfo.setCommandBuffer(commandBufferHandle);

    vk::SemaphoreSubmitInfo semaphoreInfo {};

    vk::SubmitInfo2 submitInfo {};
    submitInfo.setCommandBufferInfos(commandInfo);
    submitInfo.setWaitSemaphoreInfos(semaphoreInfo);

    // If this pass is the last user, of a swapchain image, then submit a present request, after execution.
    auto passAttachment = pass.GetSwapchainAttachment();
    if (passAttachment != nullptr && passAttachment->nextUse == nullptr)
    {
        std::span<vk::Semaphore> waitSemaphores;
        auto& swapchain  = *static_cast<Swapchain*>(passAttachment->attachment->swapchain);
        auto  imageIndex = passAttachment->attachment->swapchain->GetCurrentImageIndex();
        auto  result     = vk::Result::eErrorUnknown;

        auto swapchianHandle = swapchain.GetHandle();

        vk::PresentInfoKHR presentInfo {};
        presentInfo.setWaitSemaphores(waitSemaphores);
        presentInfo.setSwapchainCount(1);
        presentInfo.setPSwapchains(&swapchianHandle);
        presentInfo.setImageIndices(imageIndex);
        presentInfo.setResults(result);

        RHI_ASSERT(context.m_graphicsQueue.presentKHR(presentInfo) == vk::Result::eSuccess);
        RHI_ASSERT(swapchain.SwapImages() == RHI::ResultCode::Success);
    }
}

void FrameScheduler::TransientAllocatorBegin()
{
}

void FrameScheduler::TransientAllocatorEnd()
{
}

RHI::Buffer* FrameScheduler::AllocateTransientBuffer(const RHI::BufferCreateInfo& createInfo)
{
}

RHI::Image* FrameScheduler::AllocateTransientImage(const RHI::ImageCreateInfo& createInfo)
{
    return {};
}

}  // namespace Vulkan