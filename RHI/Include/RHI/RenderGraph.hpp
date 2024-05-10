#pragma once

#include "RHI/Resources.hpp"

#include "RHI/Common/Handle.hpp"
#include "RHI/QueueType.hpp"

#include "RHI/Export.hpp"

namespace RHI
{
    struct Pass;
    class Context;
    class Swapchain;
    class CommandList;

    RHI_DECALRE_OPAQUE_RESOURCE(PassSubmitData);

    enum class AttachmentLifetime
    {
        Persistent,
        Transient,
    };

    enum class LoadOperation
    {
        DontCare, // The attachment load operation undefined.
        Load,     // Load attachment content.
        Discard,  // Discard attachment content.
    };

    enum class StoreOperation
    {
        DontCare, // Attachment Store operation is undefined
        Store,    // Writes to the attachment are stored
        Discard,  // Writes to the attachment are discarded
    };

    struct LoadStoreOperations
    {
        LoadOperation  loadOperation  = LoadOperation::Discard;
        StoreOperation storeOperation = StoreOperation::Store;

        inline bool    operator==(const LoadStoreOperations& other) const
        {
            return loadOperation == other.loadOperation && storeOperation == other.storeOperation;
        }
    };

    struct DepthStencilValue
    {
        float       depthValue   = 1.0f;
        uint8_t     stencilValue = 0xff;

        inline bool operator==(const DepthStencilValue& other) const
        {
            return depthValue == other.depthValue && stencilValue == other.stencilValue;
        }
    };

    template<typename T>
    struct ColorValue
    {
        ColorValue() = default;

        ColorValue(T r = {}, T g = {}, T b = {}, T a = {});

        T           r, g, b, a;

        inline bool operator==(const ColorValue& other) const
        {
            return r == other.r && g == other.g && b == other.b && a == other.a;
        }
    };

    union ClearValue
    {
        ClearValue();

        template<typename T>
        ClearValue(ColorValue<T> value);

        template<typename T>
        ClearValue(T r, T g, T b, T a);

        ClearValue(DepthStencilValue value);

        ColorValue<uint8_t>  u8;
        ColorValue<uint16_t> u16;
        ColorValue<uint32_t> u32;
        ColorValue<float>    f32;
        DepthStencilValue    depthStencil;
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
        TL::String                           name;
        QueueType                            queueType;
        ImageSize2D                          renderTargetSize;
        PassSubmitData*                      submitData;

        TL::Vector<Handle<ImageAttachment>>  colorAttachments;
        Handle<ImageAttachment>              depthStencilAttachment;

        TL::Vector<Handle<ImageAttachment>>  imageAttachments;
        TL::Vector<Handle<BufferAttachment>> bufferAttachments;

        TL::Vector<const CommandList*>       commandList;
    };

    class RHI_EXPORT RenderGraph final
    {
        friend class Vulkan::ICommandList;

    public:
        RenderGraph(Context* context);
        RenderGraph(const RenderGraph&) = delete;
        RenderGraph(RenderGraph&&)      = delete;
        ~RenderGraph()                  = default;

        Handle<Pass>                             CreatePass(const PassCreateInfo& createInfo);

        Handle<ImageAttachment>                  CreateImage(const ImageCreateInfo& createInfo);

        Handle<BufferAttachment>                 CreateBuffer(const BufferCreateInfo& createInfo);

        Handle<ImageAttachment>                  ImportSwapchain(const char* name, Swapchain& swapchain);

        Handle<ImageAttachment>                  ImportImage(const char* name, Handle<Image> image);

        Handle<BufferAttachment>                 ImportBuffer(const char* name, Handle<Buffer> buffer);

        void                                     UseImage(Handle<Pass> pass, Handle<ImageAttachment> attachment, const ImageAttachmentUseInfo& useInfo);

        void                                     UseBuffer(Handle<Pass> pass, Handle<BufferAttachment> attachment, const BufferAttachmentUseInfo& useInfo);

        void                                     SubmitCommands(Handle<Pass> pass, TL::Span<const CommandList* const> commandLists);

        Handle<Image>                            GetImage(Handle<ImageAttachment> attachment);

        Handle<Buffer>                           GetBuffer(Handle<BufferAttachment> attachment);

        Handle<ImageView>                        GetImageView(Handle<ImageAttachment> attachment);

        Handle<BufferView>                       GetBufferView(Handle<BufferAttachment> attachment);

        Swapchain*                               GetSwapchain(Handle<ImageAttachment> attachment);

        // private:

        ImageAttachmentList*                     GetAttachmentList(Handle<ImageAttachment> attachment);
        ImageAttachment*                         GetAttachment(Handle<ImageAttachment> attachment);
        ImageAttachment*                         GetAttachmentNext(Handle<ImageAttachment> attachment);
        ImageAttachment*                         GetAttachmentPrev(Handle<ImageAttachment> attachment);

        BufferAttachmentList*                    GetAttachmentList(Handle<BufferAttachment> attachment);
        BufferAttachment*                        GetAttachment(Handle<BufferAttachment> attachment);
        BufferAttachment*                        GetAttachmentNext(Handle<BufferAttachment> attachment);
        BufferAttachment*                        GetAttachmentPrev(Handle<BufferAttachment> attachment);

        // private:

        Context* m_context;

        TL::Vector<Handle<Pass>>                 m_passes;
        TL::Vector<Handle<ImageAttachmentList>>  m_graphImageAttachments;
        TL::Vector<Handle<BufferAttachmentList>> m_graphBufferAttachments;

        HandlePool<Pass>                         m_passOwner;
        HandlePool<ImageAttachment>              m_imageAttachmentOwner;
        HandlePool<BufferAttachment>             m_bufferAttachmentOwner;

        HandlePool<ImageAttachmentList>          m_graphImageAttachmentOwner;
        HandlePool<BufferAttachmentList>         m_graphBufferAttachmentOwner;
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

namespace RHI
{
    template<typename T>
    inline ColorValue<T>::ColorValue(T r, T g, T b, T a)
        : r(r)
        , g(g)
        , b(b)
        , a(a)
    {
    }

    inline ClearValue::ClearValue()
    {
        f32.r = 0;
        f32.g = 0;
        f32.b = 0;
        f32.a = 1;
    }

    template<typename T>
    inline ClearValue::ClearValue(ColorValue<T> value)
    {
        static_assert(std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, float>, "invalid T argument");

        if constexpr (std::is_same_v<T, uint8_t>)
            u8 = value;
        else if constexpr (std::is_same_v<T, uint16_t>)
            u16 = value;
        else if constexpr (std::is_same_v<T, uint32_t>)
            u32 = value;
        else if constexpr (std::is_same_v<T, float>)
            f32 = value;
    }

    template<typename T>
    inline ClearValue::ClearValue(T r, T g, T b, T a)
    {
        static_assert(std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, float>, "invalid T argument");

        if constexpr (std::is_same_v<T, uint8_t>)
        {
            u8.r = r;
            u8.g = g;
            u8.b = b;
            u8.a = a;
        }
        else if constexpr (std::is_same_v<T, uint16_t>)
        {
            u16.r = r;
            u16.g = g;
            u16.b = b;
            u16.a = a;
        }
        else if constexpr (std::is_same_v<T, uint32_t>)
        {
            u32.r = r;
            u32.g = g;
            u32.b = b;
            u32.a = a;
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            f32.r = r;
            f32.g = g;
            f32.b = b;
            f32.a = a;
        }
    }

    inline ClearValue::ClearValue(DepthStencilValue value)
        : depthStencil(value)
    {
    }
} // namespace RHI