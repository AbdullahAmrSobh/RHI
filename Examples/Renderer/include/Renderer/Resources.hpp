#pragma once

#include <TL/Utils.hpp>
#include <TL/OffsetAllocator/OffsetAllocator.hpp>

#include <RHI/RHI.hpp>

#include "Renderer/Common.hpp"

namespace Engine
{
    // buffer range that contains elements of type T
    class Buffer
    {
    public:
        friend class BufferPool;

        Buffer() = default;

        Buffer(RHI::Buffer* buffer, OffsetAllocator::Allocation allocation, uint32_t elementSize, uint32_t elementCount)
            : m_buffer(buffer)
            , m_allocation(allocation)
            , m_elementSize(elementSize)
            , m_elementsCount(elementCount)
        {
            TL_ASSERT(elementSize != 0, "Buffer elementSize must not be zero");
            TL_ASSERT(elementCount != 0, "Buffer elementCount must not be zero");
        }

        inline const RHI::Buffer* getBuffer() const
        {
            return m_buffer;
        }

        inline RHI::Buffer* getBuffer()
        {
            return m_buffer;
        }

        inline uint32_t getOffset() const
        {
            return m_allocation.offset;
        }

        inline uint32_t getOffsetAtElement(uint32_t element) const
        {
            return getOffset() + (m_elementSize * element);
        }

        inline RHI::BufferBindingInfo getBindingAt(uint32_t element)
        {
            return {
                .buffer = getBuffer(),
                .offset = getOffsetAtElement(element),
            };
        }

        inline uint32_t getByteSize() const
        {
            return m_elementSize * m_elementsCount;
        }

        inline uint32_t getStride() const
        {
            return m_elementSize;
        }

        inline uint32_t getCount() const
        {
            return m_elementsCount;
        }

        inline bool valid() const
        {
            return m_buffer != nullptr && m_allocation.offset != OffsetAllocator::Allocation::NO_SPACE;
        }

        inline operator bool() const
        {
            return valid();
        }

        inline operator RHI::BufferBindingInfo() const
        {
            TL_ASSERT(valid());
            return {m_buffer, m_allocation.offset};
        }

    private:
        RHI::Buffer*                m_buffer        = nullptr;
        OffsetAllocator::Allocation m_allocation    = {};
        // size in bytes = elementSize * elementCount !
        // size of a single element in the buffer (element-width)
        uint32_t                    m_elementSize   = 0;
        // number of elements in the buffer
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

        // Allocate a buffer range for T elements (element size is sizeof(T))
        Buffer allocate(uint32_t count, uint32_t elementSize)
        {
            auto allocation = m_allocator->allocate(count * elementSize);
            if (allocation.offset == OffsetAllocator::Allocation::NO_SPACE)
            {
                return {};
            }
            else
            {
                // protect against stride == 0 here too
                TL_ASSERT(elementSize != 0, "Requested stride must not be zero");
                return {m_buffer, allocation, elementSize, count};
            }
        }

        // Free a previously allocated buffer range
        void free(Buffer allocation)
        {
            TL_ASSERT(allocation.valid());
            m_allocator->free(allocation.m_allocation);
        }

        // Get the size of the buffer allocation (in bytes)
        size_t getBufferSize(Buffer buffer) const
        {
            return m_allocator->allocationSize(buffer.m_allocation);
        }

        StorageReport storageReport() const
        {
            return m_allocator->storageReport();
        }

        StorageReportFull storageReportFull() const
        {
            return m_allocator->storageReportFull();
        }

    protected:
        RHI::Buffer*                        m_buffer = nullptr;
        TL::Ptr<OffsetAllocator::Allocator> m_allocator;
    };

    template<typename T>
    using GpuArrayAllocation = Buffer;

    // Could be Vertex/Index/Structured buffer of elements of type T ...
    template<uint32_t ID>
    class GpuArray : private BufferPool
    {
    public:
        using sizeType          = uint32_t;
        using StorageReport     = OffsetAllocator::StorageReport;
        using StorageReportFull = OffsetAllocator::StorageReportFull;

        template<typename T>
        using Allocation = GpuArrayAllocation<T>;

        RHI::BufferBindingInfo getBaseBinding()
        {
            return {BufferPool::m_buffer, 0};
        }

