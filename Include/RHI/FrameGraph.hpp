#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "RHI/Commands.hpp"
#include "RHI/Common.hpp"
#include "RHI/Format.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/Resource.hpp"

namespace RHI
{

class ISwapchain;

enum class EAttachmentUsage
{
    RenderTarget,
    Present,
    BufferAttachment,
    ColorAttachment,
    TexelBuffer,
    InputAttachment,
    InputResource,
};

class IPass;
template <typename Resource>
class PassAttachment;

template <typename T>
struct ResourceToView
{
    using View = void;
    static_assert(std::is_same_v<T, IImage> || std::is_same_v<T, IBuffer>, "T must be either IImage or IBuffer type");
};

template <>
struct ResourceToView<IImage>
{
    using View = IImageView;
};

template <>
struct ResourceToView<IBuffer>
{
    using View = IBufferView;
};

template <typename T>
struct ViewToResource
{
    using View = void;
    static_assert(std::is_same_v<T, IImageView> || std::is_same_v<T, IBufferView>, "T must be either IImageView or IBuffer typeView");
};

template <>
struct ViewToResource<IImageView>
{
    using Resource = IImage;
};

template <>
struct ViewToResource<IBufferView>
{
    using Resource = IBuffer;
};

template <typename Resource>
class FrameAttachment
{
public:
    using PassViewType = typename ResourceToView<Resource>::View;

    std::string_view GetName() const;

    Resource&       GetResource();
    const Resource& GetResource() const;

    const PassAttachment<PassViewType>& GetFirstUse() const;
    PassAttachment<PassViewType>&       GetFirstUse();

    const PassAttachment<PassViewType>& GetLastUse() const;
    PassAttachment<PassViewType>&       GetLastUse();

    IPass& GetFirstPass() const;
    IPass& GetLastPass() const;

protected:
    std::string                   m_name;
    Unique<Resource>              m_resource;
    PassAttachment<PassViewType>* m_pFirstUse;
    PassAttachment<PassViewType>* m_pLastUse;
};

template <typename ResourceView>
class PassAttachment
{
public:
    using ResourceType = typename ResourceToView<ResourceView>::Resource;
    class Iterator;

    const Iterator begin() const;
    Iterator       begin();

    const Iterator end() const;
    Iterator       end();

    ResourceView&       GetView();
    const ResourceView& GetView() const;

    EAccess          GetAccess() const;
    EAttachmentUsage GetUsage() const;

    FrameAttachment<ResourceType>&       GetFrameAttachment();
    const FrameAttachment<ResourceType>& GetFrameAttachment() const;

protected:
    EAccess          m_access;
    EAttachmentUsage m_usage;
};

using ImageFrameAttachment  = FrameAttachment<IImage>;
using ImagePassAttachment   = PassAttachment<IImageView>;
using BufferFrameAttachment = FrameAttachment<IBuffer>;
using BufferPassAttachment  = PassAttachment<IBufferView>;

///////////////////////////////////////////

enum class EHardwareQueueType
{
    Graphics,
    Compute,
    Transfer
};

class IPass
{
public:
    class Callbacks;

    const RenderTargetLayout& GetRenderTargetLayout() const;

    void SetCallbacks(Callbacks& callbacks);

    virtual EResultCode Submit() = 0;

    std::vector<ImagePassAttachment*> GetTransientImageAttachments();

    std::vector<BufferPassAttachment*> GetTransientBufferAttachments();

    std::vector<ImagePassAttachment*> GetProducedImageAttachments();

    std::vector<BufferPassAttachment*> GetProducedBufferAttachments();

    std::vector<ImagePassAttachment*> GetConsumedImageAttachments();

    std::vector<BufferPassAttachment*> GetConsumedBufferAttachments();

    std::vector<ISwapchain*> GetSwapchainsToPresent();

    std::vector<IFence*> GetFencesToSignal();

protected:
    std::vector<BufferPassAttachment*> m_bufferAttachments;
    std::vector<ImagePassAttachment*>  m_imageAttachments;
};

class IFrameGraph
{
public:
    class Builder;

    virtual Unique<IPass> AddRenderPass(std::string_view name, EHardwareQueueType queueType) = 0;

    EResultCode CreateTransientImageAttachment(std::string_view attachmentName, const ImageDesc& desc);
    EResultCode CreateTransientBufferAttachment(std::string_view attachmentName, const BufferDesc& desc);

    EResultCode ImportSwapchain(std::string_view attachmentName, ISwapchain& swapchain);
    EResultCode ImportImageResource(std::string_view attachmentName, Unique<IImage> image);
    EResultCode ImportBufferResource(std::string_view attachmentName, Unique<IBuffer> buffer);

    void BeginFrame();
    void EndFrame();

    virtual void BeginFrameInternal() {}
    virtual void EndFrameInternal() {}
};

class FrameGraphBindingContext
{
public:
    IImageView*  GetImageView(std::string_view attachmentName);
    IBufferView* GetBufferView(std::string_view attachmentName);
    IImage*      GetImageResource(std::string_view attachmentName);
    IBuffer*     GetBufferResource(std::string_view attachmentName);
};

class FrameGraphBuilder
{
public:
    void UseOutputAttachment(std::string_view attachmentName, ImageViewDesc& view);

    void UseDepthStencilAttachment(std::string_view attachmentName, ImageViewDesc& view);

    void UseShaderResource(std::string_view attachmentName, ImageViewDesc& view, EAccess access);

    void UseShaderResource(std::string_view attachmentName, BufferViewDesc& view, EAccess access);

    void UseShaderResource(std::string_view attachmentName, BufferViewRange& range, EAccess access);

    void AddSignalFence(Unique<IFence> fence);

    void WaitPass(std::string_view passName);
};

class IPass::Callbacks
{
public:
    virtual void SetupRenderPass(FrameGraphBuilder& builder) = 0;

    virtual void BindShaderAttachments(FrameGraphBindingContext& context) = 0;

    virtual void BuildCommandList(ICommandBuffer& commandBuffer) = 0;
};

} // namespace RHI