#include "Renderer/Resources.hpp"

namespace Engine
{
    void BufferPool::init(RHI::Device* device, const RHI::BufferCreateInfo& createInfo)
    {
        m_allocator->init(createInfo.byteSize);
        m_buffer = device->CreateBuffer(createInfo);
    }

    void BufferPool::shutdown(RHI::Device* device)
    {
        device->DestroyBuffer(m_buffer);
    }
} // namespace Engine