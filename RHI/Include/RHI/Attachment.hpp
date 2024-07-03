#pragma once

#include <RHI/Common/Handle.hpp>
#include <RHI/Common/Flags.hpp>
#include <RHI/Common/Ptr.hpp>
#include <RHI/Common/Containers.h>
#include <RHI/Definitions.hpp>
#include <RHI/Resources.hpp>
#include <RHI/RGInternals.hpp>

#include <cstdint>

namespace RHI
{
    class Pass;

    enum class AttachmentLifetime : uint8_t
    {
        None,
        Imported,
        Transient,
    };

    struct ImageViewInfo
    {
        ImageViewType         viewType;
        ImageSubresourceRange subresourceLayers;
        ComponentMapping      componentMapping;
        LoadStoreOperations   loadStoreOperations;
    };

    struct BufferViewInfo
    {
        BufferSubregion subregion;
        Format          format;
    };

    // clang-format off
    // clang-format on

    template<typename Type>
    class AttachmentImpl
    {
        friend class RenderGraph;

        inline static constexpr bool _IsImage = std::is_same_v<Type, Image>;

    public:
        struct View;

        using Resource         = std::conditional_t<_IsImage, RGImageID, RGBufferID>;
        using ResourceView     = std::conditional_t<_IsImage, RGImageViewID, RGBufferViewID>;
        using Desc             = std::conditional_t<_IsImage, ImageCreateInfo, BufferCreateInfo>;
        using ResourceView     = std::conditional_t<_IsImage, RGImageViewID, RGBufferViewID>;
        using ResourceViewInfo = std::conditional_t<_IsImage, ImageViewInfo, BufferViewInfo>;
        using Usage            = std::conditional_t<_IsImage, ImageUsage, BufferUsage>;
        using AttachmentHandle = std::conditional_t<_IsImage, Handle<class ImageAttachment>, Handle<class BufferAttachment>>;

        struct View
        {
            Handle<Pass>       pass;
            AttachmentHandle   attachment;
            View*              next;
            View*              prev;
            Usage              usage;
            Access             access;
            Flags<ShaderStage> shaderStages;
            ResourceView       view;
            ResourceViewInfo   viewInfo;
        };

        AttachmentImpl(TL::String name, const Desc& description)
            : m_name(std::move(name))
            , m_lifetime(AttachmentLifetime::Transient)
            , m_description(description)
            , m_resource()
            , m_begin(nullptr)
            , m_end(nullptr)
            , m_passToViews()
        {
        }

        AttachmentImpl(TL::String name, Resource resource)
            : m_name(std::move(name))
            , m_lifetime(AttachmentLifetime::Imported)
            , m_description()
            , m_resource(resource)
            , m_begin(nullptr)
            , m_end(nullptr)
            , m_passToViews()
        {
        }

        View* GetView(Handle<Pass> pass)
        {
            return m_passToViews.find(pass)->second;
        }

        void Clear()
        {
            m_passToViews.clear();
        }

        View* Emplace(Handle<Pass> pass, AttachmentHandle attachmentHandle, ResourceViewInfo viewInfo, Usage usage, Access access, Flags<ShaderStage> stages)
        {
            m_passToViews[pass] = new View();

            auto it          = m_passToViews[pass];
            it->pass         = pass;
            it->attachment   = attachmentHandle;
            it->next         = nullptr;
            it->prev         = m_end;
            it->usage        = usage;
            it->access       = access;
            it->shaderStages = stages;
            it->view         = NullHandle;
            it->viewInfo     = viewInfo;

            if (m_begin == nullptr)
                m_begin = it;

            m_end = it;

            return it;
        }

        TL::String         m_name;
        AttachmentLifetime m_lifetime;
        Desc               m_description;
        Resource           m_resource;
        View*              m_begin;
        View*              m_end;

    private:
        TL::UnorderedMap<Handle<Pass>, View*> m_passToViews;
    };

    inline static constexpr ImageSize2D SizeRelative2D = ImageSize2D{};

    class RHI_EXPORT ImageAttachment : public AttachmentImpl<Image>
    {
        using Base = AttachmentImpl<Image>;

        Swapchain* m_swapchain;

    public:
        ImageAttachment(TL::String name, const Desc& description)
            : Base(name, description)
            , m_swapchain(nullptr)
        {
        }

        ImageAttachment(TL::String name, Resource resource)
            : Base(name, resource)
            , m_swapchain(nullptr)
        {
        }

        ImageAttachment(TL::String name, Resource resource, Swapchain& swapchain)
            : Base(name, resource)
            , m_swapchain(&swapchain)
        {
        }

        Swapchain* GetSwapchain() const
        {
            return m_swapchain;
        }
    };

    class RHI_EXPORT BufferAttachment : public AttachmentImpl<Buffer>
    {
        using Base = AttachmentImpl<Buffer>;

    public:
        BufferAttachment(TL::String name, const Desc& description)
            : Base(name, description)
        {
        }

        BufferAttachment(TL::String name, Resource resource)
            : Base(name, resource)
        {
        }
    };

    using ImagePassAttachment  = ImageAttachment::View*;
    using BufferPassAttachment = BufferAttachment::View*;

} // namespace RHI