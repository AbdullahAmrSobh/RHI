#include "Renderer/Resources.hpp"

namespace Engine
{
    void BufferPool::init(RHI::Device* device, TL::StringView name, TL::Flags<RHI::BufferUsage> usage, size_t sizeBytes)
    {
        m_device = device;
        RHI::BufferCreateInfo createInfo{
            .name       = name.data(),
            .hostMapped = true,
            .usageFlags = usage,
            .byteSize   = sizeBytes,
        };
        m_allocator->init(createInfo.byteSize);
        m_buffer = device->CreateBuffer(createInfo);
    }

    void BufferPool::shutdown()
    {
        m_device->DestroyBuffer(m_buffer);
    }

    BufferPool::StorageReport BufferPool::storageReport() const
    {
        return m_allocator->storageReport();
    }

    BufferPool::StorageReportFull BufferPool::storageReportFull() const
    {
        return m_allocator->storageReportFull();
    }

    // expose the underlying RHI buffer for binding base
    RHI::Buffer* BufferPool::getBuffer()
    {
        return m_buffer;
    }

    const RHI::Buffer* BufferPool::getBuffer() const
    {
        return m_buffer;
    }

    void BufferPool::update(uint32_t offset, TL::Block block)
    {
        m_device->GetCurrentFrame()->BufferWrite(m_buffer, offset, block);
    }
} // namespace Engine