#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "RHI/Attachments.hpp"
#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{

enum class QueueType; 

using QueueID = uint32_t; // id of the queue family this pass is executed on

class PassState
{
    friend class FrameGraph;

public:
    PassState(std::string name, PassQueue queueType);

    PassQueue GetPassQueueType() const;

private:
    const std::string m_name;

    const PassQueue m_queueType;

    QueueID queueId; 
    
    std::unique_ptr<uint32_t> m_nodeIndex;

    std::vector<PassState*> m_producers;

    std::vector<PassState*> m_consumers;

    std::vector<PassAttachment*> m_attachments;

    std::vector<PassAttachment*> m_transientAttachments;

    std::vector<ImagePassAttachment*> m_imageAttachments;

    std::vector<BufferPassAttachment*> m_bufferAttachments;

    std::vector<Fence*> m_signalFences;

    Swapchain* m_swapchain;

    uint32_t m_commandListCount;
};

class FrameGraph
{
public:
    FrameGraph() = default;

    void Begin();
    void End();

    void Reset();

    void UseImageAttachment(const ImageAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access);

    void UseBufferAttachment(const BufferAttachmentUseInfo& useInfo, AttachmentUsage usage, AttachmentAccess access);

    void AddDependency(PassState* producer, PassState* consumer);

    void TopologicalSort();

private:
    friend class FrameScheduler;

    struct Node
    {
        PassState*              pass;
        std::vector<PassState*> producers;
        std::vector<PassState*> consumers;
        bool                    visited;
    };

    std::vector<Node> m_graph;
};

}  // namespace RHI