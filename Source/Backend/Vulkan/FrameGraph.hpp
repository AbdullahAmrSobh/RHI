#pragma once
#include "RHI/Common.hpp"
#include "RHI/FrameGraph.hpp"
#include "RHI/Swapchain.hpp"
#include "Backend/Vulkan/RenderPass.hpp"
#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{

    struct Subpass
    {
        RenderPass* pRenderPass;
        uint32_t    subpassIndex;
    };
    
    class Pass final : public IPass
    {
    public:
        Pass();
        ~Pass();

        virtual EResultCode Submit() override;

    private:
        Unique<Framebuffer> m_framebuffer;
    };

    class FrameGraph final : public IFrameGraph
    {
    public:
        FrameGraph();
        ~FrameGraph();

        virtual Unique<IPass> AddRenderPass(std::string_view name, EHardwareQueueType queueType) override;
        virtual void          BeginFrameInternal() override;
        virtual void          EndFrameInternal() override;
    };

} // namespace Vulkan
} // namespace RHI