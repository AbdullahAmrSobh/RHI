#pragma once

#include <memory>
#include <span>
#include <string>
#include <vector>

#include "RHI/Export.hpp"
#include "RHI/Flags.hpp"
#include "RHI/Object.hpp"

namespace RHI
{

class Context;

enum class QueueType
{
    Graphics,
    Compute,
    Transfer,
};

struct QueueInfo
{
    QueueType type;
    uint32_t  id;
};

struct PassCreateInfo
{
    std::string name;
    QueueType   type;
};

/// @brief Represents a pass, which encapsulates a GPU task.
class RHI_EXPORT Pass
{
public:
    virtual ~Pass() = default;
    
    /// @brief Return the current GPU state of the Pass's commandList weather it has been executed, submitted, finished or pending.
    virtual FenceState GetPassState() const;

private:
    /// @brief Pointer to the RHI context.
    Context* m_context;

    /// @brief The name of this pass.
    std::string m_name;

    /// @brief The queue which this pass is executed on.
    QueueInfo m_queueInfo;

    /// @brief An optional fence, which would be signaled when this pass is finished executing.
    Fence* m_signalFence;

    /// @brief An optional swapchain used by this pass, which would be presented at the end of the pass.
    Swapchain* m_swapchainToPresent;

    /// @brief A list of all image attachments which would be used during this pass.
    std::vector<Handle> m_imageAttachments;

    /// @brief A list of all buffer attachments which would be used during this pass.
    std::vector<Handle> m_bufferAttachments;

    /// @brief A list of all all passes that should be executed before this pass.
    std::vector<Pass*> m_producers;

    /// @brief A list of all all passes that should be executed after this pass.
    std::vector<Pass*> m_consumers;
};

}  // namespace RHI