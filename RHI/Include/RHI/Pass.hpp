#pragma once

#include "RHI/Export.hpp"
#include "RHI/ResourcePool.hpp"
#include "RHI/Handle.hpp"

namespace RHI
{

class Context;
class Pass;
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

/// @brief Enumerates the different types of attachments that can be used.
enum class AttachmentType
{
    Image,
    Buffer,
    Swapchain,
    Resolve,
};

/// @brief Enumerates the different types the attachment can be used as.
enum class AttachmentUsage
{
    None,
    RenderTarget,
    Depth,
    Stencil,
    DepthStencil,
    ShaderResource,
    Copy,
    Resolve,
};

/// @brief Enumerates how an attachment is access
enum class AttachmentAccess
{
    /// @brief Invalid option.
    None,
    /// @brief Attachment is read as a shader resource.
    Read,
    /// @brief Attachment is renderTargetOutput.
    Write,
    /// @brief Attachment is available for read and write as a shader resource.
    ReadWrite,
};

/// @brief Enumerates the different types of attachment's lifetime.
enum class AttachmentLifetime
{
    // Attachment resource is created outside of the frame
    Persistent,
    // Attachment resource is only valid for the duration of the current frame
    Transient,
};

/// @brief Enumerates ...
enum class ImageLoadOperation
{
    /// @brief The attachment load operation undefined.

    DontCare,
    /// @brief Load attachment content.

    Load,
    /// @brief Discard attachment content.

    Discard,
};

/// @brief Enumerates ...
enum class ImageStoreOperation
{
    // Attachment Store operation is undefined
    DontCare,
    // Writes to the attachment are stored
    Store,
    // Writes to the attachment are discarded
    Discard,
};

struct ColorValue
{
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;

    inline bool operator==(const ColorValue& other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    inline bool operator!=(const ColorValue& other) const
    {
        return !(r == other.r && g == other.g && b == other.b && a == other.a);
    }
};

struct DepthStencilValue
{
    float   depthValue   = 1.0f;
    uint8_t stencilValue = 0xff;

    inline bool operator==(const DepthStencilValue& other) const
    {
        return depthValue == other.depthValue && stencilValue == other.stencilValue;
    }

    inline bool operator!=(const DepthStencilValue& other) const
    {
        return !(depthValue == other.depthValue && stencilValue == other.stencilValue);
    }
};

struct ClearValue
{
    ColorValue        color;
    DepthStencilValue depth;

    inline bool operator==(const ClearValue& other) const
    {
        return color == other.color && depth == other.depth;
    }

    inline bool operator!=(const ClearValue& other) const
    {
        return !(color == other.color && depth == other.depth);
    }
};

/// @brief Structure specifying the load and store opertions for image attachment.
struct ImageLoadStoreOperations
{
    ImageLoadOperation  loadOperation  = ImageLoadOperation::Load;
    ImageStoreOperation storeOperation = ImageStoreOperation::Store;

    inline bool operator==(const ImageLoadStoreOperations& other) const
    {
        return loadOperation == other.loadOperation && storeOperation == other.storeOperation;
    }

    inline bool operator!=(const ImageLoadStoreOperations& other) const
    {
        return !(loadOperation == other.loadOperation && storeOperation == other.storeOperation);
    }
};

/// @brief Structure specifying the parameters of an image attachment.
struct ImageAttachmentUseInfo
{
    ComponentMapping         components;
    ImageSubresource         subresource;
    ImageLoadStoreOperations loadStoreOperations;
    ClearValue               clearValue;

    AttachmentUsage  usage = AttachmentUsage::None;
    AttachmentAccess access = AttachmentAccess::None;

    inline bool operator==(const ImageAttachmentUseInfo& other) const
    {
        return components == other.components && subresource == other.subresource && loadStoreOperations == other.loadStoreOperations && clearValue == other.clearValue && usage == other.usage && access == other.access;
    }

    inline bool operator!=(const ImageAttachmentUseInfo& other) const
    {
        return !(components == other.components && subresource == other.subresource && loadStoreOperations == other.loadStoreOperations && clearValue == other.clearValue && usage == other.usage && access == other.access);
    }
};

/// @brief Structure specifying the parameters of an buffer attachment.
struct BufferAttachmentUseInfo
{
    Format format;
    size_t byteOffset;
    size_t byteSize;

