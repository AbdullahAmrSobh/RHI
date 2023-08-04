#pragma once

#include "RHI/Command.hpp"
#include "RHI/Export.hpp"
#include "RHI/FrameGraphAttachments.hpp"

namespace RHI
{

class CommandList;

enum class PassQueue
{
    Graphics,
    Compute,
    Transfer,
};

class RHI_EXPORT Pass
{
    friend class FrameGraph;
    friend class FrameScheduler;

public:
    Pass(std::string name, PassQueue queueType)
        : m_name(std::move(name))
        , m_queueType(queueType)
    {
    }

    void Reset();

    inline const std::string& GetName() const
    {
        return m_name;
    }

    inline PassQueue GetPassQueueType() const
    {
        return m_queueType;
    }

    inline const std::span<const PassAttachment* const> GetImageAttachments() const
    {
        return m_imageAttachments;
    }

    inline std::span<PassAttachment* const> GetImageAttachments()
    {
        return m_imageAttachments;
    }

    inline const std::span<const PassAttachment* const> GetBufferAttachments() const
    {
        return m_bufferAttachments;
    }

    inline std::span<PassAttachment* const> GetBufferAttachments()
    {
        return m_bufferAttachments;
    }

    inline const PassAttachment* GetDepthStencilAttachment() const
    {
        return m_depthAttachment;
    }

    inline PassAttachment* GetDepthStencilAttachment()
    {
        return m_depthAttachment;
    }

    inline const PassAttachment* GetSwapchainAttachment() const
    {
        return m_swapchainAttachment;
    }

    inline PassAttachment* GetSwapchainAttachment()
    {
        return m_swapchainAttachment;
    }

    inline CommandList& GetCommandList()
    {
        return *m_commandList;
    }

protected:
    friend class FrameGraph;
    friend class FrameScheduler;

    // Unique name of the current pass.
    const std::string m_name;

    // Type of the queue this Pass is executed on.
    const PassQueue m_queueType;

    // Index of the queue this pass is executed on.
    uint32_t m_queueIndex;

    // The command list used to execute the pass.
    std::unique_ptr<CommandList> m_commandList;

    // A list of all passes that this pass depend on.
    std::vector<Pass*> m_waitPasses;

    // A list of all passes that depend on this Pass.
    std::vector<Pass*> m_signalPasses;

    // A list of image attachments used by this pass.
    std::vector<PassAttachment*> m_imageAttachments;

    // A list of buffer attachments used by this pass.
    std::vector<PassAttachment*> m_bufferAttachments;

    // Pointer to the depth attachment, if it exists and used by this pass.
    PassAttachment* m_depthAttachment;

    // Pointer to the swapchain attachment, if it exists and used by this pass.
    PassAttachment* m_swapchainAttachment;

    // A list fences which are signaled, when this pass finish executing.
    std::vector<Fence*> m_signalFences;
};

}  // namespace RHI