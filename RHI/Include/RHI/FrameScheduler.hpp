#pragma once

#include <cstdint>
#include <memory>
#include "RHI/Span.hpp"
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
struct AttachmentView;

class Context;
class PassProducer;
class Pass;

class Fence;


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
    void Submit(PassProducer& passProducer);

private:
    void BeignPass(Pass* pass);

    void EndPass();

    /// @brief Adds a dependency between consumer, and producer passes.
    void AddPassDependency(Pass* consumer, Pass* producer);

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

};

}  // namespace RHI