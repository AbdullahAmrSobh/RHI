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
        : m_cachedResourcesAllocator(CreateUnique<ResourceCachedAllocator>(device))
        , m_attachmentsRegistry(CreateUnique<AttachmentsRegistry>())
        , m_frameGraphBuilder(CreateUnique<FrameGraphBuilder>(*m_attachmentsRegistry))
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
    virtual Unique<IRenderPass> CreateRenderPass(std::string name) const = 0;

    virtual ICommandBuffer& BeginCommandBuffer(IRenderPass& renderpass) = 0;

    virtual void EndCommandBuffer() = 0;

    virtual void Submit(IRenderPass& renderpass) = 0;

private:
    Unique<ResourceCachedAllocator> m_cachedResourcesAllocator;

    Unique<AttachmentsRegistry> m_attachmentsRegistry;

    Unique<FrameGraphBuilder> m_frameGraphBuilder;

    std::vector<RenderPassProducer*> m_producers;
};

}  // namespace RHI