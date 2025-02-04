#pragma once

#include <TL/Utils.hpp>

#include "Common.hpp"

namespace Engine
{

    /// @brief A buffer suballocator that manages sub-allocations from a single large buffer
    /// @details Allows efficient allocation and deallocation of smaller buffer regions from a pre-allocated buffer
    class BufferSuballocator
    {
    public:
        BufferSuballocator();

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
} // namespace Engine