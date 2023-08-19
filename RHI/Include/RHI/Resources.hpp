#pragma once

#include <cstdint>
#include <memory>
#include <variant>

#include "RHI/Flags.hpp"
#include "RHI/Result.hpp"

namespace RHI
{

class ResourcePool;

/// @brief Represents a pointer to GPU device memory
using DeviceMemoryPtr = void*;

// Just tags, defintion is included in the backend.
struct Buffer;
struct Image;

/// @brief Enumeration for common allocation size constants
enum AllocationSizeConstant : uint32_t
{
    KB = 1024u,
    MB = 1024u * KB,
    GB = 1024u * MB,
};

/// @brief The type of memory
enum class MemoryType
{
    Host,
    Device,
    HostToDevice,
    DeviceToHost,
    Shared,
};

/// @brief  Type of the actual resource
enum class ResourceType
{
    Buffer,
    Image,
};

enum class BufferUsage
{
    None    = 0 << 0,
    Storage = 1 << 1,
    Uniform = 1 << 2,
    Vertex  = 1 << 3,
    Index   = 1 << 4,
    CopySrc = 1 << 5,
    CopyDst = 1 << 6,
};

enum class ImageUsage
{
    None        = 0 << 0,
    ShaderRead  = 1 << 1,
    ShaderWrite = 1 << 2,
    Color       = 1 << 3,
    Depth       = 1 << 4,
    Stencil     = 1 << 5,
    CopySrc     = 1 << 6,
    CopyDst     = 1 << 7,
};

enum class SampleCount
{
    None      = 1 << 0,
    Samples1  = 1 << 2,
    Samples2  = 1 << 3,
    Samples4  = 1 << 4,
    Samples8  = 1 << 5,
    Samples16 = 1 << 6,
    Samples32 = 1 << 7,
};

enum class ImageType
{
    None,
    Image1D,
    Image2D,
    Image3D,
    ImageCubeMap,
};

enum class AllocationAlgorithm
{
    Linear,
    Bump,
    Ring,
};

struct ImageOffset
{
    int32_t x;
    int32_t y;
    int32_t z;
};

struct ImageSize
{
    uint32_t width;
    uint32_t height;
    uint32_t depth;
};

struct ImageCreateInfo
{
    Flags<ImageUsage> usageFlags;
    ImageType         type;
    ImageSize         size;
    Format            format;
    uint32_t          mipLevels;
    uint32_t          arrayCount;
};

struct BufferCreateInfo
{
    Flags<BufferUsage> usageFlags;
    size_t             byteSize;
};

struct MemoryReport
{
    MemoryType type;
    size_t     size;
    size_t     alignment;
    size_t     allocationsCount;
};

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
    virtual MemoryReport GetMemoryReport() const = 0;

    /// @brief Report live objects in this resource pool.
    virtual void ReportLiveObjects() const = 0;

    /// @brief Allocate an image resource.
    virtual Result<Handle<Image>> Allocate(const ImageCreateInfo& createInfo) = 0;

    /// @brief Allocate a buffer resource.
    virtual Result<Handle<Buffer>> Allocate(const BufferCreateInfo& createInfo) = 0;

    /// @brief Allocates a shader bind group resource.
    virtual Result<Handle<ShaderBindGroup>> Allocate(const ShaderBindGroupCreateInfo& createInfo) = 0;

    /// @brief Free an allocated image resource.
    virtual void Free(Handle<Image> image) = 0;

    /// @brief Free an allocated buffer resource.
    virtual void Free(Handle<Buffer> buffer) = 0;

    /// @brief Get the size of an allocated image resource.
    virtual size_t GetSize(Handle<Image> image) const = 0;

    /// @brief Get the size of an allocated buffer resource.
    virtual size_t GetSize(Handle<Buffer> buffer) const = 0;

    /// @brief Maps the image resource for read or write operations.
    virtual DeviceMemoryPtr MapResource(Handle<Image> image, size_t offset, size_t range) = 0;

    /// @brief Maps the buffer resource for read or write operations.
    virtual DeviceMemoryPtr MapResource(Handle<Buffer> buffer, size_t offset, size_t range) = 0;

    /// @brief Unamps the image resource.
    virtual void Unamp(Handle<Image> image) = 0;

    /// @brief Unamps the buffer resource.
    virtual void Unamp(Handle<Buffer> buffer) = 0;

private:
    Context* m_context;
};

}  // namespace RHI