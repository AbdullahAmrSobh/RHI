#include "BufferPool.hpp"

namespace Engine
{
    BufferPool::BufferPool()
        : m_buffer(RHI::NullHandle)
        , m_allocator(0)
    {
    }

    ResultCode BufferPool::Init(RHI::Device& device, const RHI::BufferCreateInfo& createInfo)
    {
        m_device = &device;

        // Ugly hack to avoid calling the constructor of the allocator
        new (&m_allocator) OffsetAllocator::Allocator(createInfo.byteSize);

        m_buffer              = device.CreateBuffer(createInfo);
        return ResultCode::Success;
    }

    void BufferPool::Shutdown()
    {
        m_device->DestroyBuffer(m_buffer);
    }

    void BufferPool::Reset()
    {
        m_allocator.reset();
    }

    Result<Suballocation> BufferPool::Allocate(size_t size, size_t alignment)
    {
        auto allocation = m_allocator.allocate(TL::AlignUp(size, alignment));
        if (allocation.offset == OffsetAllocator::Allocation::NO_SPACE)
        {
            return ResultCode::ErrorPoolOutOfMemory;
        }
        return allocation;
    }

    void BufferPool::Release(const Suballocation& allocation)
    {
        m_allocator.free(allocation);
    }

    RHI::Handle<RHI::Buffer> BufferPool::GetBuffer() const
    {
        return m_buffer;
    }


    void BufferPool::Write(Suballocation suballocation, TL::Block block)
    {
        m_device->BufferWrite(m_buffer, suballocation.offset, block);
    }

} // namespace Engine