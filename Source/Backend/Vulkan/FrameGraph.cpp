#include "Backend/Vulkan/FrameGraph.hpp"
#include "Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{
    Expected<Unique<IFrameGraph>> Device::CreateFrameGraph()
    {
        Unique<FrameGraph> frameGraph = CreateUnique<FrameGraph>(*this);
        
        return std::move(frameGraph);
    } 

} // namespace Vulkan
} // namespace RHI