        void init(RHI::Device* device, TL::StringView name, uint32_t size, TL::Flags<RHI::BufferUsage> usageFlags)
        {
            RHI::BufferCreateInfo createInfo{
                .name       = name.data(),
                .hostMapped = true,
                .usageFlags = usageFlags,
                .byteSize   = size,
            };
            BufferPool::init(device, createInfo);
        }

        void shutdown(RHI::Device* device)
        {
            BufferPool::shutdown(device);
        }

        template<typename T>
        void update(RHI::Device* device, Allocation<T> allocation, uint32_t atIndex, TL::Span<const T> data)
        {
            TL_ASSERT(atIndex < allocation.getCount());
            TL_ASSERT(sizeof(T) == allocation.getStride(), "TODO: Addd message here :D");
            device->GetCurrentFrame()->BufferWrite(allocation.getBuffer(), allocation.getOffsetAtElement(atIndex), TL::Block::create(data));
        }

        template<typename T>
        Allocation<T> allocate(uint32_t count)
        {
            return BufferPool::allocate(count, sizeof(T));
        }

        template<typename T>
        void free(Allocation<T> allocation)
        {
            BufferPool::free(allocation);
        }

        StorageReport storageReport() const
        {
            return BufferPool::storageReport();
        }

        StorageReportFull storageReportFull() const
        {
            return BufferPool::storageReportFull();
        }
    };

    using StructuredBufferPool = GpuArray<0>;

    template<typename T>
    using StructuredBuffer = typename GpuArray<0>::Allocation<T>;

    static StructuredBufferPool createStructuredBufferPool(RHI::Device* device, uint32_t sizeBytes)
    {
        StructuredBufferPool pool;
        pool.init(device, "StructuredBuffer", sizeBytes, RHI::BufferUsage::Storage);
        return pool;
    }

    static void freeStructuredBufferPool(RHI::Device* device, StructuredBufferPool& pool)
    {
        pool.shutdown(device);
    }

    // Make mesh buffer pool non templated ...
    using MeshBufferPool = GpuArray<1>;

    template<typename T>
    using MeshBuffer = typename GpuArray<1>::Allocation<T>;

    static MeshBufferPool createMeshBufferPool(RHI::Device* device, uint32_t sizeBytes)
    {
        MeshBufferPool pool;
        pool.init(device, "MeshBuffer", sizeBytes, RHI::BufferUsage::Vertex | RHI::BufferUsage::Index);
        return pool;
    }

    static void freeMeshBufferPool(RHI::Device* device, MeshBufferPool& pool)
    {
        pool.shutdown(device);
    }

    template<typename T>
    using ConstantBuffer = Buffer;

    class ConstantBufferPool : private BufferPool
    {
    public:
        ConstantBufferPool() = default;

        inline void init(RHI::Device* device, TL::StringView name, uint32_t byteSize)
        {
            const uint32_t m_alignment = device->GetLimits().minUniformBufferOffsetAlignment;

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

        /// Allocate raw region (caller chooses size, must be <= stride-aligned)
        template<typename T>
        inline ConstantBuffer<T> allocate(uint32_t count = 1)
        {
            uint32_t stride = TL::AlignUp<uint32_t>(sizeof(T), m_alignment);
            return BufferPool::allocate(count, stride); // count=1, stride=aligned size
        }

        template<typename T>
        inline void free(ConstantBuffer<T> allocation)
        {
            BufferPool::free(allocation);
        }

        template<typename T>
        void update(RHI::Device* device, ConstantBuffer<T> allocation, uint32_t index, const T& data)
        {
            TL_ASSERT(index < allocation.getCount());
            device->GetCurrentFrame()->BufferWrite(allocation.getBuffer(), allocation.getOffsetAtElement(index), TL::Block::create(data));
        }

        template<typename T>
        void update(RHI::Device* device, ConstantBuffer<T> allocation, const T& data)
        {
            update(device, allocation, 0, data);
        }

        inline uint32_t alignment() const
        {
            return m_alignment;
        }

    private:
        uint32_t m_alignment = 256;
    };

    static ConstantBufferPool createConstantBufferPool(RHI::Device* device, uint32_t size)
    {
        ConstantBufferPool pool;
        pool.init(device, "ConstantBuffer", size);
        return pool;
    }

    static void freeConstantBufferPool(RHI::Device* device, ConstantBufferPool& pool)
    {
        pool.shutdown(device);
    }
} // namespace Engine
