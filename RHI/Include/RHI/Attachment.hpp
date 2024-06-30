#pragma once

#include <RHI/Common/Containers.h>
#include <RHI/Common/Handle.hpp>
#include <RHI/Common/Flags.hpp>
#include <RHI/Definition.hpp>
#include <RHI/Resources.hpp>
#include <RHI/RGInternals.hpp>

#include <cstdint>

namespace RHI
{
    struct Pass;

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

    class Swapchain;
    class EmptyBase { };
    class RHI_EXPORT SwapchainAttachment
    {
    public:
        Swapchain* m_swapchain = nullptr;
    };

    // clang-format on

    template<typename Type>
    class RHI_EXPORT AttachmentImpl : private std::conditional_t<std::is_same_v<Type, Image>, SwapchainAttachment, EmptyBase>
    {
        friend class RenderGraph;

    public:
        struct View;

        inline static constexpr bool _IsImage = std::is_same_v<Type, Image>;

        using UseList          = TL::List<View>;
        using AttachmentIt     = typename UseList::iterator;
        using Resource         = std::conditional_t<_IsImage, RGImageID, RGBufferID>;
        using ResourceView     = std::conditional_t<_IsImage, RGImageViewID, RGBufferViewID>;
        using Desc             = std::conditional_t<_IsImage, ImageCreateInfo, BufferCreateInfo>;
        using ResourceView     = std::conditional_t<_IsImage, RGImageViewID, RGBufferViewID>;
        using ResourceViewInfo = std::conditional_t<_IsImage, ImageViewInfo, BufferViewInfo>;
        using Usage            = std::conditional_t<_IsImage, ImageUsage, BufferUsage>;
        using AttachmentHandle = std::conditional_t<_IsImage, Handle<class ImageAttachment>, Handle<class BufferAttachment>>;

        struct RHI_EXPORT View
        {
            Handle<Pass>       pass;
            AttachmentHandle   attachment;
            AttachmentIt       next;
            AttachmentIt       prev;
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
            , m_useList()
            , m_passToViews()
        {
        }

        AttachmentImpl(TL::String name, Resource resource)
            : m_name(std::move(name))
            , m_lifetime(AttachmentLifetime::Imported)
            , m_description()
            , m_resource(resource)
            , m_useList()
            , m_passToViews()
        {
        }

        AttachmentImpl(TL::String name, Resource resource, Swapchain& swapchain)
            requires requires(Type type) { _IsImage; }
            : m_name(std::move(name))
            , m_lifetime(AttachmentLifetime::Imported)
            , m_description()
            , m_resource(resource)

        {
            SwapchainAttachment::m_swapchain = &swapchain;
        }

        const char* GetName() const { return m_name.c_str(); }

        AttachmentLifetime GetLifetime() const { return m_lifetime; }

        AttachmentIt GetView(Handle<Pass> pass)
        {
            if (auto it = m_passToViews.find(pass); it != m_passToViews.end())
                return *it->second;
            return {};
        }

        Swapchain* GetSwapchain() const
            requires requires(Type type) { _IsImage; }
        {
            return SwapchainAttachment::m_swapchain;
        }

        AttachmentIt begin() { return m_useList.begin(); }

        AttachmentIt end() { return m_useList.end(); }

        void Clear() { m_useList.clear(); }

        AttachmentIt Emplace(
            Handle<Pass>       pass,
            AttachmentHandle   attachmentHandle,
            ResourceViewInfo   viewInfo,
            Usage              usage,
            Access             access,
            Flags<ShaderStage> stages)
        {
            (void)pass;
            (void)attachmentHandle;
            (void)viewInfo;
            (void)usage;
            (void)access;
            (void)stages;
            return {};
        }

        Desc     m_description;
        Resource m_resource;

    private:
        TL::String         m_name;
        AttachmentLifetime m_lifetime;
        UseList            m_useList;

        TL::UnorderedMap<Handle<Pass>, AttachmentIt*> m_passToViews;
    };

    inline static constexpr ImageSize2D SizeRelative2D = ImageSize2D{};

    class RHI_EXPORT ImageAttachment : public AttachmentImpl<Image>
    {
    public:
        using AttachmentImpl<Image>::AttachmentImpl;
    };

    class RHI_EXPORT BufferAttachment : public AttachmentImpl<Buffer>
    {
    public:
        using AttachmentImpl<Buffer>::AttachmentImpl;
    };

    using ImagePassAttachment  = ImageAttachment::AttachmentIt;
    using BufferPassAttachment = BufferAttachment::AttachmentIt;

} // namespace RHI