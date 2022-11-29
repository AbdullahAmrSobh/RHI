#pragma once
#include "RHI/FrameGraphAttachment.hpp"
#include "RHI/Swapchain.hpp"

namespace RHI
{

SwapchainFrameAttachment::SwapchainFrameAttachment(std::string name, Unique<ISwapchain>& swapchain)
    : ImageFrameAttachment(name, nullptr, swapchain->GetBackBuffersDesc())
{
}

IImage& SwapchainFrameAttachment::GetResource()
{
    uint32_t index = m_pSwapchain->GetCurrentBackBufferIndex();
    return *m_pSwapchain->GetBackImages()[index];
}

const IImage& SwapchainFrameAttachment::GetResource() const
{
    uint32_t index = m_pSwapchain->GetCurrentBackBufferIndex();
    return *m_pSwapchain->GetBackImages()[index];
}

const ISwapchain& SwapchainFrameAttachment::GetSwapchain() const
{
    return *m_pSwapchain;
}

const SwapchainFrameAttachment::ResourceDesc& SwapchainFrameAttachment::GetDesc() const
{
    return m_pSwapchain->GetBackBuffersDesc();
}

} // namespace RHI