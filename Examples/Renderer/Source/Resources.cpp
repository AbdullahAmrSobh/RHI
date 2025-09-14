#include "Renderer/Resources.hpp"

namespace Engine
{
    void BufferPool::init(RHI::Device* device, const RHI::BufferCreateInfo& createInfo)
    {
        m_device = device;
        m_allocator.init(createInfo.byteSize);
        m_buffer = m_device->CreateBuffer(createInfo);
    }

    void BufferPool::shutdown()
    {
        m_device->DestroyBuffer(m_buffer);
    }
} // namespace Engine