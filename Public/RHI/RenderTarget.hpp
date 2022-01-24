#pragma once
#include "RHI/Common.hpp"
#include "RHI/Definitions.hpp"
#include "RHI/Texture.hpp"

namespace RHI
{

enum class ERenderTargetAttacmentUsage
{
    Color        = 1,
    Depth        = 2,
    Stencil      = 3,
    DepthStencil = 4,
    Present      = 5,
};

struct RenderTargetAttachmentDesc
{
    RenderTargetAttachmentDesc() = default;
    RenderTargetAttachmentDesc(EPixelFormat format, ESampleCount sampleCount, ITextureView* pAttachmentView, Extent2D extent,
                               ERenderTargetAttacmentUsage usage)
        : format(format)
        , sampleCount(sampleCount)
        , pAttachmentView(pAttachmentView)
        , extent(extent)
        , usage(usage)
    {
    }

    EPixelFormat                format;
    ESampleCount                sampleCount;
    ITextureView*               pAttachmentView;
    Extent2D                    extent;
    ERenderTargetAttacmentUsage usage;
};

struct RenderTargetDesc
{
    RenderTargetDesc(const Extent2D _extent, ArrayView<RenderTargetAttachmentDesc> _attachments)
        : extent(_extent)
        , attachments(_attachments)
    {
    }
    RenderTargetDesc(const Extent2D _extent, ArrayView<RenderTargetAttachmentDesc> _attachments, RenderTargetAttachmentDesc _depthAttachment);

    ArrayView<RenderTargetAttachmentDesc> attachments;
    RenderTargetAttachmentDesc            depthAttachment;
    bool                                  hasDepthStencil;
    Extent2D                              extent;
};

class IRenderTarget
{
public:
    virtual ~IRenderTarget() = default;
};

using RenderTargetPtr = Unique<IRenderTarget>;

} // namespace RHI
