#pragma once
#include "RHI/FrameGraphPass.hpp"
#include "RHI/Swapchain.hpp"

namespace RHI
{

std::string_view IPass::GetName() const
{
    return m_name;
}

EPassType IPass::GetType() const
{
    return m_type;
}

const IFence& IPass::GetFence() const
{
    return *m_signalFence;
}

bool IPass::HasSwapchainTarget() const
{
    return m_pSwapchain != nullptr;
}

bool IPass::HasDepthStencil() const
{
    return m_depthStencilAttachment != nullptr;
}

const ImagePassAttachment* IPass::GetSwapchainAttachemnt() const
{
    return m_swapchainImagePassAttachments[m_pSwapchain->GetCurrentBackBufferIndex()].get();
}

const ImagePassAttachment* IPass::GetDepthStencilAttachment() const
{
    return m_depthStencilAttachment.get();
}

const std::vector<const ImagePassAttachment*> IPass::GetImageAttachments() const
{
    std::vector<const ImagePassAttachment*> result;
    result.reserve(m_imageAttachments.size());
    for (auto& attachment : m_imageAttachments)
    {
        result.push_back(attachment.get());
    }
    return result;
}

const std::vector<const BufferPassAttachment*> IPass::GetBufferAttachments() const
{
    std::vector<const BufferPassAttachment*> result;
    result.reserve(m_bufferAttachments.size());
    for (auto& attachment : m_bufferAttachments)
    {
        result.push_back(attachment.get());
    }
    return result;
}

} // namespace RHI