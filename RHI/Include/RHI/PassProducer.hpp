#pragma once

#include <memory>
#include <string>

#include "RHI/Export.hpp"

namespace RHI
{

class CommandList;

struct ImageView;
struct BufferView;

enum class PassQueue
{
    Graphics,
    Compute,
    Transfer,
}

class ShaderBindGroupContext;

class FrameGraphBuilder
{
public:
    FrameGraphBuilder(FrameScheduler* secheduler, Pass* pass);
    ~FrameGraphBuilder();

    RHI_FORCE_INLINE Handle<ImageView> ImportImage(std::string name, const ImageAttachmentImportInfo& importInfo, AttachmentUsage usage, AttachmentAccess access);

    RHI_FORCE_INLINE Handle<BufferView> ImportBuffer(std ::string name, const BufferAttachmentImportInfo& importInfo, AttachmentUsage usage, AttachmentAccess access);

    RHI_FORCE_INLINE Handle<ImageView> CreateTransientImageAttachment(std::string name, const ImageCreateInfo& createInfo, AttachmentUsage usage, AttachmentAccess access);

    RHI_FORCE_INLINE Handle<BufferView> CreateTransientBufferAttachment(std::string name, const BufferCreateInfo& createInfo, AttachmentUsage usage, AttachmentAccess access);

    RHI_FORCE_INLINE Handle<ImageView> UseImageAttachment(std::string name, const ImageAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access);

    RHI_FORCE_INLINE Handle<BufferView> UseBufferAttachment(std::string name, const BufferAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access);

    RHI_FORCE_INLINE Handle<ImageView> UseRenderTarget(std::string name, const ImageAttachmentUseInfo& useInfo);

    RHI_FORCE_INLINE Handle<ImageView> UseDepthTarget(std::string name, const BufferAttachmentUseInfo& useInfo);

    RHI_FORCE_INLINE Handle<ImageView> UseStencilTarget(std::string name, const BufferAttachmentUseInfo& useInfo);

    RHI_FORCE_INLINE Handle<ImageView> UseDepthStencilTarget(std::string name, const BufferAttachmentUseInfo& useInfo);

    RHI_FORCE_INLINE Handle<BufferView> UseBufferCopySrc(std::string name, const BufferAttachmentUseInfo& useInfo);

    RHI_FORCE_INLINE Handle<BufferView> UseBufferCopyDst(std::string name, const BufferAttachmentUseInfo& useInfo);

    RHI_FORCE_INLINE Handle<ImageView> UseImageCopyDst(std::string name, const BufferAttachmentUseInfo& useInfo);

    RHI_FORCE_INLINE Handle<ImageView> UseImageShaderInput(std::string name, const BufferAttachmentUseInfo& useInfo);

    RHI_FORCE_INLINE Handle<BufferView> UseBufferShaderInput(std::string name, const BufferAttachmentUseInfo& useInfo);

    RHI_FORCE_INLINE Handle<ImageView> UseRWImageShaderInput(std::string name, const BufferAttachmentUseInfo& useInfo);

    RHI_FORCE_INLINE Handle<BufferView> UseRWBufferShaderInput(std::string name, const BufferAttachmentUseInfo& useInfo);

    RHI_FORCE_INLINE void ExecuteAfter(std::string name);

    RHI_FORCE_INLINE void ExecuteBefore(std::string name);

private:
    FrameScheduler* m_frameScheduler;
};

/// @brief Base class for Pass producers, user overides this class to setup their own pass.
class PassProducer
{
    friend class FrameScheduler;

public:
    PassProducer(std::string name, PassQueue queueType);

    virtual ~PassProducer() = default;

    Fence& GetFence() const;

    /// @brief Sets up the attachments that will be used in this pass.
    virtual void SetupAttachments(FrameGraphBuilder& builder, ShaderBindGroupContext) = 0;

    /// @brief Builds the CommandList for this pass.
    virtual void BuildCommandList(CommandList& commandList) = 0;
};

}  // namespace RHI