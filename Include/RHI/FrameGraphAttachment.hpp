#pragma once
#include "RHI/Resource.hpp"
#include "RHI/TypeTraits.hpp"

namespace RHI
{

template <typename T>
class PassAttachment;

enum class EAttachmentUsage
{
    RenderTarget,
    ShaderInput,
    DepthStencil,
    InputAttachment,
};

enum class EAttachmentAccess
{
    Read      = 0x1,
    Write     = 0x2,
    ReadWrite = 0x1 | 0x2 // = 0x3
};

enum class EAttachmentResourceType
{
    Swapchain,
    Image,
    Buffer,
    TexelBuffer
};

enum class EAttachmentLoadOp
{
    Load,
    Discard,
    DontCare,
};

enum class EAttachmentStoreOp
{
    Store,
    Discard,
    DontCare,
};

template <EAttachmentResourceType EType>
class AttachmentReference
{
public:
    AttachmentReference()
        : m_index(UINT32_MAX)
    {
    }
    
    AttachmentReference(uint32_t index)
        : m_index(index)
    {
    }

    AttachmentReference operator=(uint32_t index)
    {
        m_index = index;
    }

    operator uint32_t()
    {
        return m_index;
    }

private:
    uint32_t m_index;
};
using ImageAttachmentReference  = AttachmentReference<EAttachmentResourceType::Image>;
using BufferAttachmentReference = AttachmentReference<EAttachmentResourceType::Buffer>;

struct AttachmentLoadStoreOp
{
    EAttachmentLoadOp  loadOp;
    EAttachmentStoreOp storeOp;
};

struct ImageFrameAttachmentDesc
{
    std::string name;
    ImageDesc   imageDesc;
};

struct BufferFrameAttachmentDesc
{
    std::string name;
    BufferDesc  bufferDesc;
};

template <typename Resource>
class FrameAttachment
{
    friend class IFrameGraph;

public:
    using ResourceView = ConvertResourceToView<Resource>;
    using ResourceDesc = ConvertResourceToDesc<Resource>;

    FrameAttachment(std::string name, Unique<Resource> resource, const ResourceDesc& resourceDesc)
        : m_name(std::move(name))
        , m_resource(std::move(resource))
        , m_desc(CreateUnique<const ResourceDesc>())
        , m_pFirstUse(nullptr)
        , m_pLastUse(nullptr)
    {
        *m_desc = resourceDesc;
    }

    inline std::string_view GetName() const
    {
        return m_name;
    }

    inline Resource& GetResource()
    {
        return *m_resource;
    }

    inline const Resource& GetResource() const
    {
        return *m_resource;
    }

    inline const PassAttachment<ResourceView>& GetFirstUse() const
    {
        return *m_pFirstUse;
    }

    inline PassAttachment<ResourceView>& GetFirstUse()
    {
        return *m_pFirstUse;
    }

    inline const PassAttachment<ResourceView>& GetLastUse() const
    {
        return *m_pLastUse;
    }

    inline PassAttachment<ResourceView>& GetLastUse()
    {
        return *m_pLastUse;
    }

    inline const ResourceDesc& GetDesc() const
    {
        return *m_desc;
    }

protected:
    inline void SetFirstUse(const PassAttachment<ResourceView>& firstUse)
    {
        m_pFirstUse = &firstUse;
    }

    inline void SetLastUse(const PassAttachment<ResourceView>& lastUse)
    {
        m_pLastUse = &lastUse;
    }

protected:
    const std::string             m_name;
    Unique<Resource>              m_resource;
    Unique<const ResourceDesc>    m_desc;
    PassAttachment<ResourceView>* m_pFirstUse;
    PassAttachment<ResourceView>* m_pLastUse;
};
using ImageFrameAttachment  = FrameAttachment<IImage>;
using BufferFrameAttachment = FrameAttachment<IBuffer>;

struct ImagePassAttachmentDesc
{
    ImageAttachmentReference attachmentReference;
    AttachmentLoadStoreOp    loadStoreOps;
    ImageViewDesc            attachmentViewDesc;
};

struct BufferPassAttachmentDesc
{
    std::string    attachmentReference;
    BufferViewDesc attachmentViewDesc;
};

namespace Internal
{
    class EmptyPassAttachmentBase
    {
    };

    class ImagePassAttachmentBase
    {
    public:
        ImagePassAttachmentBase(AttachmentLoadStoreOp loadStoreOp, ESampleCount sampleCount)
            : m_loadStoreOps(loadStoreOp)
            , m_sampleCount(sampleCount)
        {
        }

        inline AttachmentLoadStoreOp GetLoadStoreOp() const
        {
            return m_loadStoreOps;
        }
        inline ESampleCount GetSampleCount() const
        {
            return m_sampleCount;
        }
    
