#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "RHI/Export.hpp"
#include "RHI/Handle.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{

struct ImageCreateInfo;
struct BufferCreateInfo;

class Context;
class PassProducer;
class Pass;

using PassAttachmentList = std::vector<AttachmentView>;

enum class AttachmentType
{
    Image,
    Buffer,
    Swapchain,
};

enum class ImageAspect
{
    None    = 0 << 0,
    Color   = 1 << 1,
    Depth   = 1 << 2,
    Stencil = 1 << 3,
};

enum class ComponentSwizzle
{
    None,
    Zero,
    One,
    R,
    G,
    B,
    A,
};

enum class AttachmentUsage
{
    None,
    RenderTarget,
    Depth,
    Stencil,
    DepthStencil,
    ShaderResource,
    Copy,
    Resolve,
};

enum class AttachmentAccess
{
    None,       // Invalid option
    Read,       // Attachment is read as a shader resource
    Write,      // Attachment is renderTargetOutput
    ReadWrite,  // Attachment is available for read and write as a shader resource
};

enum class AttachmentLifetime
{
    // Attachment resource is created outside of the frame
    Persistent,
    // Attachment resource is only valid for the duration of the current frame
    Transient,
};

enum class ImageLoadOperation
{
    // The attachment load operation undefined
    DontCare,
    // Load attachment content
    Load,
    // Discard attachment content
    Discard,
};

enum class ImageStoreOperation
{
    // Attachment Store operation is undefined
    DontCare,
    // Writes to the attachment are stored
    Store,
    // Writes to the attachment are discarded
    Discard,
};

enum class BlendFactor
{
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
};

enum class BlendEquation
{
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
};

struct ClearValue
{
    float r, g, b, a;

    inline bool operator==(const ClearValue& other) const
    {
        return r = other.r && g = other.g && b = other.b && a = other.a;
    }

    inline bool operator!=(const ClearValue& other) const
    {
        return !(*this == other);
    }
};

struct ImageLoadStoreOperations
{
    ImageLoadOperation  loadOperation;
    ImageStoreOperation storeOperation;

    inline bool operator==(const ImageLoadStoreOperations& other) const
    {
        return loadOperation == other.loadOperation && storeOperation == other.storeOperation;
    }

    inline bool operator!=(const ImageLoadStoreOperations& other) const
    {
        return !(*this == other);
    }
};

struct ImageAttachmentBlendInfo
{
    bool          blendEnable;
    BlendEquation colorBlendOp;
    BlendFactor   srcColor;
    BlendFactor   dstColor;
    BlendEquation alphaBlendOp;
    BlendFactor   srcAlpha;
    BlendFactor   dstAlpha;

    inline bool operator==(const ImageAttachmentBlendInfo& other)
    {
        return blendEnable == other.blendEnable && colorBlendOp == other.colorBlendOp && srcColor == other.srcColor && dstColor == other.dstColor && alphaBlendOp == other.alphaBlendOp && srcAlpha == other.srcAlpha && dstAlpha == other.dstAlpha;
    }

    inline bool operator!=(const ImageAttachmentBlendInfo& other)
    {
        return !(*this == other);
    }
};

struct ComponentMapping
{
    ComponentSwizzle r;
    ComponentSwizzle g;
    ComponentSwizzle b;
    ComponentSwizzle a;

    inline bool operator==(const ComponentMapping& other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    inline bool operator!=(const ComponentMapping& other) const
    {
        return !(*this == other);
    }
};

struct ImageSubresource
{
    Flags<ImageAspect> aspectMask;
    uint32_t           mipLevel;
    uint32_t           levelsCount;
    uint32_t           baseArrayLayer;
    uint32_t           layerCount;

    inline bool operator==(const ImageSubresource& other) const
    {
        return aspectMask == other.aspectMask && mipLevel == other.mipLevel && levelsCount == other.levelsCount && baseArrayLayer == other.baseArrayLayer && layerCount == other.layerCount;
    }

    inline bool operator!=(const ImageSubresource& other) const
    {
        return !(*this == other);
    }
};

struct ImageAttachmentUseInfo
{
    ComponentMapping         components;
    ImageSubresource         subresource;
    ImageLoadStoreOperations loadStoreOperations;
    ImageAttachmentBlendInfo blendInfo;
    ClearValue               clearValue;

    inline bool operator==(const ImageAttachmentUseInfo& other) const
    {
        return components == other.components && subresource == other.subresource && loadStoreOperations == other.loadStoreOperations && blendInfo == other.blendInfo && clearValue == other.clearValue;
    }

