#include "BufferPool.hpp"

namespace Engine
{
    BufferPool::BufferPool()
        : m_buffer(RHI::NullHandle)
        , m_allocator(0)
    {
    }

    ResultCode BufferPool::Init(RHI::Device& device, RHI::BufferCreateInfo& createInfo)
    {
        m_device = &device;

        // Ugly hack to avoid calling the constructor of the allocator
        new (&m_allocator) OffsetAllocator::Allocator(createInfo.byteSize);

        auto [buffer, result] = device.CreateBuffer(createInfo);
        m_buffer              = buffer;
        return result;
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

    void BufferPool::BeginUpdate()
    {
        m_mappedPtr = m_device->MapBuffer(m_buffer);
    }

    void BufferPool::EndUpdate()
    {
        m_device->UnmapBuffer(m_buffer);
        m_mappedPtr = nullptr;
    }

    void BufferPool::Write(Suballocation suballocation, TL::Block block)
    {
        bool shouldUnmap = false;
        if (!m_mappedPtr)
        {
            shouldUnmap = true;
            BeginUpdate();
        }
        memcpy((char*)m_mappedPtr + suballocation.offset, block.ptr, block.size);
        if (shouldUnmap)
        {
            EndUpdate();
        }
    }

} // namespace Engine