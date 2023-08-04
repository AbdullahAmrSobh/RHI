#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "RHI/Export.hpp"
#include "RHI/FrameGraphAttachments.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{

enum class AttachmentAccess;
enum class AttachmentUsage;

struct ImageAttachmentUseInfo;
struct BufferAttachmentUseInfo;

class Pass;

// Represent the Graph of dependency between Passes
class RHI_EXPORT FrameGraph
{
public:
    FrameGraph()
        : m_registry(std::make_unique<AttachmentsRegistry>())
        , m_currentPass(nullptr)
    {
    }

    inline const AttachmentsRegistry& GetRegistry() const
    {
        return *m_registry;
    }

    inline AttachmentsRegistry& GetRegistry()
    {
        return *m_registry;
    }

    void Begin();
    void End();

    void UseImageAttachment(AttachmentName name, const ImageAttachmentUseInfo& attachmentInfo, AttachmentUsage usage, AttachmentAccess access);

    void UseBufferAttachment(AttachmentName name, const BufferAttachmentUseInfo& attachmentInfo, AttachmentUsage usage, AttachmentAccess access);

    void UseRenderTarget(AttachmentName name, const ImageAttachmentUseInfo& attachmentInfo);

    void UseDepthStencil(AttachmentName name, const ImageAttachmentUseInfo& attachmentInfo);

    void UseShaderImageInput(AttachmentName name, const ImageAttachmentUseInfo& attachmentInfo, AttachmentAccess access);

    void UseShaderBufferInput(AttachmentName name, const BufferAttachmentUseInfo& attachmentInfo, AttachmentAccess access);

    void CopyImageAttachment(AttachmentName name, const ImageAttachmentUseInfo& attachmentInfo, AttachmentAccess access);

    void CopyBufferAttachment(AttachmentName name, const BufferAttachmentUseInfo& attachmentInfo, AttachmentAccess access);

    void AddSignalFence(Fence& fence);

    void ReserveCommandLists(uint32_t count);

private:
    friend class FrameScheduler;

    std::unique_ptr<AttachmentsRegistry> m_registry;

    struct Node
    {
        Pass*              pass;
        std::vector<Pass*> producers;
        std::vector<Pass*> consumers;
        bool               visited;
    };

    std::vector<Node> m_graph;

    Pass* m_currentPass;
};

}  // namespace RHI