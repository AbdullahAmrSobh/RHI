#pragma once

#include <TL/Utils.hpp>
#include <TL/OffsetAllocator/OffsetAllocator.hpp>

#include "Renderer/Common.hpp"
#include <cassert>

namespace Engine
{
    // buffer range that contains elements of type T
    template<typename T>
    class Buffer
    {
    public:
        Buffer() = default;

        inline const RHI::Buffer* getBuffer() const
        {
            return m_buffer;
        }

        // Get the underlying buffer
        inline RHI::Buffer* getBuffer()
        {
            return m_buffer;
        }

        inline size_t getOffset() const
        {
            return m_allocation.offset;
        }

        inline uint32_t getStride() const
        {
            return m_stride;
        }

        inline bool valid() const
        {
            return m_buffer != nullptr && m_allocation.offset != OffsetAllocator::Allocation::NO_SPACE;
        }

        inline operator bool() const
        {
            return m_buffer != nullptr && m_allocation.offset != OffsetAllocator::Allocation::NO_SPACE;
        }

        inline operator RHI::BufferBindingInfo() const
        {
            TL_ASSERT(valid());
            return {m_buffer, m_allocation.offset};
        }

    private:
        friend class BufferPool;

        template<typename U>
        friend void bufferWrite(BufferPool&, Buffer<U>, TL::Span<const U>);

        template<typename U>
        friend void bufferWrite(BufferPool&, Buffer<U>, size_t, TL::Span<const U>);

        RHI::Buffer*                m_buffer     = nullptr;
        OffsetAllocator::Allocation m_allocation = {};
        uint32_t                    m_stride     = 0;
    };

    // Buffer pool manages RHI buffer and allocate subregion within it
    class BufferPool
    {
    public:
        BufferPool() = default;

        void init(RHI::Device* device, const RHI::BufferCreateInfo& createInfo);

        void shutdown();

        // Accessor for the underlying device (useful for bufferWrite helpers)
        RHI::Device* getDevice() const { return m_device; }

        // Allocate a buffer range for T elements (element size is sizeof(T))
        template<typename T>
        Buffer<T> allocate(size_t count, uint32_t elementAlignment = alignof(T))
        {
            // elementAlignment is the per-element alignment requested by the caller (e.g. constant buffer alignment)
            uint32_t alignedElementSize = TL::AlignUp<uint32_t>(sizeof(T), elementAlignment);
            auto     totalSizeBytes     = uint32_t(alignedElementSize * count);
            auto     allocation         = m_allocator.allocate(totalSizeBytes);
            if (allocation.offset == OffsetAllocator::Allocation::NO_SPACE)
            {
                return Buffer<T>();
            }

            Buffer<T> out;
            out.m_allocation = allocation;
            out.m_buffer     = m_buffer;
            out.m_stride     = elementAlignment; // store requested alignment (used for aligned element size computation)
            return out;
        }

        // Free a previously allocated buffer range
        template<typename T>
        void free(Buffer<T> allocation)
        {
            if (!allocation.valid())
                return;
            m_allocator.free(allocation.m_allocation);
        }

        // Get the size of the buffer allocation
        template<typename T>
        size_t getBufferSize(Buffer<T> buffer) const
        {
            return m_allocator.allocationSize(buffer.m_allocation);
        }

        // Get number of elements in buffer
        template<typename T>
        uint32_t getBufferElementsCount(Buffer<T> buffer) const
        {
            if (!buffer.valid()) return 0;
            return uint32_t(getBufferSize(buffer) / buffer.getStride());
        }

    private:
        RHI::Device*               m_device = nullptr;
        RHI::Buffer*               m_buffer = nullptr;
        OffsetAllocator::Allocator m_allocator;
    };

    // --- Resource wrappers ---

    template<typename T>
    struct StructuredBuffer
    {
        friend class GpuSceneData;
        template<typename U>
        friend void bufferWrite(BufferPool&, StructuredBuffer<U>&, uint32_t, const U&);
        template<typename U>
        friend StructuredBuffer<U> createStructuredBuffer(BufferPool&, size_t, uint32_t);
        template<typename U>
        friend void freeStructuredBuffer(BufferPool&, StructuredBuffer<U>&);

        StructuredBuffer() = default;

        // Constructor to initialize from a Buffer allocation
        StructuredBuffer(Buffer<T> buffer, size_t capacity)
            : m_buffer(buffer)
            , m_capacity(capacity)
            , m_count(0)
        {
        }

        // todo: rename to Index
        using Allocation = uint32_t;

