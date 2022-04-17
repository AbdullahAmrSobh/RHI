#pragma once
#include "RHI/FrameGraphEnums.hpp"

namespace RHI
{

class ISwapChain;

struct AttachmentId 
{
    explicit AttachmentId();
    
    bool operator==(const AttachmentId& other) const;
    bool operator!=(const AttachmentId& other) const;
    
    operator uint64_t() const;
    
    EResourceType   resourceType;
    EAttachmentType attachmentType;
};

struct ImageAttachmentId final : AttachmentId
{
    using AttachmentId::AttachmentId;
};

struct BufferAttachmentId final : AttachmentId
{
    using AttachmentId::AttachmentId;
};

struct SwapChainAttachmentId final : AttachmentId
{
	using AttachmentId::AttachmentId;
};

struct BufferAttachmentDesc
{
    BufferAttachmentDesc() = default;

    bool operator==(const BufferAttachmentDesc& other) const;
    bool operator!=(const BufferAttachmentDesc& other) const;
    
    EBufferFormat format;
    size_t        size;
};

struct ImageAttachmentDesc
{
    ImageAttachmentDesc() = default;

    bool operator==(const ImageAttachmentDesc& other) const;
    bool operator!=(const ImageAttachmentDesc& other) const;

    EPixelFormat format;
    Extent2D     extent;
};

} // namespace RHI
