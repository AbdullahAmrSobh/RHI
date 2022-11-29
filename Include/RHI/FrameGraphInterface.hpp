#pragma once
#include "RHI/Commands.hpp"

namespace RHI
{

enum class  EAttachmentUsage;
enum class EAttachmentAccess;

struct ImagePassAttachmentDesc;
struct BufferPassAttachmentDesc;

class IFrameGraph;
class IAttachmentsRegistry;
class IPass;
class ICommandBuffer;

class FrameGraphBuilder
{
public:
    FrameGraphBuilder(IFrameGraph& frameGraph, IPass& pass)
        : m_pFrameGraph(&frameGraph)
    {
    }

    const IAttachmentsRegistry& GetAttachmentsRegistry() const;
    EResultCode UseImageAttachment(const ImagePassAttachmentDesc& description, EAttachmentUsage usage, EAttachmentAccess access);
    EResultCode UseBufferAttachment(const BufferPassAttachmentDesc& description, EAttachmentUsage usage, EAttachmentAccess access);
    EResultCode UseDepthStencilAttachment(const ImagePassAttachmentDesc& description, EAttachmentAccess access);
    EResultCode ExecuteAfter(const IPass& pass);

private:
    IFrameGraph* m_pFrameGraph;
    IPass*       m_pPass; 
    
};

class ExecuteContext
{
public:
    ExecuteContext(std::vector<Unique<ICommandBuffer>> commandList)
        : m_commandList(std::move(commandList))
    {
    }

    const std::vector<ICommandBuffer*> GetCommandList();

private:
    const IPass*                        m_pPass;
    std::vector<Unique<ICommandBuffer>> m_commandList;
};

} // namespace RHI