        // Allocate a new element in the structured buffer
        Allocation allocate()
        {
            if (!m_buffer.valid())
                return UINT32_MAX;

            if (m_freeList.empty())
            {
                if (m_count >= m_capacity)
                {
                    return UINT32_MAX; // No space available
                }

                return static_cast<Allocation>(m_count++);
            }
            else
            {
                auto index = m_freeList.back();
                m_freeList.pop_back();
                return index;
            }
        }

        // Free a previously allocated element
        void free(Allocation allocation)
        {
            if (allocation != UINT32_MAX && allocation < m_count)
            {
                m_freeList.push_back(allocation);
            }
        }

        // Get the underlying buffer
        RHI::Buffer* getBuffer() const
        {
            return m_buffer.getBuffer();
        }

        // Get the offset within the buffer
        size_t getOffset() const
        {
            return m_buffer.getOffset();
        }

        // Get the offset of a specific element
        size_t getOffset(Allocation element) const
        {
            // Element offset must account for per-element alignment used when allocating the buffer
            size_t elementSize = m_buffer.getAlignedElementSize();
            return m_buffer.getOffset() + (elementSize * element);
        }

        // Get the number of allocated elements
        uint32_t getCount() const
        {
            return uint32_t(m_count);
        }

        // Get the maximum number of elements
        uint32_t getCapacity() const
        {
            return uint32_t(m_capacity);
        }

        // Get the size of each element (un-aligned)
        size_t getElementSize() const
        {
            return sizeof(T);
        }

        operator RHI::BufferBindingInfo() const
        {
            return {m_buffer.getBuffer(), m_buffer.getOffset()};
        }

    private:
        Buffer<T>            m_buffer;
        size_t               m_capacity = 0;
        size_t               m_count    = 0;
        TL::Vector<uint32_t> m_freeList;
    };

    template<typename T>
    struct ConstantBuffer
    {
        friend class GpuSceneData;
        template<typename U>
        friend void bufferWrite(BufferPool&, ConstantBuffer<U>, const U&);
        template<typename U>
        friend ConstantBuffer<U> createConstantBuffer(BufferPool&, uint32_t);
        template<typename U>
        friend void freeConstantBuffer(BufferPool&, ConstantBuffer<U>&);

        ConstantBuffer() = default;

        // Constructor to initialize from a Buffer allocation
        ConstantBuffer(Buffer<T> buffer)
            : m_buffer(buffer)
        {
        }

        // Get the underlying buffer
        RHI::Buffer* getBuffer()
        {
            return m_buffer.getBuffer();
        }

        // Get the offset within the buffer
        size_t getOffset() const
        {
            return m_buffer.getOffset();
        }

    private:
        Buffer<T> m_buffer;
    };

    template<typename T>
    struct DynamicConstantBuffer
    {
        friend class GpuSceneData;
        template<typename U>
        friend void bufferWrite(BufferPool&, DynamicConstantBuffer<U>, uint32_t, const U&);
        template<typename U>
        friend DynamicConstantBuffer<U> CreateDynamicConstantBuffer(BufferPool&, size_t);
        template<typename U>
        friend void FreeDynamicConstantBuffer(BufferPool&, DynamicConstantBuffer<U>&);

        struct Element
        {
            friend class GpuSceneData;
            friend DynamicConstantBuffer<T>;

            Element() = default;

        private:
            uint32_t index = 0;
        };

        DynamicConstantBuffer() = default;

        // Constructor to initialize from a Buffer allocation (buffer must be a raw byte buffer)
        DynamicConstantBuffer(Buffer<char> buffer, size_t count, size_t perElementAlignment)
            : m_buffer(buffer)
            , m_count(uint32_t(count))
            , m_alignment(perElementAlignment)
        {
        }

        // Get the underlying buffer
        RHI::Buffer* getBuffer()
        {
            return m_buffer.getBuffer();
        }

        // Get the offset within the buffer (base offset)
        size_t getOffset() const
        {
            return m_buffer.getOffset();
        }

        // Get the offset of element idx within the buffer
        size_t getOffset(uint32_t idx) const
        {
            return m_buffer.getOffset() + (m_alignment * idx);
        }

        // Get the number of elements
        uint32_t getCount() const
        {
            return m_count;
        }

        // Get the alignment (per-element stride)
        size_t getAlignment() const
        {
            return m_alignment;
        }

        // Get the size of each logical element (sizeof(T))
        size_t getElementSize() const
        {
            return sizeof(T);
        }

        // Get the offset of a specific element
        size_t getOffset(const Element& element) const
        {
            return m_buffer.getOffset() + (TL::AlignUp<uint32_t>(getElementSize(), m_alignment) * element.index);
        }

    private:
        Buffer<char> m_buffer;
        uint32_t     m_count     = 0;
        size_t       m_alignment = 0; // stride in bytes (already aligned to the dynamic alignment requirement)
    };

    // --- Convenience create/free functions ---

