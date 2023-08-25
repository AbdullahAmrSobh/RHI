#pragma once

#include "RHI/Object.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Result.hpp"

namespace RHI
{
class ResourcePool;

/// @brief Represents a pointer to GPU device memory
using DeviceMemoryPtr = void*;

/// @brief Enumeration for common allocation size constants
enum AllocationSizeConstants : uint32_t
{
    KB = 1024u,
    MB = 1024u * KB,
    GB = 1024u * MB,
};

/// @brief Enumeration representing different memory types and their locations.
enum class MemoryType
{
    /// @brief The memory is located in the system main memory.
    CPU,

    /// @brief The memory is locaed in the GPU, and can't be accessed by the CPU.
    GPULocal,

    /// @brief The memory is locaed in the GPU, but can be accessed by the CPU.
    GPUShared,
};

/// @brief Enumeration representing the memory allocation startegy of a resource pool.
enum class AllocationAlgorithm
{
    /// @brief Memory will be allocated linearly.
    Linear,
    Bump,
    Ring,
};

/// @brief Report describe the current state of the resource pool.
struct ResourcePoolReport
{
    MemoryType type;

    size_t size;

    size_t alignment;

    size_t allocationsCount;
};

/// @brief Represent the creation parameters of an resource pool.
struct ResourcePoolCreateInfo
{
    MemoryType          heapType;
    AllocationAlgorithm allocationAlgorithm;
    size_t              capacity;
    size_t              alignment;
};

/// @brief General purpose pool used to allocate all kinds of resources.
class ResourcePool : public Object
{
public:
    using Object::Object;
    virtual ~ResourcePool() = default;

    /// @brief Get a memory report for this resource pool.
    virtual ResourcePoolReport GetMemoryReport() const = 0;

    /// @brief Report live objects in this resource pool.
    virtual void ReportLiveObjects() const = 0;

    /// @brief Allocate an image resource.
    virtual Result<Handle<Image>> Allocate(const ImageCreateInfo& createInfo) = 0;

    /// @brief Allocate a buffer resource.
    virtual Result<Handle<Buffer>> Allocate(const BufferCreateInfo& createInfo) = 0;

    /// @brief Free an allocated image resource.
    virtual void Free(Handle<Image> image) = 0;

    /// @brief Free an allocated buffer resource.
    virtual void Free(Handle<Buffer> buffer) = 0;

    /// @brief Get the size of an allocated image resource.
    virtual size_t GetSize(Handle<Image> image) const = 0;

    /// @brief Get the size of an allocated buffer resource.
    virtual size_t GetSize(Handle<Buffer> buffer) const = 0;

    /// @brief Maps the image resource for read or write operations.
    /// @return returns a pointer to GPU memory, or a nullptr in case of failure
    virtual DeviceMemoryPtr MapResource(Handle<Image> image, size_t offset, size_t range) = 0;

    /// @brief Maps the buffer resource for read or write operations.
    /// @return returns a pointer to GPU memory, or a nullptr in case of failure
    virtual DeviceMemoryPtr MapResource(Handle<Buffer> buffer, size_t offset, size_t range) = 0;

    /// @brief Unamps the image resource.
    virtual void Unmap(Handle<Image> image) = 0;

    /// @brief Unmaps the buffer resource.
    virtual void Unmap(Handle<Buffer> buffer) = 0;

private:
    Context* m_context;
};

}  // namespace RHI
