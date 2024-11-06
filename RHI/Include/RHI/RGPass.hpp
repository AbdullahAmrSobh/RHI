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
        friend class RenderGraph;

    public:
        // Pass();
        // Pass(Pass&&)      = default;
        // Pass(const Pass&) = delete;
        // ~Pass()           = default;

        const char*                                  GetName() const;

        void                                         Resize(ImageSize2D size);

        ImageSize2D                                  GetSize() const;

        TL::String                                   m_name;
        ImageSize2D                                  m_renderTargetSize;
        TL::Vector<RGImagePassAccess*>               m_colorAttachments;
        RGImagePassAccess*                           m_depthStencilAttachment;
        TL::Vector<RGImagePassAccess*>               m_imageAttachments;
        TL::Vector<RGBufferPassAccess*>              m_bufferAttachments;

        TL::Function<void(CommandList& commandList)> m_callback;
    };

    inline const char* Pass::GetName() const
    {
        return m_name.c_str();
    }

    inline void Pass::Resize(ImageSize2D size)
    {
        m_renderTargetSize = size;
    }

    inline ImageSize2D Pass::GetSize() const
    {
        return m_renderTargetSize;
    }

} // namespace RHI