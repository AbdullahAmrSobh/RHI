#pragma once

#include <TL/Utils.hpp>

#include "Common.hpp"

namespace Engine
{
    /// @brief A buffer pool that manages sub-allocations from a single large buffer
    /// @details Allows efficient allocation and deallocation of smaller buffer regions from a pre-allocated buffer
    class BufferPool
    {
    public:
        BufferPool();

        /// @brief Initializes the buffer suballocator.
        /// @param device The device to create the buffer on.
        /// @param createInfo The buffer creation parameters.
        /// @return The result of the operation.
        ResultCode Init(RHI::Device& device, RHI::BufferCreateInfo& createInfo);

        /// @brief Cleans up and releases the buffer resources
        /// @param device The RHI device used to create the buffer
        void Shutdown();

        /// @brief Resets the allocator, invalidating all previous allocations.
        void Reset();

        /// @brief Allocates a region of memory from the buffer
        /// @param size The size in bytes to allocate
        /// @param alignment The required alignment of the allocation
        /// @return Result containing the suballocation details if successful
        Result<Suballocation> Allocate(size_t size, size_t alignment);

        /// @brief Release a previously allocated region
        /// @param allocation The suballocation to deallocate
        void Release(const Suballocation& allocation);

        /// @brief Gets the handle to the underlying buffer
        /// @return Handle to the RHI buffer being suballocated from
        RHI::Handle<RHI::Buffer> GetBuffer() const;

        void BeginUpdate();

        void EndUpdate();

        void Write(Suballocation suballocation, TL::Block block);

    protected:
        RHI::Device*             m_device;
        RHI::Handle<RHI::Buffer> m_buffer;
        Suballocator             m_allocator;
        RHI::DeviceMemoryPtr     m_mappedPtr;
    };

    // Forward declaration of GpuArray for use in the iterator
    template<typename T>
    class GpuArray;

    template<typename T>
    class GpuArrayHandle
    {
    public:
        // Standard iterator typedefs
        using iterator_category = std::forward_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;

        // Constructor
        GpuArrayHandle(GpuArray<T>* array, uint32_t index);

        // Dereference operators
        reference operator*() const;
        pointer   operator->() const;

        // Pre-increment operator
        GpuArrayHandle& operator++();
        // Post-increment operator
        GpuArrayHandle  operator++(int);

        // Equality/Inequality comparisons
        bool operator==(const GpuArrayHandle& other) const;
        bool operator!=(const GpuArrayHandle& other) const;

    private:
        GpuArray<T>* m_array;
        uint32_t     m_index;
    };

    template<typename T>
    class GpuArray
    {
    public:
        /// @brief Initializes the GPU array.
        /// @param device The device to create the GPU buffer on.
        /// @param createInfo The buffer creation parameters.
        /// @param capacity The maximum number of elements the array can hold.
        /// @return The result of the initialization.
        ResultCode Init(RHI::Device& device, const char* name, TL::Flags<RHI::BufferUsage> usageFlags, uint32_t capacity);

        /// @brief Shuts down the GPU array and releases associated resources.
        void Shutdown(RHI::Device& device);

        /// @brief Inserts an element into the GPU array.
        /// @param element The element to insert.
        /// @return The index at which the element was inserted, wrapped in a Result.
        Result<GpuArrayHandle<T>> Insert(const T& element);

        /// @brief Removes an element from the GPU array at the given index.
        /// @param index The index of the element to remove.
        void Remove(uint32_t index);

        void BeginUpdate(RHI::Device& device);
        void EndUpdate(RHI::Device& device);

        /// @brief Updates the element at the specified index.
        /// @param index The index of the element to update.
        /// @param element The new value for the element.
        /// @return The result of the update operation.
        ResultCode Update(GpuArrayHandle<T> iterator, const T& element);

        /// @brief Gets the binding info of the underlying GPU buffer for shader usage.
        /// @return The buffer binding info.
        RHI::BufferBindingInfo GetBindingInfo() const;

        /// @brief Gets the number of active elements stored in the GPU array.
        /// @return The current count of elements.
        uint32_t GetCount() const;

        /// @brief Gets the total capacity of the GPU array.
        /// @return The capacity of the array.
        uint32_t GetCapacity() const;

    private:
        // List of free indices available for new elements (for reuse when elements are removed)
        TL::Vector<uint32_t> m_freeList;

        // Offset into the buffer (in case if buffer is shared with other systems)
        size_t m_bufferOffset;

        // Handle to the GPU buffer that stores the array elements
        RHI::Handle<RHI::Buffer> m_buffer;

        // Mapped pointer to the device memory for CPU access (if applicable)
        RHI::DeviceMemoryPtr m_mappedPtr;

        // Current number of active elements
        uint32_t m_count = 0;

        // Total capacity (maximum number of elements) of the GPU array
        uint32_t m_capacity  = 0;
        uint32_t m_allocated = 0;
    };

    // --- GpuArrayHandle Implementation ---

    template<typename T>
    GpuArrayHandle<T>::GpuArrayHandle(GpuArray<T>* array, uint32_t index)
        : m_array(array)
        , m_index(index)
    {
    }

