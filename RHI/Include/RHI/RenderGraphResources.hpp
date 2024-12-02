#pragma once

#include "RHI/Handle.hpp"
#include "RHI/Image.hpp"
#include "RHI/Buffer.hpp"
#include "RHI/BindGroup.hpp"
#include "RHI/PipelineAccess.hpp"
#include "RHI/Queue.hpp"
#include "RHI/RenderTarget.hpp"

#include <TL/Flags.hpp>
#include <TL/Containers.hpp>
#include <TL/UniquePtr.hpp>
#include <TL/Memory.hpp>

#include <cstdint>

namespace RHI
{
    class Swapchain;
    class Pass;

    enum class RenderGraphResourceFlags : uint8_t
    {
        None,
        Imported,
        Transient,
        Swapchain,
    };

    enum class RenderGraphResourceAccessType : uint8_t
    {
        None,
        Image,
        Buffer,
        RenderTarget,
        Resolve,
        SwapchainPresent,
    };

    class RenderGraphResource
    {
        friend class RenderGraph;

    public:
        bool isSwapchainImage = false;
        struct AccessedResource
        {
            Pass*                         pass = nullptr;
            AccessedResource*             next = nullptr;
            AccessedResource*             prev = nullptr;
            RenderGraphResourceAccessType type = RenderGraphResourceAccessType::None;

            union
            {
                bool             asNone = false;

                RenderTargetInfo asRenderTarget;

                struct
                {
                    class RenderGraphImage*  image;
                    ImageUsage               usage;
                    TL::Flags<PipelineStage> stage;
                    TL::Flags<Access>        access;
                    ImageSubresourceRange    subresourceRange;
                } asImage;

                struct
                {
                    class RenderGraphBuffer* buffer;
                    BufferUsage              usage;
                    TL::Flags<PipelineStage> stage;
                    TL::Flags<Access>        access;
                    BufferSubregion          subregion;
                } asBuffer;

                struct
                {
                    Swapchain*               swapchain;
                    TL::Flags<PipelineStage> stage;
                } asSwapchain;
            };
        };

    public:
        const char*             GetName() const { return m_name.c_str(); }

        const AccessedResource* GetFirstAccess() const { return m_first; }

        AccessedResource*       GetFirstAccess() { return m_first; }

        const AccessedResource* GetLastAccess() const { return m_last; }

        AccessedResource*       GetLastAccess() { return m_last; }

        const Pass*             GetFirstPass() const { return m_first->pass; }

        Pass*                   GetFirstPass() { return m_first->pass; }

        const Pass*             GetLastPass() const { return m_last->pass; }

        Pass*                   GetLastPass() { return m_last->pass; }

        /// @brief Adds an access to the resource and updates usage flags.
        void                    PushAccess(AccessedResource* access);

    protected:
        RenderGraphResource(const char* name, RenderGraphResourceAccessType type, TL::Flags<RenderGraphResourceFlags> flags);

        TL::String                          m_name;
        AccessedResource*                   m_first;
        AccessedResource*                   m_last;
        TL::Flags<RenderGraphResourceFlags> m_flags;
        RenderGraphResourceAccessType       m_type;
        Format                              m_format = Format::Unknown;

        union
        {
            Handle<Image>  asImage;
            Handle<Buffer> asBuffer;
        } m_handle;

        union
        {
            TL::Flags<ImageUsage>  asImage;
            TL::Flags<BufferUsage> asBuffer;
        } m_usage;
    };

    using PassAccessedResource = RenderGraphResource::AccessedResource;

    class RenderGraphImage final : public RenderGraphResource
    {
        friend RenderGraph;

    public:
        RenderGraphImage(const char* name, Handle<Image> image, Format format);
        RenderGraphImage(const char* name, Format format);

        Handle<Image>         GetImage() const { return m_handle.asImage; }

        Format                GetFormat() const { return m_format; }

        TL::Flags<ImageUsage> GetImageUsage() const { return m_usage.asImage; }
    };

    class RenderGraphBuffer final : public RenderGraphResource
    {
    public:
        RenderGraphBuffer(const char* name, Handle<Buffer> buffer);
        RenderGraphBuffer(const char* name);

        Handle<Buffer>         GetBuffer() const { return m_handle.asBuffer; }

        TL::Flags<BufferUsage> GetBufferUsage() const { return m_usage.asBuffer; }
    };

} // namespace RHI
