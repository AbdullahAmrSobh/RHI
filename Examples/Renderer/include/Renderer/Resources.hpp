#pragma once

#include <TL/Utils.hpp>
#include <TL/OffsetAllocator/OffsetAllocator.hpp>

#include <RHI/RHI.hpp>

#include <algorithm>

namespace Engine
{
    // TODO: Format, split into .hpp + .inl files
    inline constexpr static uint32_t alignUp(uint32_t val, uint32_t alignment)
    {
        // revist this; copied from chatGPT
        return (val + alignment - 1) / alignment * alignment;
    }

    // Templated buffer range that contains elements of type T
    template<typename T>
    class Buffer
    {
    public:
        Buffer() = default;

        Buffer(RHI::Buffer*             buffer,
            OffsetAllocator::Allocation allocation,
            uint32_t                    elementSize,
            uint32_t                    elementCount)
            : m_buffer(buffer)
            , m_allocation(allocation)
            , m_stride(elementSize)
            , m_elementsCount(elementCount)
        {
            TL_ASSERT(elementSize != 0, "Buffer elementSize must not be zero");
            TL_ASSERT(elementCount != 0, "Buffer elementCount must not be zero");
        }

        // return the underlaying RHI::Buffer
        inline const RHI::Buffer* getBuffer() const
        {
            TL_ASSERT(valid());
            return m_buffer;
        }

        // return the underlaying RHI::Buffer
        inline RHI::Buffer* getBuffer()
        {
            return m_buffer;
        }

        // return the offset in bytes to the start of the buffer allocation
        inline uint32_t getOffsetBytes() const
        {
            TL_ASSERT(valid());
            return alignUp(m_allocation.offset, m_stride);
        }

        // return the offset in bytess to the specified element
        inline uint32_t getOffsetBytes(uint32_t index) const
        {
            TL_ASSERT(valid());
            TL_ASSERT(index < m_elementsCount);
            return getOffsetBytes() + (m_stride * index);
        }

        // Return the size in bytes of this buffer allocation
        inline uint32_t getSizeBytes() const
        {
            TL_ASSERT(valid());
            return getElementsStride() * m_elementsCount;
        }

        // Return the per-element stirde. Usefull when buffer elements are aligned with custom device alignment e.g. minUniformBufferOffsetAlignment ...
        inline uint32_t getElementsStride() const
        {
            TL_ASSERT(valid());
            return alignUp(sizeof(T), m_stride);
        }

        // Return the number of elements in the buffer allocation
        inline uint32_t getElementsCount() const
        {
            return m_elementsCount;
        }

        inline bool valid() const
        {
            return m_buffer != nullptr && m_allocation.offset != OffsetAllocator::Allocation::NO_SPACE;
        }

        inline operator bool() const { return valid(); }

        inline operator RHI::BufferBindingInfo() const
        {
            return getBinding();
        }

        inline RHI::BufferBindingInfo getBinding() const
        {
            TL_ASSERT(valid());
            return {
                .buffer = m_buffer,
                .offset = getOffsetBytes(),
                .range  = getSizeBytes(),
            };
        }

        inline RHI::BufferBindingInfo getBinding(uint32_t begin, uint32_t end) const
        {
            TL_ASSERT(valid());
            return {
                .buffer = m_buffer,
                .offset = getOffsetBytes(begin),
                .range  = (end - begin) * getElementsStride(),
            };
        }

        inline RHI::BufferBindingInfo getBinding(uint32_t begin) const
        {
            TL_ASSERT(valid());
            return {
                .buffer = m_buffer,
                .offset = getOffsetBytes(begin),
                .range  = (m_elementsCount - begin) * getElementsStride(),
            };
        }

        const OffsetAllocator::Allocation& allocation() const
        {
            return m_allocation;
        }

    private:
        RHI::Buffer*                m_buffer        = nullptr;
        OffsetAllocator::Allocation m_allocation    = {};
        uint32_t                    m_stride        = 0;
        uint32_t                    m_elementsCount = 0;
    };

