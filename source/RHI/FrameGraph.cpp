#include <stack>

#include "RHI/FrameGraph.hpp"

#include "RHI/Debug.hpp"

namespace RHI
{

void FrameGraph::Reset()
{
}

void FrameGraph::ImportImage(std::string name, Image& image)
{
}

void FrameGraph::ImportBuffer(std::string name, Buffer& buffer)
{
}

void FrameGraph::ImportSwapchainImage(std::string name, Swapchain& swapchain)
{
}

void FrameGraph::CreateTransientImageAttachment(std::string name, const ImageCreateInfo& createInfo)
{
}

void FrameGraph::CreateTransientBufferAttachment(std::string name, const BufferCreateInfo& createInfo)
{
}

void FrameGraph::UseImageAttachment(std::string_view name, ImageRegion attachmentInfo, AttachmentUsage usage, AttachmentAccess access)
{
}

void FrameGraph::UseBufferAttachment(std::string_view     name,
                                     BufferViewCreateInfo attachmentInfo,
                                     AttachmentUsage      usage,
                                     AttachmentAccess     access)
{
}

void FrameGraph::AddDependency(PassState* producer, PassState* consumer)
{
}

Attachment* FrameGraph::FindImageAttachment(std::string_view name)
{
    return {};
}

Attachment* FrameGraph::FindBufferAttachment(std::string_view name)
{
    return {};
}

void FrameGraph::TopologicalSort()
{
}

}  // namespace RHI