#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

class IFence;
class ISwapChain;
class IBuffer;
class ITextureView;
class ICommandContext;

enum class EPassTaskClass
{
    Undefined = 0,
    Present   = 1,
    Graphics  = 2,
    Compute   = 3,
    Copy      = 4,
};

enum class EAttachmentLoadOp
{
    Clear,
    Load,
    DontCare,
};

enum class EAttachmentStoreOp
{
    Store,
    DontCare,
};

enum class EAttachmentInitialState
{
    Optimal,
    Undefined,
};

struct BufferAttachmentDesc
{
    explicit BufferAttachmentDesc(size_t size, EBufferUsageFlagBits usage)
        : size(size)
        , usage(usage)
    {
    }

    size_t               size;
    EBufferUsageFlagBits usage;
};

struct TextureAttachmentDesc
{
    explicit TextureAttachmentDesc(Extent2D extent, EPixelFormat format, ETextureUsageFlagBits usage)
        : extent(extent)
        , format(format)
        , usage(usage)
    {
    }

    Extent2D              extent;
    EPixelFormat          format;
    ETextureUsageFlagBits usage;
};

enum class EPassAttachmentAccess
{
    Invalid   = 0x0,
    Read      = 0x01,
    Write     = 0x02,
    ReadWrite = Read | Write,
};

enum class EPassAttachmentUsage
{
    Color,
    DepthStencil,
    CopySrc,
    CopyDst,
    Present,
};

class PassAttachment
{
public:
    PassAttachment(bool presistent)
        : m_presistentResource(presistent)
    {
    }

    inline bool IsPresistent() const { return m_presistentResource; }

protected:
    bool m_presistentResource;
};

class BufferPassAttachmentDesc final : public PassAttachment
{
public:
    BufferPassAttachmentDesc(size_t size, EBufferFormat format)
        : PassAttachment(false)
        , m_size(size)
        , m_format(format)
    {
    }

    size_t        m_size;
    EBufferFormat m_format;
};

class TexturePassAttachmentDesc final : public PassAttachment
{
public:
    explicit TexturePassAttachmentDesc(Extent3D extent, EPixelFormat format)
        : PassAttachment(false)
        , m_extent(extent)
        , m_format(format)
    {
    }

    Extent3D     m_extent;
    EPixelFormat m_format;
};

// class ResolvePassAttachmentDesc final : public PassAttachment
// {
// public:
//     ResolvePassAttachmentDesc() = default;
// };

class SwapChainAttachmentDesc final : public PassAttachment
{
public:
    SwapChainAttachmentDesc();
};

struct PassId
{
    PassId(uint32_t id = UINT32_MAX)
        : id(id)
    {
    }
    uint32_t id;
};

enum class EAttachmentLifetimeType
{
    Presistent,
    Transient,
};

struct AttachmentId
{
    AttachmentId(uint32_t id, EAttachmentLifetimeType lifetimeType)
        : id(id)
        , lifetime(lifetimeType)
    {
    }

    uint32_t                id;
    EAttachmentLifetimeType lifetime;

    inline bool operator==(const AttachmentId& rhs) const { return id == rhs.id && lifetime == rhs.lifetime; }
    inline bool operator!=(const AttachmentId& rhs) const { return !(*this == rhs); }

    operator uint32_t() const { return id; }
};

using BufferAttachmentId  = AttachmentId;
using TextureAttachmentId = AttachmentId;

class Pass
{
private:
    friend class RenderGraphBuilder;
    Pass(std::string name, EPassTaskClass taskClass)
        : m_name(name)
        , m_taskClass(taskClass)
    {
    }

    void SetId(const PassId id) { m_id = id; }

public:
    struct PassBufferAttachment
    {
        BufferAttachmentId    id;
        EPassAttachmentAccess access;
        EPassAttachmentUsage  usage;
    };

    struct PassTextureAttachment
    {
        TextureAttachmentId   id;
        EPassAttachmentAccess access;
        EPassAttachmentUsage  usage;
    };

public:
    inline const std::string    GetName() const { return m_name; }
    inline const PassId         GetId() const { return m_id; }
    inline const EPassTaskClass GetTaskClass() const { return m_taskClass; }

    inline void UseTextureAttachment(const TextureAttachmentId attachmentId, EPassAttachmentAccess access, EPassAttachmentUsage usage)
    {
        m_textureAttachments.push_back({attachmentId, access, usage});
    }

    inline void UseBufferAttachment(const BufferAttachmentId attachmentId, EPassAttachmentAccess access, EPassAttachmentUsage usage)
    {
        m_bufferAttachments.push_back({attachmentId, access, usage});
    }

    inline void UseTextureAttachments(ArrayView<const TextureAttachmentId> attachmentIds, EPassAttachmentAccess access, EPassAttachmentUsage usage)
    {
        for (const auto& id : attachmentIds)
            m_textureAttachments.push_back({id, access, usage});
    }

    // inline void UseResolveAttachment(const TextureAttachmentId resolveAttachmentId) {}

