#pragma once
#include "RHI/FrameGraph.hpp"

#include "Backend/Vulkan/Commands.hpp"
#include "Backend/Vulkan/Common.hpp"
#include "Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{

    struct Subpass
    {
        RenderPass* pRenderPass;
        uint32_t    subpassIndex;
    };
    

    class FrameGraph final : public IFrameGraph
    {
    public:
        FrameGraph(const Device& device);
        ~FrameGraph();

        VkResult Init();

        virtual void Submit(const IPass& pass) override;

    private:
        const Device* m_pDevice;
    };

} // namespace Vulkan
} // namespace RHI