    AttachmentUsage  usage = AttachmentUsage::None;
    AttachmentAccess access = AttachmentAccess::None;

    inline bool operator==(const BufferAttachmentUseInfo& other) const
    {
        return format == other.format && byteOffset == other.byteOffset && byteSize == other.byteSize && usage == other.usage && access == other.access;
    }

    inline bool operator!=(const BufferAttachmentUseInfo& other) const
    {
        return !(format == other.format && byteOffset == other.byteOffset && byteSize == other.byteSize && usage == other.usage && access == other.access);
    }
};

struct ImageAttachment
{
    std::string name;

    AttachmentLifetime lifetime;

    ImageCreateInfo info;

    Handle<Image> image;
};

struct BufferAttachment
{
    std::string name;

    AttachmentLifetime lifetime;

    BufferCreateInfo info;

    Handle<Buffer> buffer;
};

struct ImagePassAttachment
{
    Handle<ImageAttachment> attachment;

    Pass* pass;

    ImageAttachmentUseInfo info;

    Handle<struct ImageView> view;
};

struct BufferPassAttachment
{
    Handle<BufferAttachment> attachment;

    Pass* pass;

    BufferAttachmentUseInfo info;

    Handle<struct BufferView> view;
};

using ImagePassAttachmentList  = std::vector<ImagePassAttachment>;
using BufferPassAttachmentList = std::vector<BufferPassAttachment>;

// clang-format off
struct ImageView  {};
struct BufferView {};
// clang-format on

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
    /// @return Handle to an image view into the used resource.
    ImagePassAttachment ImportImageResource(std::string name, Handle<Image> image, const ImageAttachmentUseInfo& useInfo);

    /// @brief Imports an external buffer resource to be used in this pass.
    /// @param buffer handle to the buffer resource.
    /// @param useInfo resource use information.
    /// @return Handle to an buffer view into the used resource.
    BufferPassAttachment ImportBufferResource(std::string name, Handle<Buffer> buffer, const BufferAttachmentUseInfo& useInfo);

    /// @brief Creates a new transient image resource, and use it in this pass.
    /// @param createInfo transient image create info.
    /// @return Handle to an image view into the used resource.
    ImagePassAttachment CreateTransientImageResource(std::string name, const ImageCreateInfo& createInfo, const ImageAttachmentUseInfo& useInfo);

    /// @brief Creates a new transient buffer resource, and use it in this pass.
    /// @param createInfo transient buffer create info.
    /// @return Handle to an buffer view into the used resource.
    BufferPassAttachment CreateTransientBufferResource(std::string name, const BufferCreateInfo& createInfo, const BufferAttachmentUseInfo& useInfo);

    /// @brief Use an existing image resource in this pass.
    /// @param view Handle to the used resource.
    /// @param useInfo image resource use information.
    /// @return Handle to an image resource.
    ImagePassAttachment UseImageResource(const ImagePassAttachment& view, const ImageAttachmentUseInfo& useInfo);

    /// @brief Use an existing buffer resource in this pass.
    /// @param view Handle to the used resource.
    /// @param useInfo buffer resource use information.
    /// @return Handle to an buffer resource.
    BufferPassAttachment UseBufferResource(const BufferPassAttachment& view, const BufferAttachmentUseInfo& useInfo);

    /// @brief Begins the command list associated with this pass.
    /// @param commandsCount Number of commands to be submitted.
    /// @return reference to the command lists
    virtual CommandList& BeginCommandList(uint32_t commandsCount = 1) = 0;
    
    /// @brief Ends the command list of assoicated with this pass.
    virtual void EndCommandList() = 0;

protected:
    virtual void OnBegin() = 0;

    virtual void OnEnd() = 0;

    /// @brief Used to inspect the current state of this pass in the command queue.
    virtual PassQueueState GetPassQueueStateInternal() const = 0;

protected:
    FrameScheduler* m_scheduler;

    /// @brief A list of passes that must be executed before this pass.
    std::vector<Pass*> m_waitPasses;

    /// @brief Handle to depth stencil attachment (if present).
    Handle<ImageView> m_depthStencilAttachment;

    /// @brief A list of all image resource used by this pass.
    std::vector<Handle<ImageView>> m_usedImages;

    /// @brief A list of all buffer resources used by this pass.
    std::vector<Handle<BufferView>> m_usedBuffers;
};

}  // namespace RHI