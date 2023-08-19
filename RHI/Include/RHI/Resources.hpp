#pragma once

#include <cstdint>
#include <memory>
#include <variant>

#include "RHI/Export.hpp"
#include "RHI/Flags.hpp"
#include "RHI/Format.hpp"
#include "RHI/Object.hpp"
#include "RHI/Result.hpp"

namespace RHI
{

class ResourcePool;

/// @brief Represents a pointer to GPU device memory
using DeviceMemoryPtr = void*;

// Just tags, defintion is included in the backend.

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

/// @brief  Type of the resource.
enum class ResourceType
{
    Buffer,
    Image,
};

/// @brief Enumeration representing how the buffer resource is intented to used.
enum class BufferUsage
{
    None = 0 << 0,

    /// @brief The buffer will be used as a storage buffer object.
    Storage = 1 << 1,

    /// @brief The buffer will be used as an uniform buffer object.
    Uniform = 1 << 2,

    /// @brief The buffer will be used as a vertex buffer object.
    Vertex = 1 << 3,

    /// @brief The buffer will be used as a index buffer object.
    Index = 1 << 4,

    /// @brief This buffer content will be copied from.
    CopySrc = 1 << 5,

    /// @brief This buffer content will be overwritten by a copy command.
    CopyDst = 1 << 6,
};

/// @brief Enumeration representing how the image resource is intented to used.
enum class ImageUsage
{
    None = 0 << 0,

    /// @brief The image will be used in an shader as bind resource.
    ShaderResource = 1 << 1,

    /// @brief The image will be the render target color attachment.
    Color = 1 << 3,

    /// @brief The image will be the render target depth attachment.
    Depth = 1 << 4,

    /// @brief The image will be the render target stencil attachment.
    Stencil = 1 << 5,

    /// @brief The image content will be copied.
    CopySrc = 1 << 6,

    /// @brief The image content will be overwritten by a copy command.
    CopyDst = 1 << 7,
};

/// @brief Enumeration how many samples a multisampled image contain.
enum class ImageSampleCount
{
    None      = 0 << 0,
    Samples1  = 1 << 1,
    Samples2  = 1 << 2,
    Samples4  = 1 << 3,
    Samples8  = 1 << 4,
    Samples16 = 1 << 5,
    Samples32 = 1 << 6,
};

/// @brief Enumeration representing the dimensions of an image resource.
enum class ImageType
{
    None,

    /// @brief Image is 1 dimentional.
    Image1D,

    /// @brief Image is 2 dimentional.
    Image2D,

    /// @brief Image is 3 dimentional.
    Image3D,

    /// @brief Image is a cube map.
    ImageCubeMap,
};

/// @brief Enumeration representing the memory allocation startegy of a resource pool.
enum class AllocationAlgorithm
{
    /// @brief Memory will be allocated linearly.
    Linear,
    Bump,
    Ring,
};

/// @brief Represent the offset into an image resource.
struct ImageOffset
{
    /// @brief Offset in the X direction.
    int32_t x;

    /// @brief Offset in the Y direction.
    int32_t y;

    /// @brief Offset in the Z direction.
    int32_t z;
};

/// @brief Represent the size of an image resource or subregion.
struct ImageSize
{
    /// @brief The width of the image.
    uint32_t width;

    /// @brief The height of the image.
    uint32_t height;

    /// @brief The depth of the image.
    uint32_t depth;
};

/// @brief Represent the creation parameters of an image resource.
struct ImageCreateInfo
{
    /// @brief Usage flags.
    Flags<ImageUsage> usageFlags;

    /// @brief The type of the image.
    ImageType type;

    /// @brief The size of the image.
    ImageSize size;

    /// @brief The format of the image.
    Format format;

    /// @brief The number of mip levels in the image.
    uint32_t mipLevels;

    /// @brief The number of images in the images array.
    uint32_t arrayCount;
};

/// @brief Represent the creation parameters of an buffer resource.
struct BufferCreateInfo
{
    /// @brief Usage flags.
    Flags<BufferUsage> usageFlags;

    /// @brief The size of the buffer.
    size_t byteSize;
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

/// @brief Empty base class for a image resource.
/// Backends extend this type to add implementation details.
struct Image;

/// @brief Empty base class for a buffer resource.
/// Backends extend this type to add implementation details.
struct Buffer;

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