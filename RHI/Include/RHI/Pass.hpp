#pragma once

#include <memory>
#include <string>
#include <vector>

#include "RHI/Attachment.hpp"
#include "RHI/Export.hpp"
#include "RHI/Flags.hpp"
#include "RHI/Object.hpp"
#include "RHI/Span.hpp"

namespace RHI
{

class Context;
class FrameScheduler;
class CommandList;

enum class PassQueueState
{
    NotSubmitted,
    Pending,
    Executing,
    Finished,
    Building,
    Compiling,
    Failed,
};

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
class Pass : public Object
{
public:
    using Object::Object;
    virtual ~Pass() = default;

    /// @brief Called at the beginning of this pass building phase.
    void Begin();

    /// @brief Called at the end of this pass building phase.
    void End();

    /// @brief Used to inspect the current state of this pass.
    PassQueueState GetPassQueueState() const;

    /// @brief Adds a pass to the wait list.
    void ExecuteAfter(Pass& pass);

    /// @brief Adds a pass to the signal list.
    void ExecuteBefore(Pass& pass);

    /// @brief Imports an external image resource to be used in this pass.
    /// @param image handle to the image resource.
    /// @param useInfo resource use information.
    /// @param usage resource shader usage
    /// @param access resource shader access
    /// @return Handle to an image view into the used resource.
    Handle<ImageView> ImportImageResource(Handle<Image> image, const ImageAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access);

    /// @brief Imports an external buffer resource to be used in this pass.
    /// @param buffer handle to the buffer resource.
    /// @param useInfo resource use information.
    /// @param usage resource shader usage
    /// @param access resource shader access
    /// @return Handle to an buffer view into the used resource.
    Handle<BufferView> ImportBufferResource(Handle<Buffer> buffer, const BufferAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access);

    /// @brief Creates a new transient image resource, and use it in this pass.
    /// @param createInfo transient image create info.
    /// @param usage resource shader usage
    /// @param access resource shader access
    /// @return Handle to an image view into the used resource.
    Handle<ImageView> CreateTransientImageResource(const TransientImageCreateInfo& createInfo, AttachmentUsage usage, AttachmentAccess access);

    /// @brief Creates a new transient buffer resource, and use it in this pass.
    /// @param createInfo transient buffer create info.
    /// @param usage resource shader usage
    /// @param access resource shader access
    /// @return Handle to an buffer view into the used resource.
    Handle<BufferView> CreateTransientBufferResource(const TransientBufferCreateInfo& createInfo, AttachmentUsage usage, AttachmentAccess access);

    /// @brief Use an existing image resource in this pass.
    /// @param view Handle to the used resource.
    /// @param useInfo image resource use information.
    /// @param usage resource shader usage.
    /// @param access resource shader access.
    /// @return Handle to an image resource.
    Handle<ImageView> UseImageResource(Handle<ImageView> view, const ImageAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access);

    /// @brief Use an existing buffer resource in this pass.
    /// @param view Handle to the used resource.
    /// @param useInfo buffer resource use information.
    /// @param usage resource shader usage.
    /// @param access resource shader access.
    /// @return Handle to an buffer resource.
    Handle<BufferView> UseBufferResource(Handle<BufferView> view, const BufferAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access);

    /// @brief Return an list of command list executed during this pass.
    /// @param commandsCount Number of submit commands.
    /// @return reference to the command lists
    virtual CommandList& GetCommandList(uint32_t commandsCount = 1) = 0;

protected:
    virtual void OnBegin() = 0;

    virtual void OnEnd() = 0;

    /// @brief Used to inspect the current state of this pass in the command queue.
    virtual PassQueueState GetPassQueueStateInternal() const = 0;

protected:
    FrameScheduler* m_scheduler;

    std::vector<Pass*> m_waitPasses;

    std::vector<Handle<ImageView>> m_usedImageAttachments;

    std::vector<Handle<BufferView>> m_usedBufferAttachments;

};

}  // namespace RHI