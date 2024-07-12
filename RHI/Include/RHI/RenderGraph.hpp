#pragma once

#include "RHI/Resources.hpp"

#include "RHI/Common/Handle.hpp"
#include "RHI/Definitions.hpp"

#include "RHI/Export.hpp"

namespace RHI
{
    struct Pass;
    class Context;
    class Swapchain;
    class CommandList;
    class RenderGraph;

    struct ImageAttachment;
    struct BufferAttachment;

    RHI_DECALRE_OPAQUE_RESOURCE(PassSubmitData);

    enum class AttachmentLifetime
    {
        Persistent,
        Transient,
    };

    struct ImageAttachmentUseInfo
    {
        ImageUsage            usage;
        ImageViewType         viewType;
        ImageSubresourceRange subresourceRange;
        ComponentMapping      componentMapping;
        LoadStoreOperations   loadStoreOperations;
        ClearValue            clearValue;
    };

    struct BufferAttachmentUseInfo
    {
        BufferUsage usage;
        size_t      size;
        size_t      offset;
    };

    struct ImageAttachmentList
    {
        RenderGraph*            renderGraph;
        TL::String              name;
        AttachmentLifetime      lifetime;
        uint32_t                referenceCount;
        Handle<Image>           handle;
        ImageCreateInfo         info;
        Swapchain*              swapchain;
        Handle<ImageAttachment> begin;
        Handle<ImageAttachment> end;
    };

    struct BufferAttachmentList
    {
        RenderGraph*             renderGraph;
        TL::String               name;
        AttachmentLifetime       lifetime;
        uint32_t                 referenceCount;
        Handle<Buffer>           handle;
        BufferCreateInfo         info;
        Handle<BufferAttachment> begin;
        Handle<BufferAttachment> end;
    };

    struct Attachment
    {
        Handle<Pass>       pass;
        Access             access;
        Flags<ShaderStage> stage;
    };

    struct ImageAttachment final : public Attachment
    {
        ImageAttachmentUseInfo      useInfo;
        Handle<ImageAttachmentList> list;
        Handle<ImageAttachment>     next;
        Handle<ImageAttachment>     prev;
        Handle<ImageView>           view;
    };

    struct BufferAttachment final : public Attachment
    {
        BufferAttachmentUseInfo      useInfo;
        Handle<BufferAttachmentList> list;
        Handle<BufferAttachment>     next;
        Handle<BufferAttachment>     prev;
        Handle<BufferView>           view;
    };

    namespace Vulkan
    {
        class ICommandList;
    } // namespace Vulkan

    struct PassCreateInfo
    {
        const char* name;
        QueueType   queueType;
    };

    struct Pass
    {
        TL::String      name;
        QueueType       queueType;
        ImageSize2D     renderTargetSize;
        PassSubmitData* submitData;

        TL::Vector<Handle<ImageAttachment>> colorAttachments;
        Handle<ImageAttachment>             depthStencilAttachment;

        TL::Vector<Handle<ImageAttachment>>  imageAttachments;
        TL::Vector<Handle<BufferAttachment>> bufferAttachments;

        TL::Vector<const CommandList*> commandList;
    };

    class RHI_EXPORT RenderGraph final
    {
        friend class Vulkan::ICommandList;

    public:
        RenderGraph(Context* context);
        RenderGraph(const RenderGraph&) = delete;
        RenderGraph(RenderGraph&&)      = delete;
        ~RenderGraph()                  = default;

        RHI_NODISCARD Handle<Pass> CreatePass(const PassCreateInfo& createInfo);

        RHI_NODISCARD Handle<ImageAttachment> CreateImage(const ImageCreateInfo& createInfo);

        RHI_NODISCARD Handle<BufferAttachment> CreateBuffer(const BufferCreateInfo& createInfo);

        RHI_NODISCARD Handle<ImageAttachment> ImportSwapchain(const char* name, Swapchain& swapchain);

        RHI_NODISCARD Handle<ImageAttachment> ImportImage(const char* name, Handle<Image> image);

        RHI_NODISCARD Handle<BufferAttachment> ImportBuffer(const char* name, Handle<Buffer> buffer);

        Handle<ImageAttachment> UseImage(Handle<Pass> pass, Handle<ImageAttachment> attachment, const ImageAttachmentUseInfo& useInfo);

        Handle<BufferAttachment> UseBuffer(Handle<Pass> pass, Handle<BufferAttachment> attachment, const BufferAttachmentUseInfo& useInfo);

        void SubmitCommands(Handle<Pass> pass, TL::Span<const CommandList* const> commandLists);