    inline void UseColorAttachment(const TextureAttachmentId colorAttachmentId)
    {
        UseTextureAttachment(colorAttachmentId, EPassAttachmentAccess::ReadWrite, EPassAttachmentUsage::Color);
    }

    inline void UseColorAttachments(ArrayView<const TextureAttachmentId> colorAttachmentIds)
    {
        UseTextureAttachments(colorAttachmentIds, EPassAttachmentAccess::ReadWrite, EPassAttachmentUsage::Color);
    }

    inline void UseSwapchainAttachment(ISwapChain& swapchain) { m_pSwapchain = &swapchain; }

    // inline void UseDepthStencilAttachment(const TextureAttachmentId attachmentId, EPassAttachmentAccess access) {}
    // inline void UseSubpassInputAttachment(const TextureAttachmentId attachmentId) {}
    // inline void UseSubpassInputAttachments(ArrayView<const TextureAttachmentId> attachmentIds) {}
    // inline void UseCopyAttachment(const BufferAttachmentId copyAttachmentId, EPassAttachmentAccess access) {}
    // inline void UseCopyAttachment(const TextureAttachmentId copyAttachmentId, EPassAttachmentAccess access) {}

    inline void ExecuteAfter(const PassId& scopeId) { m_executeAfter.push_back(scopeId); }
    inline void ExecuteBefore(const PassId& scopeId) { m_executeBefore.push_back(scopeId); }
    inline void SignalFence(IFence& fence) { m_signalFences.push_back(&fence); }
    inline void WaitFence(IFence& fence) { m_waitFences.push_back(&fence); }

    // Getters

    inline std::vector<PassBufferAttachment>  GetBufferAttachments() const { return m_bufferAttachments; };
    inline std::vector<PassTextureAttachment> GetTextureAttachments() const { return m_textureAttachments; };

public:
    inline bool IsSwapchainBound() const { return m_pSwapchain; };

private:
    PassId         m_id;
    std::string    m_name;
    EPassTaskClass m_taskClass;

    std::vector<PassId> m_executeAfter;
    std::vector<PassId> m_executeBefore;

    std::vector<IFence*> m_waitFences;
    std::vector<IFence*> m_signalFences;

    std::vector<PassBufferAttachment>  m_bufferAttachments;
    std::vector<PassTextureAttachment> m_textureAttachments;
    
    ISwapChain* m_pSwapchain;
};

class RenderGraphBuilder
{
public:
    struct PresistentTextureAttachment
    {
        TexturePassAttachmentDesc desc;
        ITextureView*             pView;
    };

    struct PresistentBufferAttachment
    {
        BufferPassAttachmentDesc desc;
        IBuffer*                 pBuffer;
    };

public:
    Pass& CreatePass(std::string name, EPassTaskClass taskType)
    {
        m_passes.emplace_back(name, taskType);
        m_passes.back().SetId(PassId(m_passes.size() - 1));
        return m_passes.back();
    }

    Pass& GetPass(PassId id) { return m_passes[id.id]; }

    inline TextureAttachmentId BindPersistentTextureAttachment(const TexturePassAttachmentDesc& attachmentDesc, ITextureView& attachmentResource)
    {
        m_persistentTextureAttachments.push_back({attachmentDesc, &attachmentResource});
        return BufferAttachmentId(m_persistentTextureAttachments.size() - 1, EAttachmentLifetimeType::Presistent);
    }

    inline BufferAttachmentId BindPersistentBufferAttachment(const BufferPassAttachmentDesc& attachmentDesc, IBuffer& attachmentResource)
    {
        m_persistentBufferAttachments.push_back({attachmentDesc, &attachmentResource});
        return BufferAttachmentId(m_persistentBufferAttachments.size() - 1, EAttachmentLifetimeType::Presistent);
    }

    inline TextureAttachmentId CreateTransientTextureAttachment(const TexturePassAttachmentDesc& attachmentDesc)
    {
        m_transientTextureAttachments.push_back(attachmentDesc);
        return BufferAttachmentId(m_transientTextureAttachments.size() - 1, EAttachmentLifetimeType::Transient);
    }

    inline BufferAttachmentId CreateTransientBufferAttachment(const BufferPassAttachmentDesc& attachmentDesc)
    {
        m_transientBufferAttachments.push_back(attachmentDesc);
        return BufferAttachmentId(m_transientBufferAttachments.size() - 1, EAttachmentLifetimeType::Transient);
    }

    inline const std::vector<Pass>& GetPasses() const { return m_passes; };

private:
    std::vector<Pass> m_passes;

    std::vector<PresistentTextureAttachment> m_persistentTextureAttachments;
    std::vector<PresistentBufferAttachment>  m_persistentBufferAttachments;

    std::vector<TexturePassAttachmentDesc> m_transientTextureAttachments;
    std::vector<BufferPassAttachmentDesc>  m_transientBufferAttachments;
};

} // namespace RHI
