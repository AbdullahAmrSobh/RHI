#pragma once
#include "RHI/Common.hpp"
#include "RHI/FrameGraph.hpp"

namespace RHI
{
namespace Vulkan
{
    class Device; 

    class FrameGraph final : public IFrameGraph
    {
    public:
        FrameGraph(const Device& device)
            : m_pDevice(&device)
        {
        }

    private:
        const Device* m_pDevice;
    };

} // namespace Vulkan
} // namespace RHI