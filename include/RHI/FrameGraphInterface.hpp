#pragma once

#include <cstdint>
#include <memory>

#include "RHI/Command.hpp"
#include "RHI/FrameGraph.hpp"


namespace RHI
{

enum class PassQueue;

class FrameGraph;
class PassState;

struct ImageAttachmentUseInfo;
struct BufferAttachmentUseInfo;

// An interface for building the FrameGraph
class FrameGraphBuilder
{
public:
    FrameGraphBuilder(FrameGraph& frameGraph);
        
    void UseImageAttachment(const ImageAttachmentUseInfo& attachmentInfo, AttachmentUsage usage, AttachmentAccess access);

    void UseBufferAttachment(const BufferAttachmentUseInfo& attachmentInfo, AttachmentUsage usage, AttachmentAccess access);

    void UseRenderTarget(const ImageAttachmentUseInfo& attachmentInfo);

    void UseDepthStencil(const ImageAttachmentUseInfo& attachmentInfo);

    void UseShaderImageInput(const ImageAttachmentUseInfo& attachmentInfo, AttachmentAccess access);

    void UseShaderBufferInput(const BufferAttachmentUseInfo& attachmentInfo, AttachmentAccess access);

    void CopyImageAttachment(const ImageAttachmentUseInfo& useInfo, AttachmentAccess acess);

    void CopyBufferAttachment(const BufferAttachmentUseInfo& useInfo, AttachmentAccess acess);

    void AddSignalFence(const Fence& fence);

private:
    FrameGraph* m_frameGraph;
};

// The Pass class represents a pass in the frame graph.
// A pass is a self-contained stage of the rendering pipeline that can be executed independently of other passes.
class Pass
{
public:
    Pass(std::string name, PassQueue queueType) 
        : m_name(std::move(name))
        , m_queueType(queueType)
    {}
    virtual ~Pass() = default;

    // Sets up the attachments that will be used in this pass.
    virtual void SetupAttachments(FrameGraphBuilder builder) = 0;

    // Builds all the command lists required by this pass for the specified dispatch index.
    virtual void BuildCommandLists(uint32_t dispatchIndex, CommandList& commandList) = 0;

private:
    friend class FrameScheduler;
    const std::string          m_name;
    const PassQueue            m_queueType;
    std::unique_ptr<PassState> m_passState;
};

}  // namespace RHI