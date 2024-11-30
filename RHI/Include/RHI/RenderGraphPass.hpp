#pragma once

#include "RHI/Common.hpp"
#include "RHI/Export.hpp"
#include "RHI/RenderGraphResources.hpp"

#include <TL/Containers.hpp>

namespace RHI
{
    class Pass;
    class RenderGraph;
    class CommandList;

    using PassSetupCallback   = TL::Function<void(RenderGraph& renderGraph, Pass& pass)>;
    using PassCompileCallback = TL::Function<void(RenderGraph& renderGraph, Pass& pass)>;
    using PassExecuteCallback = TL::Function<void(CommandList& commandList)>;

    enum class PassFlags
    {
        None      = 0,
        Graphics  = 1 << 0,
        Compute   = 1 << 1,
        Transfer  = 1 << 2,
        Immeidate = 1 << 3
    };

    struct PassCreateInfo
    {
        const char*          name;
        TL::Flags<PassFlags> flags;
        PassSetupCallback    setupCallback;
        PassCompileCallback  compileCallback;
        PassExecuteCallback  executeCallback;
    };

    class RHI_EXPORT Pass final
    {
        friend class RenderGraph;

    public:
        RHI_INTERFACE_BOILERPLATE(Pass);

        const char* GetName() const;

        void        Resize(ImageSize2D size);

        ImageSize2D GetSize() const;

    private:
        TL::String                               m_name;
        ImageSize2D                              m_size;
        TL::Vector<RenderGraphImagePassAccess*>  m_colorAttachments;
        RenderGraphImagePassAccess*              m_depthStencilAttachment;
        TL::Vector<RenderGraphImagePassAccess*>  m_imageAttachments;
        TL::Vector<RenderGraphBufferPassAccess*> m_bufferAttachments;
        PassSetupCallback                        m_onSetupCallback;
        PassCompileCallback                      m_onCompileCallback;
        PassExecuteCallback                      m_onExecuteCallback;
    };

} // namespace RHI