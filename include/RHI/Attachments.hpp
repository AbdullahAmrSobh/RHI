#pragma once

#include <string>

#include "RHI/Resources.hpp"

namespace RHI
{

class PassState;
class PassAttachment;
class ImagePassAttachment;
class BufferPassAttachment;

enum class ResourceType
{
    Image,
    Buffer,
};

enum class AttachmentUsage
{
    None,
    RenderTarget,
    DepthStencil,
    ShaderResource,
};

enum class AttachmentAccess
{
    None,
    Read,
    Write,
    ReadWrite,
};

enum class AttachmentLifetime
{
    Persistent,
    Transient,
};

enum class ImageAttachmentLoadOperation
{
    DontCare,
    Load,
    Discard,
};

enum class ImageAttachmentStoreOperation
{
    DontCare,
    Store,
    Discard,
};

enum class PassQueue
{
    Graphics,
    Compute,
};

struct ImageAttachmentLoadStoreOperations
{
    ImageAttachmentLoadOperation  loadOperation  = ImageAttachmentLoadOperation::Load;
    ImageAttachmentStoreOperation storeOperation = ImageAttachmentStoreOperation::Store;
};

struct ImageAttachmentClearValue
{
};

struct TransientImageCreateInfo
{
    std::string name;
};

struct TransientBufferCreateInfo
{
    std::string name;
};

struct ImageAttachmentUseInfo
{
    std::string_view                   name;
    ImageViewCreateInfo                viewInfo;
    ImageAttachmentLoadStoreOperations loadStoreOperations;
};

struct BufferAttachmentUseInfo
{
    std::string_view     name;
    BufferViewCreateInfo viewInfo;
};

class Attachment
{
public:
    Attachment(const Attachment&) = delete;

    Attachment(std::string name, const TransientImageCreateInfo createInfo);

    Attachment(std::string name, const TransientBufferCreateInfo createInfo);

    Attachment(std::string name, std::unique_ptr<Image> image)
        : m_name(std::move(name))
        , m_lifetime(AttachmentLifetime::Persistent)
        , m_type(ResourceType::Image)
        , m_asImage(std::move(image))
    {
    }

    Attachment(std::string name, std::unique_ptr<Buffer> buffer)
        : m_name(std::move(name))
        , m_lifetime(AttachmentLifetime::Persistent)
        , m_type(ResourceType::Buffer)
        , m_asBuffer(std::move(buffer))
    {
    }

    const std::string& GetName() const
    {
        return m_name;
    }

    ResourceType GetResourceType() const
    {
        return m_type;
    }

    AttachmentLifetime GetLifetimeType() const
    {
        return m_lifetime;
    }

    bool IsInitialized() const
    {
        if (m_type == ResourceType::Image)
            return m_asImage != nullptr;
        else
            return m_asBuffer != nullptr;
    }

protected:
    const std::string m_name;

    AttachmentLifetime m_lifetime;

    ResourceType m_type;

    union
    {
        std::unique_ptr<Image>  m_asImage;
        std::unique_ptr<Buffer> m_asBuffer;
    };

    std::list<PassAttachment> m_uses;
};

class ImageAttachment final : public Attachment
{
public:
    ImageAttachment(std::string name, const TransientImageCreateInfo createInfo);

    ImageAttachment(std::string name, std::unique_ptr<Image> image);
    void SetImage(std::unique_ptr<Image> image);

    const Image& GetImage() const;
    Image&       GetImage();

    const ImagePassAttachment* GetFirstUse() const;
    ImagePassAttachment*       GetFirstUse();

    const ImagePassAttachment* GetLastUse() const;
    ImagePassAttachment*       GetLastUse();

    const ImageCreateInfo& GetInfo() const;

    bool IsSwapchain() const;

    const Swapchain* GetSwapchain() const;
    Swapchain*       GetSwapchain();

private:
    ImageCreateInfo           m_info;
    ImageAttachmentClearValue m_clearValue;
    Swapchain*                m_swapchain;
};

class BufferAttachment final : public Attachment
{
public:
    BufferAttachment(std::string name, const TransientBufferCreateInfo createInfo);

