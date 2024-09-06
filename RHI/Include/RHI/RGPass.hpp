#pragma once

#include "RHI/Export.hpp"
#include "RHI/Image.hpp"
#include "RHI/RGResources.hpp"

#include <TL/Containers.hpp>

namespace RHI
{
    class CommandList;

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
    };

    class RHI_EXPORT Pass final
    {
    public:
        TL::String                        m_name;
        ImageSize2D                       m_renderTargetSize;
        TL::Vector<ImagePassAttachment*>  m_colorAttachments;
        ImagePassAttachment*              m_depthStencilAttachment;
        TL::Vector<ImagePassAttachment*>  m_imageAttachments;
        TL::Vector<BufferPassAttachment*> m_bufferAttachments;
        TL::Vector<CommandList*>          m_commandLists;
    };

} // namespace RHI