    // Create a structured buffer from the pool. Returns an empty StructuredBuffer if allocation fails.
    template<typename T>
    StructuredBuffer<T> createStructuredBuffer(BufferPool& pool, size_t capacity, uint32_t elementAlignment = alignof(T))
    {
        auto buffer = pool.allocate<T>(capacity, elementAlignment);
        if (!buffer.valid())
            return StructuredBuffer<T>();
        return StructuredBuffer<T>(buffer, capacity);
    }

    template<typename T>
    void freeStructuredBuffer(BufferPool& pool, StructuredBuffer<T>& buffer)
    {
        pool.free(buffer.m_buffer);
        buffer = StructuredBuffer<T>();
    }

    // Create a constant buffer for a single element
    template<typename T>
    ConstantBuffer<T> createConstantBuffer(BufferPool& pool, uint32_t elementAlignment = alignof(T))
    {
        auto cb = pool.allocate<T>(1, elementAlignment);
        if (!cb.valid())
            return ConstantBuffer<T>();
        return ConstantBuffer<T>(cb);
    }

    template<typename T>
    void freeConstantBuffer(BufferPool& pool, ConstantBuffer<T>& buffer)
    {
        pool.free(buffer.m_buffer);
        buffer = ConstantBuffer<T>();
    }

    // Create a dynamic constant buffer. dynamicAlignment is the GPU required alignment for dynamic CB offsets
    template<typename T>
    DynamicConstantBuffer<T> createDynamicConstantBuffer(BufferPool& pool, size_t count)
    {
        // stride = AlignUp(sizeof(T), dynamicAlignment)
        auto minAlignment = pool.getDevice()->GetLimits().minUniformBufferOffsetAlignment;

        uint32_t stride = TL::AlignUp<uint32_t>(uint32_t(sizeof(T)), minAlignment);
        size_t   total  = size_t(stride) * count;
        auto     raw    = pool.allocate<char>(total, stride);
        if (!raw.valid())
            return DynamicConstantBuffer<T>();
        return DynamicConstantBuffer<T>(raw, count, stride);
    }

    template<typename T>
    void freeDynamicConstantBuffer(BufferPool& pool, DynamicConstantBuffer<T>& dcb)
    {
        pool.free(dcb.m_buffer);
        dcb = DynamicConstantBuffer<T>();
    }

    template<typename T>
    inline void bufferWrite(BufferPool& pool, Buffer<T> buffer, TL::Span<const T> data)
    {
        TL_ASSERT(pool.getDevice() != nullptr);
        TL_ASSERT(buffer.valid());
        // Ensure buffer alignment can contain T
        TL_ASSERT(buffer.getStride() == alignof(T));
        auto frame = pool.getDevice()->GetCurrentFrame();
        frame->BufferWrite(
            buffer.getBuffer(),
            buffer.getOffset(),
            TL::Block::fromSpan(data));
    }

    template<typename T>
    inline void bufferWrite(BufferPool& pool, Buffer<T> buffer, size_t offset, TL::Span<const T> data)
    {
        TL_ASSERT(pool.getDevice() != nullptr);
        TL_ASSERT(buffer.valid());
        TL_ASSERT(buffer.getStride() == alignof(T));
        auto frame = pool.getDevice()->GetCurrentFrame();

        frame->BufferWrite(
            buffer.getBuffer(),
            buffer.getOffset() + (buffer.getAlignedElementSize() * offset),
            TL::Block::fromSpan(data));
    }

    template<typename T>
    inline void bufferWrite(BufferPool& pool, ConstantBuffer<T> buffer, const T& data)
    {
        TL_ASSERT(pool.getDevice() != nullptr);
        auto frame = pool.getDevice()->GetCurrentFrame();
        frame->BufferWrite(buffer.getBuffer(), buffer.getOffset(), TL::Block::create(data));
    }

    template<typename T>
    inline void bufferWrite(BufferPool& pool, DynamicConstantBuffer<T> buffer, uint32_t index, const T& data)
    {
        TL_ASSERT(pool.getDevice() != nullptr);
        TL_ASSERT(index < buffer.getCount());
        auto frame = pool.getDevice()->GetCurrentFrame();
        frame->BufferWrite(buffer.getBuffer(), buffer.getOffset(index), TL::Block::create(data));
    }

    template<typename T>
    inline void bufferWrite(BufferPool& pool, StructuredBuffer<T>& buffer, uint32_t index, const T& data)
    {
        TL_ASSERT(pool.getDevice() != nullptr);
        TL_ASSERT(index < buffer.getCount());
        auto frame = pool.getDevice()->GetCurrentFrame();
        frame->BufferWrite(buffer.getBuffer(), buffer.getOffset(index), TL::Block::create(data));
    }

} // namespace Engine
