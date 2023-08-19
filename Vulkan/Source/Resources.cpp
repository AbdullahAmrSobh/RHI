#include "RHI/Resources.hpp"

namespace Vulkan
{

Result<ResourcePool> ResourcePool::Create(Context* context, RHI::ResourcePoolCreateInfo& createInfo)
{
    return {};
}

ResourcePool::~ResourcePool()
{
}

RHI::ResourcePoolReport ResourcePool::GetMemoryReport() const
{
    return {};
}

void ResourcePool::ReportLiveObjects() const
{
}

RHI::Result<RHI::Handle<RHI::Image>> ResourcePool::Allocate(const ImageCreateInfo& createInfo)
{
    return {};
}

RHI::Result<RHI::Handle<RHI::Buffer>> ResourcePool::Allocate(const BufferCreateInfo& createInfo)
{
    return {};
}

void ResourcePool::Free(RHI::Handle<RHI::Image> image)
{
}

void ResourcePool::Free(RHI::Handle<RHI::Buffer> buffer)
{
}

size_t ResourcePool::GetSize(RHI::Handle<RHI::Image> image) const
{
    return {};
}

size_t ResourcePool::GetSize(RHI::Handle<RHI::Buffer> buffer) const
{
    return {};
}

DeviceMemoryPtr ResourcePool::MapResource(RHI::Handle<RHI::Image> image, size_t offset, size_t range)
{
    return {};
}

DeviceMemoryPtr ResourcePool::MapResource(RHI::Handle<RHI::Buffer> buffer, size_t offset, size_t range)
{
    return {};
}

void ResourcePool::Unamp(RHI::Handle<RHI::Image> image)
{
}

void ResourcePool::Unamp(RHI::Handle<RHI::Buffer> buffer)
{
}

}  // namespace Vulkan