    template<typename T>
    typename GpuArrayHandle<T>::reference GpuArrayHandle<T>::operator*() const
    {
        // Assume that m_mappedPtr is a pointer to the beginning of the buffer.
        T* data = reinterpret_cast<T*>(static_cast<char*>(m_array->m_mappedPtr) + m_array->m_bufferOffset);
        return data[m_index];
    }

    template<typename T>
    typename GpuArrayHandle<T>::pointer GpuArrayHandle<T>::operator->() const
    {
        T* data = reinterpret_cast<T*>(static_cast<char*>(m_array->m_mappedPtr) + m_array->m_bufferOffset);
        return &data[m_index];
    }

    template<typename T>
    GpuArrayHandle<T>& GpuArrayHandle<T>::operator++()
    {
        ++m_index;
        return *this;
    }

    template<typename T>
    GpuArrayHandle<T> GpuArrayHandle<T>::operator++(int)
    {
        GpuArrayHandle temp(*this);
        ++(*this);
        return temp;
    }

    template<typename T>
    bool GpuArrayHandle<T>::operator==(const GpuArrayHandle<T>& other) const
    {
        return m_array == other.m_array && m_index == other.m_index;
    }

    template<typename T>
    bool GpuArrayHandle<T>::operator!=(const GpuArrayHandle<T>& other) const
    {
        return !(*this == other);
    }

    // --- GpuArray Implementation ---

    // Initializes the GPU array (creates a buffer and maps memory).
    template<typename T>
    ResultCode GpuArray<T>::Init(RHI::Device& device, const char* name, TL::Flags<RHI::BufferUsage> usageFlags, uint32_t capacity)
    {
        m_capacity  = capacity;
        m_count     = 0;
        m_allocated = 0; // Internal counter for the next new slot

        size_t                bufferSize = capacity * sizeof(T);
        RHI::BufferCreateInfo bufferCI{
            .name       = name,
            .hostMapped = true,
            .usageFlags = usageFlags,
            .byteSize   = bufferSize,

        };
        auto [buffer, result] = device.CreateBuffer(bufferCI);
        if (result != ResultCode::Success)
            return result;

        m_bufferOffset = 0;
        m_freeList.clear();

        return ResultCode::Success;
    }

    // Shuts down the GPU array and releases resources.
    template<typename T>
    void GpuArray<T>::Shutdown(RHI::Device& device)
    {
        // If necessary, unmap the memory. (Depends on your RHI API.)
        if (m_mappedPtr)
        {
            // For example: device.UnmapMemory(m_buffer);
            m_mappedPtr = nullptr;
        }

        device.DestroyBuffer(m_buffer);

        m_freeList.clear();
        m_count     = 0;
        m_allocated = 0;
    }

    // Inserts an element into the GPU array.
    // Returns a handle (iterator) to the inserted element.
    template<typename T>
    Result<GpuArrayHandle<T>> GpuArray<T>::Insert(const T& element)
    {
        uint32_t index = 0;
        if (!m_freeList.empty())
        {
            index = m_freeList.back();
            m_freeList.pop_back();
        }
        else
        {
            if (m_allocated >= m_capacity)
                return Result<GpuArrayHandle<T>>::Error("GpuArray is full");
            index = m_allocated;
            m_allocated++;
        }
        // Write the element into the mapped memory.
        T* data     = reinterpret_cast<T*>(static_cast<char*>(m_mappedPtr) + m_bufferOffset);
        data[index] = element;
        m_count++;
        return Result<GpuArrayHandle<T>>::Success(GpuArrayHandle<T>(this, index));
    }

    // Removes an element from the GPU array at the specified index.
    template<typename T>
    void GpuArray<T>::Remove(uint32_t index)
    {
        // (For simplicity, bounds-checking is omitted.)
        m_freeList.push_back(index);
        m_count--;
    }

    // Prepares the GPU array for updates. This might re-map memory or flush caches.
    template<typename T>
    void GpuArray<T>::BeginUpdate(RHI::Device& device)
    {
        m_mappedPtr = device.MapBuffer(m_buffer);
    }

    // Ends the update phase and flushes changes if required.
    template<typename T>
    void GpuArray<T>::EndUpdate(RHI::Device& device)
    {
        device.UnmapBuffer(m_buffer);
        m_mappedPtr = nullptr;
    }

    // Updates the element at the position specified by the handle.
    template<typename T>
    ResultCode GpuArray<T>::Update(GpuArrayHandle<T> iterator, const T& element)
    {
        uint32_t index = iterator.m_index;
        if (index >= m_allocated)
            return ResultCode::ErrorPoolOutOfMemory;

        T* data     = reinterpret_cast<T*>(static_cast<char*>(m_mappedPtr) + m_bufferOffset);
        data[index] = element;
        return ResultCode::Success;
    }

    // Returns the binding info of the underlying GPU buffer.
    template<typename T>
    RHI::BufferBindingInfo GpuArray<T>::GetBindingInfo() const
    {
        return RHI::BufferBindingInfo{
            .buffer = m_buffer,
            .offset = m_bufferOffset,
        };
    }

    // Returns the number of active elements in the GPU array.
    template<typename T>
    uint32_t GpuArray<T>::GetCount() const
    {
        return m_count;
    }

    // Returns the total capacity of the GPU array.
    template<typename T>
    uint32_t GpuArray<T>::GetCapacity() const
    {
        return m_capacity;
    }

} // namespace Engine