    // Generic buffer pool that can be allocated from
    class BufferPool
    {
    public:
        BufferPool()
        {
            m_allocator = TL::CreatePtr<OffsetAllocator::Allocator>();
        }

        using sizeType          = uint32_t;
        using StorageReport     = OffsetAllocator::StorageReport;
        using StorageReportFull = OffsetAllocator::StorageReportFull;

        void init(RHI::Device* device, TL::StringView name, TL::Flags<RHI::BufferUsage> usage, size_t sizeBytes);
        void shutdown();

        // Allocate a buffer range for elements of type T with alignment
        template<typename T>
        Buffer<T> allocate(uint32_t count = 1, uint32_t minOffsetAlignment = sizeof(T))
        {
            TL_ASSERT(minOffsetAlignment != 0, "Requested stride must not be zero");

            size_t size       = alignUp(sizeof(T), minOffsetAlignment) * count;
            auto   allocation = m_allocator->allocate(size);

            TL_ASSERT(allocation.offset != OffsetAllocator::Allocation::NO_SPACE, "No space!");
            if (allocation.offset == OffsetAllocator::Allocation::NO_SPACE)
            {
                TL_UNREACHABLE();
                return {};
            }
            // allocation.offset is already aligned by allocator
            return Buffer<T>(m_buffer, allocation, minOffsetAlignment, count);
        }

        // Free a previously allocated buffer range
        template<typename T>
        void free(Buffer<T>& allocation)
        {
            TL_ASSERT(allocation.valid());
            m_allocator->free(allocation.allocation());
            allocation = {};
        }

        void update(uint32_t offset, TL::Block block);

        // Update contents of a BufferRange<T> at given element index
        template<typename T>
        void update(Buffer<T> allocation, uint32_t atIndex, TL::Span<const T> data)
        {
            auto expecteElementSize = sizeof(T);
            auto actualElementSize  = allocation.getElementsStride();

            TL_ASSERT(expecteElementSize == actualElementSize);
            TL_ASSERT(allocation.getElementsCount() - atIndex >= data.size());

            update(allocation.getOffsetBytes(atIndex), TL::Block::fromSpan(data));
        }

        // convenience overload for single-element updates
        template<typename T>
        void update(Buffer<T> allocation, const T& data)
        {
            update(allocation.getOffsetBytes(), TL::Block::create(data));
        }

        // Get the size of the buffer allocation (in bytes)
        template<typename T>
        size_t getBufferSize(const Buffer<T>& buffer) const
        {
            return m_allocator->allocationSize(buffer.allocation());
        }

        StorageReport     storageReport() const;
        StorageReportFull storageReportFull() const;

        const RHI::Buffer* getBuffer() const;

        RHI::Buffer* getBuffer();

    private:
        RHI::Device*                        m_device;
        RHI::Buffer*                        m_buffer = nullptr;
        TL::Ptr<OffsetAllocator::Allocator> m_allocator;
    };

    // Array allocator

    template<typename T>
    class GPUArrayAllocation
    {
    public:
        GPUArrayAllocation() = default;

        GPUArrayAllocation(Buffer<T> buffer, uint32_t offsetIndex, uint32_t count)
            : m_buffer(buffer)
            , m_offset(offsetIndex)
            , m_count(count)
        {
        }

        inline uint32_t getOffsetBytes() const
        {
            TL_ASSERT(valid());
            return m_buffer.getOffsetBytes(m_offset);
        }

        inline uint32_t getOffsetBytes(uint32_t element) const
        {
            TL_ASSERT(valid());
            TL_ASSERT(element < m_count);
            return m_buffer.getOffsetBytes(m_offset + element);
        }

        inline uint32_t getOffsetIndex() const
        {
            TL_ASSERT(valid());
            return m_offset;
        }

        inline uint32_t getOffsetIndex(uint32_t element) const
        {
            TL_ASSERT(valid());
            TL_ASSERT(element < m_count);
            return m_offset + element;
        }

        inline uint32_t getSizeBytes() const
        {
            TL_ASSERT(valid());
            return m_buffer.getElementsStride() * getElementsCount();
        }

