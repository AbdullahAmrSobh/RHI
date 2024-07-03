#pragma once

#include "RHI/Export.hpp"

#include "RHI/Common/Containers.h"
#include "RHI/Common/Ptr.hpp"

#include "RHI/Attachment.hpp"

namespace RHI
{
    class CommandList;

    enum class PassFlags
    {
        None,
        Graphics,
        Compute,
        Transfer,
    };

    RHI_DEFINE_FLAG_OPERATORS(PassFlags);

    struct PassCreateInfo
    {
        const char*      name;
        Flags<PassFlags> flags;
    };

    struct RenderPassAttachments
    {
        ImageSize2D                     size;                   // Size of the render targets
        RenderTargetLayoutDesc          layout;                 // Color formats of the pass
        TL::Vector<ImagePassAttachment> colorAttachments;       // List of color attachments used by this pass
        ImagePassAttachment             depthStencilAttachment; // List of depth-stencil attachments used by this pass
    };

    class RHI_EXPORT Pass
    {
    public:
        // Pass(const PassCreateInfo& createInfo);
        // ~Pass() = default;

        RHI_NODISCARD const char* GetName() const;

        RHI_NODISCARD TL::Span<const ImagePassAttachment> GetImageAttachments() const;

        RHI_NODISCARD TL::Span<const BufferPassAttachment> GetBufferAttachments() const;

        RHI_NODISCARD Flags<PassFlags> GetFlags() const;

        RHI_NODISCARD ImageSize2D GetRenderTargetSize() const;

        RHI_NODISCARD TL::Span<const ImagePassAttachment> GetColorAttachments() const;

        RHI_NODISCARD ImagePassAttachment GetDepthStencilAttachment() const;

        friend class RenderGraph;

        TL::String                       name;
        Flags<PassFlags>                 flags;
        RenderPassAttachments            renderingAttachments;
        TL::Vector<ImagePassAttachment>  imagesAttachments;
        TL::Vector<BufferPassAttachment> bufferAttachments;
        Ptr<CommandList>                 primaryCommandList;
        TL::Vector<Handle<CommandList>>  commandLists;
    };

} // namespace RHI