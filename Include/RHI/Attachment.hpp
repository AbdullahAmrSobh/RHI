#pragma once
#include <utility>

#include "RHI/ObjectCache.hpp"
#include "RHI/Swapchain.hpp"

namespace RHI
{
//
class IRenderPass;
class ISwapchain;
class UsedImageAttachment;

struct DepthStencilClearValue
{
    float    depth;
    uint32_t stencil;
};

using ColorClearValueFloat = std::array<float, 4>;
using ColorClearValueSint  = std::array<int32_t, 4>;
using ColorClearValueUint  = std::array<uint32_t, 4>;
using ClearValue           = std::variant<ColorClearValueFloat, ColorClearValueSint, ColorClearValueUint, DepthStencilClearValue>;

struct TransientImageAttachmentDesc
{
    std::string name;
    ImageDesc   imageDesc;
    ClearValue  clearValue;
};

class ImageAttachment
{
public:
    using iterator = std::list<UsedImageAttachment>::iterator;

    ImageAttachment() = default;

    ImageAttachment(std::string name, const ImageDesc& resourceDesc)
        : m_name(std::move(name))
        , m_swapchain(nullptr)
        , m_resource(nullptr)
        , m_resourceDesc(resourceDesc)
    {
    }

    ImageAttachment(std::string name, ISwapchain& swapchain)
        : m_name(std::move(name))
        , m_swapchain(&swapchain)
        , m_resource(&swapchain.GetCurrentImage())
        , m_resourceDesc(swapchain.GetImageDescription())
    {
    }

    ImageAttachment(std::string name, IImage& image)
        : m_name(std::move(name))
        , m_swapchain(nullptr)
        , m_resource(&image)
        , m_resourceDesc(image.GetDescription())
    {
    }

    size_t GetHash() const
    {
        assert(m_resource);
        return std::bit_cast<size_t>(m_resource);
    }

    std::string_view GetName() const
    {
        return m_name;
    }

    size_t GetSize() const
    {
        return HashDescriptor(m_resourceDesc);
    }

    ISwapchain* GetSwapchain()
    {
        return m_swapchain;
    }

    bool IsSwapchainImage() const
    {
        return m_swapchain;
    }

    IImage& GetResource()
    {
        return *m_resource;
    }

    ImageDesc GetResourceDesc() const
    {
        return m_resourceDesc;
    }

    void SetClearValue(const ClearValue& clearValue)
    {
        m_clearValue = clearValue;
    }

    ClearValue GetClearValue() const
    {
        return m_clearValue;
    }

    inline const IRenderPass* GetFirstUser() const;
    inline const IRenderPass* GetLastUser() const;

    UsedImageAttachment& GetFirstUse()
    {
        return m_usedInstances.front();
    }

    UsedImageAttachment& GetLastUse()
    {
        return m_usedInstances.back();
    }

    iterator begin()
    {
        return m_usedInstances.begin();
    }

    iterator end()
    {
        return m_usedInstances.end();
    }

    const std::list<UsedImageAttachment>& GetUseList() const
    {
        return m_usedInstances;
    }

    std::list<UsedImageAttachment>& GetUseList()
    {
        return m_usedInstances;
    }


    UsedImageAttachment* Use(UsedImageAttachment usedAttachment);

private:
    std::string m_name;

    ISwapchain* m_swapchain;

    IImage* m_resource;

    ImageDesc m_resourceDesc;

    ClearValue m_clearValue;

    std::list<UsedImageAttachment> m_usedInstances;
};

enum class AttachmentUsage
{
    Uninitalized,
    RenderTarget,
    DepthStencil,
    Input,
    ShaderResource,
};

enum class AttachmentAccess
{
    Read,
    Write,
    ReadWrite,
};

enum class AttachmentLoadOperation
{
    Uninitialized,
    DontCare,
    Load,
    Clear,
};

enum class AttachmentStoreOperation
{
    Uninitialized,
    DontCare,
    Store,
    Discard,
};

struct AttachmentLoadStoreOperations
{
    AttachmentLoadStoreOperations() = default;

    AttachmentLoadOperation  loadOperation  = AttachmentLoadOperation::Load;
    AttachmentStoreOperation storeOperation = AttachmentStoreOperation::Store;
};

struct AttachmentBlendState
{
};

class UsedImageAttachment
{
public:
    UsedImageAttachment(const IRenderPass&            renderpass,
                        ImageAttachment&              attachment,
                        const ImageViewDesc&          viewDesc,
                        AttachmentLoadStoreOperations loadStoreOperations,
                        AttachmentUsage               usage,
                        AttachmentAccess              access)
        : m_renderpass(&renderpass)
        , m_attachment(&attachment)
        , m_view(nullptr)
        , m_viewDesc(viewDesc)
        , m_loadStoreOperations(loadStoreOperations)
        , m_usage(usage)
        , m_access(access)
    {
    }

    const IRenderPass& GetRenderPass() const
    {
        return *m_renderpass;
    }

    const ImageAttachment& GetAttachment() const
    {
        return *m_attachment;
    }

    ImageAttachment& GetAttachment()
    {
        return *m_attachment;
    }

    size_t GetViewHash() const
    {
        return HashCombine(HashDescriptor(m_viewDesc), m_attachment->GetHash());
    }

    IImageView& GetView()
    {
        return *m_view;
    }

    ImageViewDesc GetViewDesc() const
    {
        return m_viewDesc;
    }

    AttachmentLoadStoreOperations GetLoadStoreOperations() const
    {
        return m_loadStoreOperations;
    }

    AttachmentUsage GetUsage() const
    {
        return m_usage;
    }

    AttachmentAccess GetAccess() const
    {
        return m_access;
    }

    const UsedImageAttachment* GetPreviousUse() const
    {
        return nullptr;
    }

    const UsedImageAttachment* GetNextUse() const
    {
        return nullptr;
    }

private:
    friend class IFrameScheduler;

    void SetView(std::shared_ptr<IImageView> view)
    {
        m_view = view;
    }

private:
    const IRenderPass* m_renderpass;

    ImageAttachment*          m_attachment;
    ImageAttachment::iterator m_position;

    std::shared_ptr<IImageView> m_view;

    ImageViewDesc m_viewDesc;

    AttachmentLoadStoreOperations m_loadStoreOperations;
    AttachmentUsage               m_usage;
    AttachmentAccess              m_access;
};

inline UsedImageAttachment* ImageAttachment::Use(UsedImageAttachment usedAttachment)
{
    return &(*m_usedInstances.insert(m_usedInstances.end(), std::move(usedAttachment)));
}

inline const IRenderPass* ImageAttachment::GetFirstUser() const
{
    if (m_usedInstances.empty())
        return nullptr;

    return &(m_usedInstances.front().GetRenderPass());
}

inline const IRenderPass* ImageAttachment::GetLastUser() const
{
    if (m_usedInstances.empty())
        return nullptr;

    return &(m_usedInstances.back().GetRenderPass());
}

}  // namespace RHI