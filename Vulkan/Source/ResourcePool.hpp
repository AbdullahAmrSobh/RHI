#pragma once

#include <RHI/ResourcePool.hpp>

namespace Vulkan
{

class Context;

class ResourcePool final : public RHI::ResourcePool
{
public:
    static RHI::Result<ResourcePool*> Create(Context* context, RHI::ResourcePoolCreateInfo& createInfo);

    using RHI::ResourcePool::ResourcePool;
    ~ResourcePool();

    RHI::ResourcePoolReport GetMemoryReport() const override;

    void ReportLiveObjects() const override;

    RHI::Result<RHI::Handle<RHI::Image>>  Allocate(const RHI::ImageCreateInfo& createInfo) override;
    RHI::Result<RHI::Handle<RHI::Buffer>> Allocate(const RHI::BufferCreateInfo& createInfo) override;

    void Free(RHI::Handle<RHI::Image> image) override;
    void Free(RHI::Handle<RHI::Buffer> buffer) override;

    size_t GetSize(RHI::Handle<RHI::Image> image) const override;
    size_t GetSize(RHI::Handle<RHI::Buffer> buffer) const override;

    RHI::DeviceMemoryPtr MapResource(RHI::Handle<RHI::Image> image, size_t offset, size_t range) override;
    RHI::DeviceMemoryPtr MapResource(RHI::Handle<RHI::Buffer> buffer, size_t offset, size_t range) override;

    void Unmap(RHI::Handle<RHI::Image> image) override;
    void Unmap(RHI::Handle<RHI::Buffer> buffer) override;
};

}  // namespace Vulkan