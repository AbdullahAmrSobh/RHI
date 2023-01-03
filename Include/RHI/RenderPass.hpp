#pragma once
#include "RHI/Attachment.hpp"
#include "RHI/Swapchain.hpp"

namespace RHI
{

class ICommandBuffer;

class IRenderPass
{
public:
    IRenderPass(std::string name)
        : m_name(std::move(name))
    {
    }

    virtual ~IRenderPass() = default;

    void Reset()
    {
        m_usedSwapchainAttachment = nullptr;
        m_usedImageAttachments.clear();
    }

    ISwapchain* GetSwapchain()
    {
        return m_usedSwapchainAttachment->GetAttachment().GetSwapchain();
    }

    const UsedImageAttachment* GetUsedSwapchainAttachment() const
    {
        return m_usedSwapchainAttachment;
    }

    UsedImageAttachment* GetUsedSwapchainAttachment()
    {
        return m_usedSwapchainAttachment;
    }

    std::span<const UsedImageAttachment* const> GetUsedAttachments() const
    {
        return m_usedImageAttachments;
    }

    std::span<UsedImageAttachment* const> GetUsedAttachments()
    {
        return m_usedImageAttachments;
    }

private:
    friend class FrameGraphBuilder;

    std::string m_name;

    UsedImageAttachment*              m_usedSwapchainAttachment;
    std::vector<UsedImageAttachment*> m_usedImageAttachments;
};

}  // namespace RHI