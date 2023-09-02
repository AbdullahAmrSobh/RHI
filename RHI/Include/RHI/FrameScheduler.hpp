#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "RHI/Attachment.hpp"
#include "RHI/Export.hpp"
#include "RHI/Handle.hpp"
#include "RHI/Span.hpp"

namespace RHI
{

struct PassCreateInfo;

class Swapchain;
class Pass;

class Fence;

/// @brief A frame scheduler is a frame-graph system breaks down the final frame
/// into a set of passes, each pass represent a GPU workload. Passes share resources
/// as Attachments. The frame scheduler tracks every attachment state accross passe.
class RHI_EXPORT FrameScheduler : public Object
{
public:
    using Object::Object;
    virtual ~FrameScheduler() = default;

    /// @brief Called at the beginning of the render-loop.
    /// This marks the begining of a graphics frame.
    void Begin();

    /// @brief Called at the ending of the render-loop.
    /// This marks the ending of a graphics frame.
    void End();

    /// @brief Register a pass producer, to be called this frame.
    void Submit(Pass& pass);

    /// @brief Compiles the frame graph, no new passes are allowed to be submitted 
    /// in the current frame after a this function is called.
    void Compile();

    virtual std::unique_ptr<Pass> CreatePass(const PassCreateInfo& createInfo) = 0;

protected:
    virtual void BeginInternal() = 0;

    virtual void EndInternal() = 0;

protected:
    friend class Pass;

    struct Node
    {
        Pass*                 pass;
        std::vector<uint32_t> m_producerIndcies;
        std::vector<uint32_t> m_consumerIndcies;
    };

    std::vector<Node> m_graphNodes;

    std::vector<Pass*> m_passList;

    AttachmentPool m_imageAttachments;

    AttachmentPool m_bufferAttachments;

    std::vector<Handle<Attachment>> m_transientImagesAttachments;

    std::vector<Handle<Attachment>> m_transientBuffersAttachments;

    std::vector<Swapchain*> m_swapchainsToPresent;
};

}  // namespace RHI