        RHI_NODISCARD Handle<Image> GetImage(Handle<ImageAttachment> attachment);

        RHI_NODISCARD Handle<Buffer> GetBuffer(Handle<BufferAttachment> attachment);

        RHI_NODISCARD Handle<ImageView> GetImageView(Handle<ImageAttachment> attachment);

        RHI_NODISCARD Handle<BufferView> GetBufferView(Handle<BufferAttachment> attachment);

        RHI_NODISCARD Swapchain* GetSwapchain(Handle<ImageAttachment> attachment);

        // private:

        ImageAttachmentList* GetAttachmentList(Handle<ImageAttachment> attachment);
        ImageAttachment*     GetAttachment(Handle<ImageAttachment> attachment);
        ImageAttachment*     GetAttachmentNext(Handle<ImageAttachment> attachment);
        ImageAttachment*     GetAttachmentPrev(Handle<ImageAttachment> attachment);

        BufferAttachmentList* GetAttachmentList(Handle<BufferAttachment> attachment);
        BufferAttachment*     GetAttachment(Handle<BufferAttachment> attachment);
        BufferAttachment*     GetAttachmentNext(Handle<BufferAttachment> attachment);
        BufferAttachment*     GetAttachmentPrev(Handle<BufferAttachment> attachment);

        // private:

        Context* m_context;

        TL::Vector<Handle<Pass>>                 m_passes;
        TL::Vector<Handle<ImageAttachmentList>>  m_graphImageAttachments;
        TL::Vector<Handle<BufferAttachmentList>> m_graphBufferAttachments;

        HandlePool<Pass>             m_passOwner;
        HandlePool<ImageAttachment>  m_imageAttachmentOwner;
        HandlePool<BufferAttachment> m_bufferAttachmentOwner;

        HandlePool<ImageAttachmentList>  m_graphImageAttachmentOwner;
        HandlePool<BufferAttachmentList> m_graphBufferAttachmentOwner;
    };

} // namespace RHI

namespace RHI
{

    inline ImageAttachmentList* RenderGraph::GetAttachmentList(Handle<ImageAttachment> attachmentHandle)
    {
        if (attachmentHandle == NullHandle)
            return nullptr;

        auto attachment = m_imageAttachmentOwner.Get(attachmentHandle);
        return m_graphImageAttachmentOwner.Get(attachment->list);
    }

    inline ImageAttachment* RenderGraph::GetAttachment(Handle<ImageAttachment> attachmentHandle)
    {
        if (attachmentHandle == NullHandle)
            return nullptr;

        return m_imageAttachmentOwner.Get(attachmentHandle);
    }

    inline ImageAttachment* RenderGraph::GetAttachmentNext(Handle<ImageAttachment> attachmentHandle)
    {
        if (attachmentHandle == NullHandle)
            return nullptr;

        auto attachment = m_imageAttachmentOwner.Get(attachmentHandle);
        return m_imageAttachmentOwner.Get(attachment->next);
    }

    inline ImageAttachment* RenderGraph::GetAttachmentPrev(Handle<ImageAttachment> attachmentHandle)
    {
        if (attachmentHandle == NullHandle)
            return nullptr;

        auto attachment = m_imageAttachmentOwner.Get(attachmentHandle);
        return m_imageAttachmentOwner.Get(attachment->prev);
    }

    inline BufferAttachmentList* RenderGraph::GetAttachmentList(Handle<BufferAttachment> attachmentHandle)
    {
        if (attachmentHandle == NullHandle)
            return nullptr;

        auto attachment = m_bufferAttachmentOwner.Get(attachmentHandle);
        return m_graphBufferAttachmentOwner.Get(attachment->list);
    }

    inline BufferAttachment* RenderGraph::GetAttachment(Handle<BufferAttachment> attachmentHandle)
    {
        if (attachmentHandle == NullHandle)
            return nullptr;

        return m_bufferAttachmentOwner.Get(attachmentHandle);
    }

    inline BufferAttachment* RenderGraph::GetAttachmentNext(Handle<BufferAttachment> attachmentHandle)
    {
        if (attachmentHandle == NullHandle)
            return nullptr;

        auto attachment = m_bufferAttachmentOwner.Get(attachmentHandle);
        return m_bufferAttachmentOwner.Get(attachment->next);
    }

    inline BufferAttachment* RenderGraph::GetAttachmentPrev(Handle<BufferAttachment> attachmentHandle)
    {
        if (attachmentHandle == NullHandle)
            return nullptr;

        auto attachment = m_bufferAttachmentOwner.Get(attachmentHandle);
        return m_bufferAttachmentOwner.Get(attachment->prev);
    }

} // namespace RHI
