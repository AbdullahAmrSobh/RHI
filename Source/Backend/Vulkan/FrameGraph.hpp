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
        {}
        
        VkResult Init();

        virtual Expected<Unique<IPass>> CreatePass(std::string name, EPassType type) override;
        virtual EResultCode             Execute(const IPassProducer& producer) override;
    
    private:
        const Device* m_pDevice;
    };

} // namespace Vulkan
} // namespace RHI