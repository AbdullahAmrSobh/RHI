#pragma once
#include "RHI/Common.hpp"
#include "RHI/FrameGraph.hpp"
#include "RHI/PipelineState.hpp"

#include "Backend/Vulkan/Commands.hpp"
#include "Backend/Vulkan/Common.hpp"
#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/RenderPass.hpp"

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

        virtual Expected<Unique<IPass>> CreatePass(std::string name, EHardwareQueueType queueType) override;
        virtual void                    Submit(const IPass& pass) override;

    private:
        const Device* m_pDevice;

    };

} // namespace Vulkan
} // namespace RHI