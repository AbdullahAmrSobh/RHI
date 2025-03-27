#pragma once

#include "RHI/Handle.hpp"
#include "RHI/Resources.hpp"
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
    class RenderGraph;
    class Swapchain;
    class Pass;
    class RenderGraphResource;

    struct RenderGraphResourceTransition
    {
        Pass*                          pass     = nullptr;
        RenderGraphResourceTransition* next     = nullptr;
        RenderGraphResourceTransition* prev     = nullptr;
        RenderGraphResource*           resource = nullptr;

        union
        {
            struct
            {
                ImageUsage               usage;
                TL::Flags<PipelineStage> stage;
                TL::Flags<Access>        access;
                ImageSubresourceRange    subresourceRange;
            } asImage = {};

            struct
            {
                BufferUsage              usage;
                TL::Flags<PipelineStage> stage;
                TL::Flags<Access>        access;
                BufferSubregion          subregion;
            } asBuffer;
        };
    };

    class RenderGraphResource
    {
    public:
        enum class Type : uint8_t
        {
            Image,
            Buffer
        };

        Type                                 GetType() const { return m_type; }

        const char*                          GetName() const { return m_name.c_str(); }

        const RenderGraphResourceTransition* GetFirstAccess() const { return m_first; }

        RenderGraphResourceTransition*       GetFirstAccess() { return m_first; }

        const RenderGraphResourceTransition* GetLastAccess() const { return m_last; }

        RenderGraphResourceTransition*       GetLastAccess() { return m_last; }

        const Pass*                          GetFirstPass() const { return m_first->pass; }

        Pass*                                GetFirstPass() { return m_first->pass; }

        const Pass*                          GetLastPass() const { return m_last->pass; }

        Pass*                                GetLastPass() { return m_last->pass; }

        void                                 PushAccess(RenderGraphResourceTransition* access);

    protected:
        friend class RenderGraph;

        RenderGraphResource(const char* name, Type type);

        TL::String                     m_name;
        RenderGraphResourceTransition* m_first;
        RenderGraphResourceTransition* m_last;
        Type                           m_type;
        Format                         m_format = Format::Unknown;

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

        bool isImported = false;
    };

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

    struct ColorRGAttachment
    {
        RenderGraphImage* view        = nullptr;
        LoadOperation     loadOp      = LoadOperation::Discard;
        StoreOperation    storeOp     = StoreOperation::Store;
        ClearValue        clearValue  = {.f32 = {0.0f, 0.0f, 0.0f, 1.0f}};
        ResolveMode       resolveMode = ResolveMode::None;
        RenderGraphImage* resolveView = nullptr;
    };

    struct DepthStencilRGAttachment
    {
        RenderGraphImage* view           = nullptr;
        LoadOperation     depthLoadOp    = LoadOperation::Discard;
        StoreOperation    depthStoreOp   = StoreOperation::Store;
        LoadOperation     stencilLoadOp  = LoadOperation::Discard;
        StoreOperation    stencilStoreOp = StoreOperation::Store;
        DepthStencilValue clearValue     = {0.0f, 0};
    };

} // namespace RHI