    BufferAttachment(std::string name, std::unique_ptr<Buffer> buffer);

    void SetBuffer(std::unique_ptr<Buffer> buffer);

    const Buffer& GetBuffer() const;
    Buffer&       GetBuffer();

    const BufferPassAttachment* GetFirstUse() const;
    BufferPassAttachment*       GetFirstUse();

    const BufferPassAttachment* GetLastUse() const;
    BufferPassAttachment*       GetLastUse();

    const BufferCreateInfo& GetInfo() const;

private:
    BufferCreateInfo m_info;
};

class PassAttachment
{
public:
    PassAttachment();

    bool IsInitialized() const;

private:
    Attachment* m_attachment;

    PassState* m_pass;

    PassAttachment* m_nextUse;
    PassAttachment* m_pervUse;

    union
    {
        std::unique_ptr<ImageView>  m_asImageView;
        std::unique_ptr<BufferView> m_asBufferView;
    };

    AttachmentUsage  m_usage;
    AttachmentAccess m_access;
};

class ImagePassAttachment final : public PassAttachment
{
public:
    ImagePassAttachment();

    ImageAttachment& GetAttachment();

    const ImagePassAttachment* GetPervUse() const;
    ImagePassAttachment*       GetPervUse();

    const ImagePassAttachment* GetNextUse() const;
    ImagePassAttachment*       GetNextUse();

    const ImageView& GetImagView() const;
    ImageView&       GetImagView();

    const ImageViewCreateInfo& GetImageInfo();

    const Swapchain* GetSwapchain();

    ImageAttachmentLoadStoreOperations GetLoadStoreoperations() const;

private:
    ImageViewCreateInfo m_info;
};

class BufferPassAttachment final : public PassAttachment
{
public:
    BufferPassAttachment();

    const BufferAttachment& GetAttachment() const;
    BufferAttachment&       GetAttachment();

    const BufferPassAttachment* GetPervUse() const;
    BufferPassAttachment*       GetPervUse();

    const BufferPassAttachment* GetNextUse() const;
    BufferPassAttachment*       GetNextUse();

    const BufferView& GetBuffeView() const;
    BufferView&       GetBuffeView();

    const BufferViewCreateInfo& GetBufferInfo();

private:
    BufferViewCreateInfo m_info;
};

class AttachmentsRegistry
{
public:
    AttachmentsRegistry();

    void Reset();

    void ImportImage(std::string name, Image& image);

    void ImportBuffer(std::string name, Buffer& buffer);

    void ImportSwapchain(std::string name, Swapchain& swapchain);

    void CreateTransientImageAttachment(std::string name, const TransientImageCreateInfo& createInfo);

    void CreateTransientBufferAttachment(std::string name, const TransientBufferCreateInfo& createInfo);

    const std::span<const Attachment*> GetImportedAttachments() const;
    std::span<Attachment*>             GetImportedAttachments();

    const std::span<const ImageAttachment*> GetSwapchainAttachments() const;
    std::span<ImageAttachment*>             GetSwapchainAttachments();

    const std::span<const Attachment*> GetTransientAttachments() const;
    std::span<Attachment*>             GetTransientAttachments();

    const std::span<const ImageAttachment*> GetImageAttachments() const;
    std::span<ImageAttachment*>             GetImageAttachments();

    const std::span<const BufferAttachment*> GetBufferAttachments() const;
    std::span<BufferAttachment*>             GetBufferAttachments();

private:
    std::unordered_map<std::string, ImageAttachment> m_imageAttachmentLookup;

    std::unordered_map<std::string, BufferAttachment> m_bufferAttachmentsLookup;

    std::vector<Attachment*> m_importedAttachments;

    std::vector<ImageAttachment*> m_swapchainAttachments;

    std::vector<Attachment*> m_transientAttachments;

    std::vector<ImageAttachment*> m_imageAttachments;

    std::vector<BufferAttachment*> m_bufferAttachments;
};

}  // namespace RHI