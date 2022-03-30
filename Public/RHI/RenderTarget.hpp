#pragma once
#include "RHI/Definitions.hpp"

#include "RHI/Texture.hpp"

namespace RHI
{

struct RenderTargetAttachmentDesc
{
    RenderTargetAttachmentDesc() = default;
    ITextureView* pView          = nullptr;
    EPixelFormat  format         = EPixelFormat::None;
};

struct RenderTargetDesc
{

    explicit RenderTargetDesc(Extent3D extent, const std::initializer_list<RenderTargetAttachmentDesc> attachments)
        : extent(extent)
        , attachments(attachments)
    {
    }

    explicit RenderTargetDesc(Extent3D extent, const std::initializer_list<RenderTargetAttachmentDesc> attachments,
                              const RenderTargetAttachmentDesc& depthStencilAttachment)
        : extent(extent)
        , attachments(attachments)
        , depthStencilAttachment(depthStencilAttachment)
    {
    }

    Extent3D                                extent;
    std::vector<RenderTargetAttachmentDesc> attachments;
    RenderTargetAttachmentDesc              depthStencilAttachment;

    inline bool IsDepthStencilEnabled() const { return depthStencilAttachment.pView != nullptr; }
};

class IRenderTarget
{
public:
    virtual ~IRenderTarget() = default;
    
    virtual const ArrayView<const ITextureView*> GetAttachments() const = 0;
};
using RenderTargetPtr = Unique<IRenderTarget>;

} // namespace RHI