    private:
        AttachmentLoadStoreOp m_loadStoreOps;
        ESampleCount          m_sampleCount;
    };
} // namespace Internal

template <typename ResourceView>
class PassAttachment final
    : public std::conditional_t<std::is_same_v<ResourceView, IImageView>, Internal::ImagePassAttachmentBase, Internal::EmptyPassAttachmentBase>
{
    friend class IFrameGraph;

public:
    using ResourceType     = ConvertViewToResource<ResourceView>;
    using ResourceViewDesc = ConvertViewToDesc<ResourceView>;
    using FrameAttachment  = FrameAttachment<ResourceType>;
    
    PassAttachment(const FrameAttachment& frameAttachment, Unique<ResourceView> view, const ResourceViewDesc& viewDesc, EAccess access, EAttachmentUsage usage)
        : m_frameAttachment(FrameAttachment)
        , m_access(access)
        , m_usage(usage)
        , m_view(std::move(view))
        , m_desc(CreateUnique<const ResourceViewDesc>())
        , m_pNext(nullptr)
        , m_pPrev(nullptr)
    {
        *m_desc = viewDesc;
    }
    
    PassAttachment(const FrameAttachment& frameAttachment, Unique<ResourceView> view, const ResourceViewDesc& viewDesc, EAccess access, EAttachmentUsage usage, EAttachmentLoadOp loadStoreOp, ESampleCount sampleCount)
        : Internal::ImagePassAttachmentBase(loadStoreOp, sampleCount)
        , m_frameAttachment(FrameAttachment)
        , m_access(access)
        , m_usage(usage)
        , m_view(std::move(view))
        , m_desc(CreateUnique<const ResourceViewDesc>())
        , m_pNext(nullptr)
        , m_pPrev(nullptr)
    {
        *m_desc = viewDesc;
    }

    inline ResourceView& GetView()
    {
        return *m_view;
    }

    inline const ResourceView& GetView() const
    {
        return *m_view;
    }

    inline EAttachmentAccess GetAccess() const
    {
        return m_access;
    }

    inline EAttachmentUsage GetUsage() const
    {
        return m_usage;
    }

    inline const FrameAttachment& GetFrameAttachment() const
    {
        return *m_frameAttachment;
    }

    inline FrameAttachment& GetFrameAttachment()
    {
        return *m_frameAttachment;
    }

    inline const PassAttachment* GetNext() const
    {
        return m_pNext;
    }

    inline PassAttachment* GetNext()
    {
        return m_pNext;
    }
    
    inline const PassAttachment* GetPerv() const
    {
        return m_pPrev;
    }

    inline PassAttachment* GetPerv()
    {
        return m_pPrev;
    }

    inline const auto& GetDesc() const
    {
        return *m_desc;
    }

protected:
    inline void SetNext(const PassAttachment& nextPassAttachment)
    {
        m_pNext = &nextPassAttachment;
    }

    inline void SetPrev(const PassAttachment& prevPassAttachment)
    {
        m_pPrev = &prevPassAttachment;
    }

protected:
    FrameAttachment*         m_frameAttachment;
    EAttachmentAccess        m_access;
    EAttachmentUsage         m_usage;
    Unique<ResourceView>     m_view;
    Unique<ResourceViewDesc> m_desc;
    PassAttachment*          m_pNext;
    PassAttachment*          m_pPrev;
};

using BufferPassAttachment = PassAttachment<IBufferView>;
<<<<<<< HEAD
using ImagePassAttachment  = PassAttachment<IImageView>;
=======

class ImagePassAttachment final : public PassAttachment<IImageView>
{
public:
    inline AttachmentLoadStoreOp GetLoadStoreOp() const
    {
        return m_loadStoreOps;
    }

    inline ESampleCount GetSampleCount() const
    {
        return m_sampleCount;
    }

    // TMEP remove later.
    inline ImagePassAttachment* GetNext() const
    {
        return static_cast<ImagePassAttachment*>(m_pNext);
    }

    inline ImagePassAttachment* GetNext()
    {
        return static_cast<ImagePassAttachment*>(m_pNext);
    }

    inline const ImagePassAttachment* GetPerv() const
    {
        return static_cast<const ImagePassAttachment*>(m_pPrev);
    }

    inline ImagePassAttachment* GetPerv()
    {
        return static_cast<ImagePassAttachment*>(m_pPrev);
    }

private:
    AttachmentLoadStoreOp m_loadStoreOps;
    ESampleCount          m_sampleCount;
};
>>>>>>> 49ff0baea8856acd38e8e358c4e24685c7cec3bb

} // namespace RHI