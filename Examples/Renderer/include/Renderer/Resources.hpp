#pragma once

#include <TL/Utils.hpp>
#include <TL/OffsetAllocator/OffsetAllocator.hpp>

#include <RHI/RHI.hpp>

#include <algorithm>

namespace Engine
{
    // TODO: Format, split into .hpp + .inl files

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
            , m_elementSize(elementSize)
            , m_elementsCount(elementCount)
        {
            TL_ASSERT(elementSize != 0, "Buffer elementSize must not be zero");
            TL_ASSERT(elementCount != 0, "Buffer elementCount must not be zero");
        }

        inline const RHI::Buffer* getBuffer() const { return m_buffer; }

        inline RHI::Buffer* getBuffer() { return m_buffer; }

        inline uint32_t getOffset() const { return m_allocation.offset; }

        inline uint32_t getOffsetAtElement(uint32_t element) const
        {
            TL_ASSERT(element < m_elementsCount);
            return getOffset() + (m_elementSize * element);
        }

        inline RHI::BufferBindingInfo getBindingAt(uint32_t element) const
        {
            return {
                .buffer = m_buffer,
                .offset = getOffsetAtElement(element),
            };
        }

        inline uint32_t getByteSize() const { return m_elementSize * m_elementsCount; }

        inline uint32_t getStride() const { return m_elementSize; }

        inline uint32_t getCount() const { return m_elementsCount; }

        inline bool valid() const
        {
            return m_buffer != nullptr && m_allocation.offset != OffsetAllocator::Allocation::NO_SPACE;
        }

        inline operator bool() const { return valid(); }

        inline operator RHI::BufferBindingInfo() const
        {
            TL_ASSERT(valid());
            return {m_buffer, m_allocation.offset};
        }

        // expose allocation for allocator queries (intended for BufferPool)
        const OffsetAllocator::Allocation& allocation() const { return m_allocation; }

    private:
        RHI::Buffer*                m_buffer        = nullptr;
        OffsetAllocator::Allocation m_allocation    = {};
        uint32_t                    m_elementSize   = 0;
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

        void init(RHI::Device* device, const RHI::BufferCreateInfo& createInfo);
        void shutdown(RHI::Device* device);

        // Allocate a buffer range for elements of type T
        template<typename T>
        Buffer<T> allocate(uint32_t count)
        {
            static_assert(!std::is_void_v<T>, "T must be a concrete type");
            const uint32_t elementSize = static_cast<uint32_t>(sizeof(T));
            TL_ASSERT(elementSize != 0, "Requested stride must not be zero");
            auto allocation = m_allocator->allocate(count * elementSize);
            if (allocation.offset == OffsetAllocator::Allocation::NO_SPACE)
            {
                return {};
            }
            return Buffer<T>(m_buffer, allocation, elementSize, count);
        }

        // Free a previously allocated buffer range
        template<typename T>
        void free(Buffer<T> allocation)
        {
            TL_ASSERT(allocation.valid());
            m_allocator->free(allocation.allocation());
        }

        // Update contents of a BufferRange<T> at given element index
        template<typename T>
        void update(RHI::Device* device, Buffer<T> allocation, uint32_t atIndex, TL::Span<const T> data)
        {
            TL_ASSERT(allocation.valid());
            TL_ASSERT(atIndex < allocation.getCount());
            TL_ASSERT(sizeof(T) == allocation.getStride(), "Size mismatch in update");
            device->GetCurrentFrame()->BufferWrite(allocation.getBuffer(),
                allocation.getOffsetAtElement(atIndex),
                TL::Block::create(data));
        }

        // convenience overload for single-element updates
        template<typename T>
        void update(RHI::Device* device, Buffer<T> allocation, const T& data)
        {
            update(device, allocation, 0, TL::Span<const T>(&data, 1));
        }

        // Get the size of the buffer allocation (in bytes)
        template<typename T>
        size_t getBufferSize(const Buffer<T>& buffer) const
        {
            return m_allocator->allocationSize(buffer.allocation());
        }

        StorageReport storageReport() const { return m_allocator->storageReport(); }

        StorageReportFull storageReportFull() const { return m_allocator->storageReportFull(); }

        // expose the underlying RHI buffer for binding base
        RHI::Buffer* underlyingBuffer() { return m_buffer; }

        const RHI::Buffer* underlyingBuffer() const { return m_buffer; }