    inline bool operator!=(const ImageAttachmentUseInfo& other) const
    {
        return !(*this == other);
    }
};

struct BufferAttachmentUseInfo
{
    Format format;
    size_t byteOffset;
    size_t byteSize;

    inline bool operator==(const ImageAttachmentUseInfo& other) const
    {
        return format == other.format && byteOffset == other.byteOffset && byteSize == other.byteSize;
    }

    inline bool operator!=(const ImageAttachmentUseInfo& other) const
    {
        return !(*this == other);
    }
};

struct ImageAttachmentImportInfo
{
};

struct BufferAttachmentImportInfo
{
};

struct Attachment
{
    std::string name;

    const AttachmentLifetime lifetime;

    const AttachmentType type;

    Handle resource;

    union
    {
        ImageCreateInfo  imageInfo;
        BufferCreateInfo bufferInfo;
    };
};

struct AttachmentView
{
    Attachment* attachment;

    Pass* pass;

    AttachmentUsage  usage;
    AttachmentAccess access;

    Handle view;

    union
    {
        BufferAttachmentUseInfo bufferInfo;
        ImageAttachmentUseInfo  imageInfo;
    };
};

/// @brief A frame scheduler is a frame-graph system breaks down the final frame
/// into a set of passes, each pass represent a GPU workload. Passes share resources
/// as Attachments. The frame scheduler tracks every attachment state accross passe.
class RHI_EXPORT FrameScheduler
{
public:
    virtual ~FrameScheduler() = default;

    /// @brief Called at the beginning of the render-loop.
    /// This marks the begining of a graphics frame.
    void Begin();

    /// @brief Called at the ending of the render-loop.
    /// This marks the ending of a graphics frame.
    void End();

    /// @brief Register a pass producer, to be called this frame.
    /// @param passProducer
    void AddPass(const PassProducer& passProducer);

private:
    void BeignPass(Pass* pass);

    void EndPass();

    /// @brief Adds a dependency between consumer, and producer passes.
    void AddPassDependency(Pass* consumer, Pass* producer);

    /// @brief Returns a Fence, which is signaled when its associated pass is finished executing.
    Fence& AddSignalFence();

    /// @brief Imports an image resource, to be used in the current pass.
    void ImportImage(std::string name, const ImageAttachmentImportInfo& importInfo, AttachmentUsage usage, AttachmentAccess access);

    /// @brief Imports a buffer resource, to be used in the current pass.
    void ImportBuffer(std::string name, const BufferAttachmentImportInfo& importInfo, AttachmentUsage usage, AttachmentAccess access);

    /// @brief Creates a transient image attachment, which is used in the current pass.
    void CreateTransientImageAttachment(std::string name, const ImageCreateInfo& createInfo, AttachmentUsage usage, AttachmentAccess access);

    /// @brief Creates a transient buffer attachment, which is used in the current pass.
    void CreateTransientBufferAttachment(std::string name, const BufferCreateInfo& createInfo, AttachmentUsage usage, AttachmentAccess access);

    /// @brief Uses an existing image attachment in the current pass.
    void UseImageAttachment(std::string name, const ImageAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access);

    /// @brief Uses an existing buffer attachment in the current pass.
    void UseBufferAttachment(std::string name, const BufferAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access);

    /// @brief Return the attachment handle assocaited with the given name.
    Handle GetHandleFromName(std::string name);

    /// @brief Return a list of handles to all the passes in the graph, topologically sorted.
    std::vector<Handle> ReorederPasses();

protected:
    virtual void BeginInternal() = 0;

    virtual void EndInternal() = 0;

private:
    Context* m_context;

    struct Node
    {
        Pass*                 pass;
        std::vector<uint32_t> m_producerIndcies;
        std::vector<uint32_t> m_consumerIndcies;
    };

    std::vector<Node> m_graphNodes;

protected:
    std::unordered_map<std::string, Handle> m_handleNamesMap;

    std::HandlePool<Attachment, PassAttachmentList> m_imageAttachments;

    std::HandlePool<Attachment, PassAttachmentList> m_bufferAttachments;

    std::vector<Handle> m_transientImageAttachments;

    std::vector<Handle> m_transientBufferAttachments;

    /// @brief List of all registered pass producers, to be executed this frame.
    std::vector<PassProducer*> m_passProducers;
};

}  // namespace RHI