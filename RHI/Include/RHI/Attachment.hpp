#pragma once

#include "RHI/Handle.hpp"
#include "RHI/HandleTypes.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{

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

/// @brief Enumerates ...
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

/// @brief Enumerates ...
enum class BlendEquation
{
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
};

/// @brief Structure specifying the clear values of an image render target attachment.
struct ClearValue
{
    float r, g, b, a;

    inline bool operator==(const ClearValue& other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    inline bool operator!=(const ClearValue& other) const
    {
        return !(r == other.r && g == other.g && b == other.b && a == other.a);
    }
};

/// @brief Structure specifying the load and store opertions for image attachment.
struct ImageLoadStoreOperations
{
    ImageLoadOperation  loadOperation;
    ImageStoreOperation storeOperation;

    inline bool operator==(const ImageLoadStoreOperations& other) const
    {
        return loadOperation == other.loadOperation && storeOperation == other.storeOperation;
    }

    inline bool operator!=(const ImageLoadStoreOperations& other) const
    {
        return !(loadOperation == other.loadOperation && storeOperation == other.storeOperation);
    }
};

/// @brief Structure specifying the blending parameters for an image render target attachment.
struct ImageAttachmentBlendInfo
{
    bool          blendEnable;
    BlendEquation colorBlendOp;
    BlendFactor   srcColor;
    BlendFactor   dstColor;
    BlendEquation alphaBlendOp;
    BlendFactor   srcAlpha;
    BlendFactor   dstAlpha;

    inline bool operator==(const ImageAttachmentBlendInfo& other) const
    {
        return blendEnable == other.blendEnable && colorBlendOp == other.colorBlendOp && srcColor == other.srcColor && dstColor == other.dstColor && alphaBlendOp == other.alphaBlendOp && srcAlpha == other.srcAlpha && dstAlpha == other.dstAlpha;
    }

    inline bool operator!=(const ImageAttachmentBlendInfo& other) const
    {
        return !(blendEnable == other.blendEnable && colorBlendOp == other.colorBlendOp && srcColor == other.srcColor && dstColor == other.dstColor && alphaBlendOp == other.alphaBlendOp && srcAlpha == other.srcAlpha && dstAlpha == other.dstAlpha);
    }
};

/// @brief Structure specifying the parameters of an image attachment.
struct ImageAttachmentUseInfo
{
    ComponentMapping         components;
    ImageSubresource         subresource;
    ImageLoadStoreOperations loadStoreOperations;
    ImageAttachmentBlendInfo blendInfo;
    ClearValue               clearValue;

    inline bool operator==(const ImageAttachmentUseInfo& other) const
    {
        return components == other.components && subresource == other.subresource && loadStoreOperations == other.loadStoreOperations && clearValue == other.clearValue && blendInfo == other.blendInfo;
    }

    inline bool operator!=(const ImageAttachmentUseInfo& other) const
    {
        return !(components == other.components && subresource == other.subresource && loadStoreOperations == other.loadStoreOperations && clearValue == other.clearValue && blendInfo == other.blendInfo);
    }
};

/// @brief Structure specifying the parameters of an buffer attachment.
struct BufferAttachmentUseInfo
{
    Format format;
    size_t byteOffset;
    size_t byteSize;

    inline bool operator==(const BufferAttachmentUseInfo& other) const
    {
        return format == other.format && byteOffset == other.byteOffset && byteSize == other.byteSize;
    }

    inline bool operator!=(const BufferAttachmentUseInfo& other) const
    {
        return !(format == other.format && byteOffset == other.byteOffset && byteSize == other.byteSize);
    }
};

/// @brief Attachment
struct Attachment
{
    std::string name;

    const AttachmentLifetime lifetime;

    const AttachmentType type;

    union
    {
        Handle<Image>  image;
        Handle<Buffer> buffer;
    };

    union
    {
        ImageCreateInfo  imageInfo;
        BufferCreateInfo bufferInfo;
    };
};

/// @brief Attachment
struct AttachmentView
{
    Handle<Attachment> attachment;

    Pass* pass;

    AttachmentUsage  usage;
    AttachmentAccess access;

    union
    {
        Handle<ImageView> imageView;
        Handle<ImageView> bufferView;
    };

    union
    {
        BufferAttachmentUseInfo bufferInfo;
        ImageAttachmentUseInfo  imageInfo;
    };
};

class AttachmentRegistry
{
public:
    enum class AttachmentType
    {
        TransientImage,
        TransientBuffer,
        ImportedImage,
        ImportedBuffer,
    };

    template<AttachmentType type>
    Pass* GetAttachmentFirstPass(Handle<Attachment> attachment);

    template<AttachmentType type>
    Pass* GetAttachmentLastPass(Handle<Attachment> attachment);

    template<AttachmentType type>
    Handle<AttachmentView> GetFirstUse(Handle<Attachment> attachment);

    template<AttachmentType type>
    Handle<AttachmentView> GetLastUse(Handle<Attachment> attachment);

    template<AttachmentType type>
    Handle<AttachmentView> GetNextUse(Handle<AttachmentView> attachment);

    template<AttachmentType type>
    Handle<AttachmentView> GetPrevUse(Handle<AttachmentView> attachment);

    template<AttachmentType type>
    Attachment GetAttachment(Handle<Attachment> handle);

    template<AttachmentType type>
    AttachmentView GetAttachmentView(Handle<AttachmentView> handle);

private:
    using AttachmentViewList = std::vector<AttachmentView>;
    using AttachmentPool     = HandlePool<Attachment, AttachmentViewList>;

    AttachmentPool m_transientImageAttachments;

    AttachmentPool m_transientBufferAttachments;

    AttachmentPool m_importedImageAttachments;

    AttachmentPool m_importedBufferAttachments;
};

}  // namespace RHI