    protected:
        RHI::Buffer*                        m_buffer = nullptr;
        TL::Ptr<OffsetAllocator::Allocator> m_allocator;
    };

    // Constant buffer pool needs to align per-device requirements
    class ConstantBufferPool : private BufferPool
    {
    public:
        ConstantBufferPool() = default;

        inline void init(RHI::Device* device, TL::StringView name, uint32_t byteSize)
        {
            m_alignment = device->GetLimits().minUniformBufferOffsetAlignment;
            RHI::BufferCreateInfo createInfo{
                .name       = name.data(),
                .hostMapped = true,
                .usageFlags = RHI::BufferUsage::Uniform,
                .byteSize   = TL::AlignUp(byteSize, m_alignment),
            };
            BufferPool::init(device, createInfo);
        }

        inline void shutdown(RHI::Device* device)
        {
            BufferPool::shutdown(device);
        }

        /// Allocate raw region (caller chooses count of T; stride aligned to UB offset alignment)
        template<typename T>
        Buffer<T> allocate(uint32_t count = 1)
        {
            uint32_t stride     = TL::AlignUp<uint32_t>(sizeof(T), m_alignment);
            // Use allocator directly: allocate(count * stride)
            auto     allocation = m_allocator->allocate(count * stride);
            if (allocation.offset == OffsetAllocator::Allocation::NO_SPACE)
            {
                return {};
            }
            return Buffer<T>(m_buffer, allocation, stride, count);
        }

        template<typename T>
        void free(Buffer<T> allocation)
        {
            BufferPool::free(allocation);
        }

        template<typename T>
        void update(RHI::Device* device, Buffer<T> allocation, uint32_t index, const T& data)
        {
            TL_ASSERT(index < allocation.getCount());
            device->GetCurrentFrame()->BufferWrite(allocation.getBuffer(),
                allocation.getOffsetAtElement(index),
                TL::Block::create(data));
        }

        template<typename T>
        void update(RHI::Device* device, Buffer<T> allocation, const T& data)
        {
            update(device, allocation, 0, data);
        }

        inline uint32_t alignment() const { return m_alignment; }

    private:
        uint32_t m_alignment;
    };

    // Convenience factory functions (keeps same usage style as before)
    inline BufferPool createStructuredBufferPool(RHI::Device* device, uint32_t sizeBytes)
    {
        BufferPool            pool;
        RHI::BufferCreateInfo createInfo{
            .name       = "StructuredBuffer",
            .hostMapped = true,
            .usageFlags = RHI::BufferUsage::Storage,
            .byteSize   = sizeBytes,
        };
        pool.init(device, createInfo);
        return pool;
    }

    inline void freeStructuredBufferPool(RHI::Device* device, BufferPool& pool)
    {
        pool.shutdown(device);
    }

    inline BufferPool createMeshBufferPool(RHI::Device* device, uint32_t sizeBytes)
    {
        BufferPool            pool;
        RHI::BufferCreateInfo createInfo{
            .name       = "MeshBuffer",
            .hostMapped = true,
            .usageFlags = RHI::BufferUsage::Vertex | RHI::BufferUsage::Index,
            .byteSize   = sizeBytes,
        };
        pool.init(device, createInfo);
        return pool;
    }

    inline void freeMeshBufferPool(RHI::Device* device, BufferPool& pool)
    {
        pool.shutdown(device);
    }

    inline ConstantBufferPool createConstantBufferPool(RHI::Device* device, uint32_t size)
    {
        ConstantBufferPool pool;
        pool.init(device, "ConstantBuffer", size);
        return pool;
    }

    inline void freeConstantBufferPool(RHI::Device* device, ConstantBufferPool& pool)
    {
        pool.shutdown(device);
    }

    // Array allocator

    template<typename T>
    class GPUArrayAllocation
    {
    public:
        GPUArrayAllocation() = default;

        GPUArrayAllocation(Buffer<T> parent, uint32_t offset, uint32_t count)
            : m_parent(parent)
            , m_offset(offset)
            , m_count(count)
        {
        }

        bool valid() const { return m_parent.valid() && m_count > 0; }

        uint32_t getCount() const { return m_count; }

        uint32_t getOffsetElementsRaw() const { return m_parent.getOffsetAtElement(m_offset); } // offset in elements

        uint32_t getOffsetElements() const { return m_offset; } // offset in elements

        uint32_t getStride() const { return sizeof(T); }

        // RHI::Buffer* getBuffer() const { return m_parent.getBuffer(); }

        auto getBuffer() const { return m_parent; }

        RHI::BufferBindingInfo getBinding() const
        {
            return {getBuffer(), m_parent.getOffsetAtElement(m_offset)};
        }

    private:
        Buffer<T> m_parent;
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
            m_buffer = pool.allocate<T>(maxElementsCapacity);
            m_freeList.push_back({0, m_buffer.getCount()});
        }

        void shutdown(BufferPool& pool)
        {
            pool.free(m_buffer);
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

            TL_UNREACHABLE_MSG("No space!");
            return {};
        }

        void free(const GPUArrayAllocation<T>& alloc)
        {
            if (!alloc.valid()) return;

            FreeBlock block{alloc.getOffsetElements(), alloc.getCount()};
            auto      it = std::lower_bound(
                m_freeList.begin(), m_freeList.end(), block, [](const FreeBlock& a, const FreeBlock& b)
                {
                    return a.offset < b.offset;
                });

            // Insert in sorted order
            it = m_freeList.insert(it, block);

            // Coalesce with neighbors
            coalesce();
        }

        void reset()
        {
            m_freeList.clear();
            m_freeList.push_back({0, m_buffer.getCount()});
        }

        uint32_t capacity() const { return m_buffer.getCount(); }

        // convenience overload for single-element updates
        void update(RHI::Device* device, BufferPool& pool, GPUArrayAllocation<T> allocation, TL::Span<const T> data)
        {
            pool.update(device, allocation.getBuffer(), 0, data);
        }

        operator RHI::BufferBindingInfo() const
        {
            return m_buffer;
        }

    private:
        struct FreeBlock
        {
            uint32_t offset;
            uint32_t size;
        };

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