        inline uint32_t getElementsStride() const
        {
            TL_ASSERT(valid());
            return m_buffer.getElementsStride();
        }

        inline uint32_t getElementsCount() const
        {
            return m_count;
        }

        bool valid() const
        {
            return m_buffer.valid() && m_count > 0;
        }

        RHI::BufferBindingInfo getBinding()
        {
            TL_ASSERT(valid());
            return m_buffer.getBinding(m_offset);
        }

        const RHI::BufferBindingInfo getBinding() const
        {
            TL_ASSERT(valid());
            return m_buffer.getBinding(m_offset, m_offset + m_count);
        }

    private:
        Buffer<T> m_buffer;
        uint32_t  m_offset = 0; // element offset in parent
        uint32_t  m_count  = 0; // element count
    };

    template<typename T>
    class GPUArray
    {
    public:
        GPUArray() = default;

        void init(BufferPool& pool, uint32_t maxElementsCapacity)
        {
            m_pool   = &pool;
            m_buffer = pool.allocate<T>(maxElementsCapacity);
            m_freeList.push_back({0, m_buffer.getElementsCount()});
        }

        void shutdown()
        {
            if (m_pool)
                m_pool->free(m_buffer);
            m_buffer = {};
        }

        GPUArrayAllocation<T> allocate(uint32_t count)
        {
            for (auto it = m_freeList.begin(); it != m_freeList.end(); ++it)
            {
                auto& block = *it;
                if (block.size >= count)
                {
                    uint32_t offset = block.offset;

                    // Shrink block
                    block.offset += count;
                    block.size -= count;

                    if (block.size == 0)
                        m_freeList.erase(it);

                    return GPUArrayAllocation<T>(m_buffer, offset, count);
                }
            }

            TL_ASSERT(false, "No space!");
            return {};
        }

        void free(GPUArrayAllocation<T>& alloc)
        {
            if (!alloc.valid()) return;

            FreeBlock block{alloc.getOffsetIndex(), alloc.getElementsCount()};
            auto      it = std::lower_bound(
                m_freeList.begin(), m_freeList.end(), block, [](const FreeBlock& a, const FreeBlock& b)
                {
                    return a.offset < b.offset;
                });

            // Insert in sorted order
            it = m_freeList.insert(it, block);

            // Coalesce with neighbors
            coalesce();

            alloc = {};
        }

        void reset()
        {
            m_freeList.clear();
            m_freeList.push_back({0, m_buffer.getElementsCount()});
        }

        uint32_t capacity() const
        {
            return m_buffer.getElementsCount();
        }

        void update(GPUArrayAllocation<T> allocation, TL::Span<const T> data)
        {
            auto elementsCount = allocation.getElementsCount();

            TL_ASSERT(elementsCount >= data.size());

            m_pool->update(allocation.getOffsetBytes(), TL::Block::fromSpan(data));
        }

        inline RHI::BufferBindingInfo getBinding() const
        {
            // TL_ASSERT(valid());
            return m_buffer.getBinding();
        }

        operator RHI::BufferBindingInfo() const
        {
            return getBinding();
        }

    private:
        struct FreeBlock
        {
            uint32_t offset;
            uint32_t size;
        };

        BufferPool*           m_pool;
        Buffer<T>             m_buffer;
        TL::Vector<FreeBlock> m_freeList;

        void coalesce()
        {
            if (m_freeList.empty()) return;

            std::sort(m_freeList.begin(), m_freeList.end(), [](const FreeBlock& a, const FreeBlock& b)
                {
                    return a.offset < b.offset;
                });

            TL::Vector<FreeBlock> merged;
            merged.reserve(m_freeList.size());

            FreeBlock cur = m_freeList[0];
            for (size_t i = 1; i < m_freeList.size(); i++)
            {
                auto& next = m_freeList[i];
                if (cur.offset + cur.size == next.offset)
                {
                    // merge
                    cur.size += next.size;
                }
                else
                {
                    merged.push_back(cur);
                    cur = next;
                }
            }
            merged.push_back(cur);

            m_freeList.swap(merged);
        }
    };

} // namespace Engine
