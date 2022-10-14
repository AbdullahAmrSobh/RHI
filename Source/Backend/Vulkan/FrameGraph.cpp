#include "Backend/Vulkan/FrameGraph.hpp"

namespace RHI
{
namespace Vulkan
{

    VkResult IFrameGraph::Init()
    {
        Result<Unique<CommandAllocator>> result = CommandAllocator::Create(*m_pDevice);
    }

    void IFrameGraph::Submit(const IPass& pass) 
    {
        
    }

} // namespace Vulkan
} // namespace RHI