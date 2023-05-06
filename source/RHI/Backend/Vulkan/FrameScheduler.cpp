#pragma once

#include "RHI/Backend/Vulkan/FrameScheduler.hpp"

#include "RHI/Backend/Vulkan/Command.hpp"
#include "RHI/Backend/Vulkan/Context.hpp"
#include "RHI/FrameGraphInterface.hpp"

namespace Vulkan
{

RHI::CommandList& FrameScheduler::BeginPassCommandList(uint32_t dispatchIndex)
{
    Context&   context = *m_context;
    vk::Device device  = context.GetDevice();

    vk::CommandBufferAllocateInfo allocateInfo {};
    auto                          handle = device.allocateCommandBuffers(allocateInfo).value.front();

    m_commandBuffer = new CommandList(handle);

    return *m_commandBuffer;
}

void FrameScheduler::EndPassCommandList()
{
    delete m_commandBuffer;
}

void FrameScheduler::PassPresent(RHI::PassState& passState)
{
    Context&   context = *m_context;
    vk::Device device  = context.GetDevice();
    vk::Queue  queue   = context.m_graphicsQueue;

    uint32_t                   imageIndex;
    std::vector<vk::Semaphore> waitSemaphores;
    vk::SwapchainKHR           swapchain;

    vk::PresentInfoKHR presentInfo {};
    presentInfo.setSwapchainCount(1);
    presentInfo.setPSwapchains(&swapchain);
    presentInfo.setPImageIndices(&imageIndex);
    presentInfo.setWaitSemaphores(waitSemaphores);
    queue.presentKHR(presentInfo);
}

void FrameScheduler::PassExecute(RHI::PassState& passState)
{
    Context&   context = *m_context;
    vk::Device device  = context.GetDevice();
    vk::Queue  queue   = context.m_graphicsQueue;

    std::vector<vk::SemaphoreSubmitInfo>     waitSemaphores;
    std::vector<vk::SemaphoreSubmitInfo>     signalSemaphores;
    std::vector<vk::CommandBufferSubmitInfo> commandBufferInfos;

    vk::SubmitInfo2 submitInfo {};
    submitInfo.setWaitSemaphoreInfos(waitSemaphores);
    submitInfo.setSignalSemaphoreInfos(signalSemaphores);
    submitInfo.setCommandBufferInfos(commandBufferInfos);
    queue.submit2(submitInfo);
}

void FrameScheduler::CompileGraphResources()
{
}

}  // namespace Vulkan