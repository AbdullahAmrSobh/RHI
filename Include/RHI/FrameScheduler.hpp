#pragma once
#include "RHI/AttachmentsRegistry.hpp"
#include "RHI/FrameGraphBuilder.hpp"
#include "RHI/RenderPass.hpp"
#include "RHI/RenderPassProducer.hpp"
#include "RHI/ResourceCachedAllocator.hpp"

namespace RHI
{
class IDevice;
class RenderPassProducer;

class IFrameScheduler
{
public:
    IFrameScheduler(IDevice& device)
        : m_cachedResourcesAllocator(std::make_unique<ResourceCachedAllocator>(device))
        , m_attachmentsRegistry(std::make_unique<AttachmentsRegistry>())
        , m_frameGraphBuilder(std::make_unique<FrameGraphBuilder>(*m_attachmentsRegistry))
    {
    }

    virtual ~IFrameScheduler() = default;

    const AttachmentsRegistry& GetAttachmentsRegistry() const
    {
        return *m_attachmentsRegistry;
    }

    AttachmentsRegistry& GetAttachmentsRegistry()
    {
        return *m_attachmentsRegistry;
    }

    void Begin();
    void End();

    void Schedule(RenderPassProducer& producer);

private:
    virtual std::unique_ptr<IRenderPass> CreateRenderPass(std::string name) const = 0;

    virtual ICommandBuffer& BeginCommandBuffer(IRenderPass& renderpass) = 0;

    virtual void EndCommandBuffer() = 0;

    virtual void Submit(IRenderPass& renderpass) = 0;

private:
    std::unique_ptr<ResourceCachedAllocator> m_cachedResourcesAllocator;

    std::unique_ptr<AttachmentsRegistry> m_attachmentsRegistry;

    std::unique_ptr<FrameGraphBuilder> m_frameGraphBuilder;

    std::vector<RenderPassProducer*> m_producers;
};

}  // namespace RHI