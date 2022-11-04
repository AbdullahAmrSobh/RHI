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

        virtual EResultCode BeginFrameInternal() override;

        virtual EResultCode EndFrameInternal() override;

        virtual EResultCode ExecuteInternal(IPassProducer& producer) override;

        virtual EResultCode CompilePass(IPass& pass) override;

    private:
        const Device* m_pDevice;

        std::vector<IPassProducer*> m_passes;
    };

} // namespace Vulkan
} // namespace RHI