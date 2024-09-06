#include "RHI/BufferPool.hpp"

#include <TL/Utils.hpp>
#include "RHI/Context.hpp"

#include "OffsetAllocator/offsetAllocator.hpp"

namespace RHI
{
    using Allocator = OffsetAllocator::Allocator;

    class BufferPool::Impl
    {
    public:
        Impl(uint32_t size)
            : m_allocator(size)
        {
        }

        Allocator m_allocator;
    };

    BufferPool BufferPool::Create(Context* context, const BufferCreateInfo& createInfo)
    {
        auto buffer = context->CreateBuffer(createInfo);
        auto pool = BufferPool(buffer.GetValue(), createInfo.byteSize);
        pool.m_context = context;
        return pool;
    }

    BufferPool::~BufferPool()
    {
        if (m_context)
        {
            m_context->DestroyBuffer(m_buffer);
        }
    }

    BufferPool::BufferPool(Handle<Buffer> buffer, size_t bufferSize)
        : m_context(nullptr)
        , m_impl(TL::CreatePtr<BufferPool::Impl>((uint32_t)bufferSize))
        , m_buffer(buffer)
    {
    }

    Result<BufferRange> BufferPool::Allocate(size_t size, size_t alignment)
    {
        auto alignedSize = TL::AlignUp(size, alignment);
        auto allocation = m_impl->m_allocator.allocate((uint32_t)alignedSize);

        if (allocation.offset == OffsetAllocator::Allocation::NO_SPACE)
        {
            return ResultCode::ErrorOutOfMemory;
        }

        return BufferRange{
            .buffer = m_buffer,
            .subregion = {.offset = allocation.offset, .size = alignedSize},
            ._metaData = allocation.metadata,
        };
    }

    void BufferPool::Release(BufferRange buffer)
    {
        m_impl->m_allocator.free({
            .offset = (uint32_t)buffer.subregion.offset,
            .metadata = buffer._metaData,
        });
    }

} // namespace RHI