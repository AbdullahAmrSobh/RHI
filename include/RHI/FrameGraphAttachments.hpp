#pragma once

#include <span>
#include <string>

#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{

struct PassAttachment;

class Pass;

using AttachmentName     = const char*;
using PassAttachmentList = std::vector<std::unique_ptr<PassAttachment>>;

enum class AttachmentType
{
    Image,
    Buffer,
    Swapchain,
};

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

enum class AttachmentAccess
{
    None,       // Invalid option
    Read,       // Attachment is read as a shader resource
    Write,      // Attachment is renderTargetOutput
    ReadWrite,  // Attachment is available for read and write as a shader resource
};

enum class AttachmentLifetime
{
    // Attachment resource is created outside of the frame
    Persistent,
    // Attachment resource is only valid for the duration of the current frame
    Transient,
};

enum class ImageLoadOperation
{
    // The attachment load operation undefined
    DontCare,
    // Load attachment content
    Load,
    // Discard attachment content
    Discard,
};

enum class ImageStoreOperation
{
    // Attachment Store operation is undefined
    DontCare,
    // Writes to the attachment are stored
    Store,
    // Writes to the attachment are discarded
    Discard,
};

enum class BlendFactor
{
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
};

enum class BlendEquation
{
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
};

union ClearValue
{
    struct
    {
        float r, g, b, a;
    } asFloat;

    struct
    {
        uint8_t r, g, b, a;
    } asUint8;

    struct
    {
        float   depth;
        uint8_t stencil;
    } asDepthStencil;
};

struct ImageLoadStoreOperations
{
    ImageLoadOperation  loadOperation;
    ImageStoreOperation storeOperation;
};

struct ImageAttachmentBlendInfo
{
    bool          blendEnable;
    BlendEquation colorBlendOp;
    BlendFactor   srcColor;
    BlendFactor   dstColor;
    BlendEquation alphaBlendOp;
    BlendFactor   srcAlpha;
    BlendFactor   dstAlpha;
};

struct ImageAttachmentUseInfo
{
    ImageViewCreateInfo      viewInfo;
    ImageLoadStoreOperations loadStoreOperations;
    ImageAttachmentBlendInfo blendInfo;
    ClearValue               clearValue;
};

struct BufferAttachmentUseInfo
{
    BufferViewCreateInfo viewInfo;
};

/// @brief Represent an attachment resource
struct Attachment
{
    Attachment(AttachmentName name, Image& image);
    Attachment(AttachmentName name, Buffer& buffer);
    Attachment(AttachmentName name, Swapchain& buffer);
    Attachment(AttachmentName name, const ImageCreateInfo& info);
    Attachment(AttachmentName name, const BufferCreateInfo& info);

    PassAttachment* Use(Pass& pass, AttachmentUsage usage, AttachmentAccess acces, const ImageAttachmentUseInfo& useInfo);
    PassAttachment* Use(Pass& pass, AttachmentUsage usage, AttachmentAccess acces, const BufferAttachmentUseInfo& useInfo);

    AttachmentName name;

    const AttachmentLifetime lifetime;

    const AttachmentType type;

    union
    {
        Image*     image;
        Buffer*    buffer;
        Swapchain* swapchain;
    };

    union
    {
        ImageCreateInfo  imageInfo;
        BufferCreateInfo bufferInfo;
    };

    PassAttachmentList useList;
};

/// @brief Represent how a Pass will use an attachment
struct PassAttachment
{
    PassAttachment(Attachment* attachment, Pass* pass, AttachmentUsage usage, AttachmentAccess access, const ImageAttachmentUseInfo& useInfo);

    PassAttachment(Attachment* attachment, Pass* pass, AttachmentUsage usage, AttachmentAccess access, const BufferAttachmentUseInfo& useInfo);

    Attachment* attachment;

    Pass* pass;

    AttachmentUsage  usage;
    AttachmentAccess access;

    PassAttachment* nextUse;
    PassAttachment* pervUse;

    union
    {
        ImageView*  imageView;
        BufferView* bufferView;
    };

    union
    {
        BufferAttachmentUseInfo bufferInfo;
        ImageAttachmentUseInfo  imageInfo;
    };
};

// AttachmentsRegistry creates, tracks, and manage all attachments for the current frame
class RHI_EXPORT AttachmentsRegistry
{
    friend class FrameScheduler;

public:
    AttachmentsRegistry() = default;

    void Reset();

    // Import an external image resource as an attachment for the current frame
    void ImportImage(std::string name, Image& image);

    // Import an external buffer resource as an attachment for the current frame
    void ImportBuffer(std::string name, Buffer& buffer);

    // Import an swapchain's current image as an attachment for the current frame
    void ImportSwapchain(std::string name, Swapchain& swapchain);

    // Create a transient image attachment that is only valid for the current frame
    void CreateTransientImageAttachment(std::string name, const ImageCreateInfo& createInfo);

    // Create a transient buffer attachment that is only valid for the current frame
    void CreateTransientBufferAttachment(std::string name, const BufferCreateInfo& createInfo);

    inline std::span<Attachment*> GetTransientAttachments()
    {
        return m_transientResources;
    }

private:
    std::unordered_map<std::string, Attachment> m_attachmentsLookup;

    std::vector<Attachment*> m_imageAttachments;

    std::vector<Attachment*> m_bufferAttachments;

    std::vector<Attachment*> m_swapchainAttachments;

    std::vector<Attachment*> m_transientResources;
};

}  // namespace RHI