#pragma once

#include "RHI/Common.hpp"
#include "RHI/Object.hpp"
#include "RHI/SampleCount.hpp"

namespace RHI
{

/// @brief Enumeration for common allocation size constants
namespace AllocationSizeConstants
{

inline static constexpr uint32_t KB = 1024;

inline static constexpr uint32_t MB = 1024 * KB;

inline static constexpr uint32_t GB = 1024 * MB;

};  // namespace AllocationSizeConstants

class ResourcePool;

/// @brief Represents a pointer to GPU device memory
using DeviceMemoryPtr = void*;

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

/// @brief Enumeration representing the aspects of an image resource.
enum class ImageAspect
{
    None         = 0,
    Color        = 1 << 1,
    Depth        = 1 << 2,
    Stencil      = 1 << 3,
    DepthStencil = Depth | Stencil,
    All          = Color | DepthStencil,
};

/// @brief Enumeration representing the component mapping.
enum class ComponentSwizzle
{
    Identity = 0,
    Zero,
    One,
    R,
    G,
    B,
    A,
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

/// @brief Represent the offset into an image resource.
struct ImageOffset
{
    /// @brief Offset in the X direction.
    int32_t x;

    /// @brief Offset in the Y direction.
    int32_t y;

    /// @brief Offset in the Z direction.
    int32_t z;

    inline bool operator==(const ImageOffset& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }

    inline bool operator!=(const ImageOffset& other) const
    {
        return !(x == other.x && y == other.y && z == other.z);
    }
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

    inline bool operator==(const ImageSize& other) const
    {
        return width == other.width && height == other.height && depth == other.depth;
    }

    inline bool operator!=(const ImageSize& other) const
    {
        return !(width == other.width && height == other.height && depth == other.depth);
    }
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

    /// @brief The number of samples in each texel.
    SampleCount sampleCount;

    /// @brief The number of mip levels in the image.
    uint32_t mipLevels = 1;

    /// @brief The number of images in the images array.
    uint32_t arrayCount = 1;

    inline bool operator==(const ImageCreateInfo& other) const
    {
        return usageFlags == other.usageFlags && type == other.type && size == other.size && format == other.format && mipLevels == other.mipLevels && arrayCount == other.arrayCount;
    }

    inline bool operator!=(const ImageCreateInfo& other) const
    {
        return !(usageFlags == other.usageFlags && type == other.type && size == other.size && format == other.format && mipLevels == other.mipLevels && arrayCount == other.arrayCount);
    }
};

/// @brief Represent the creation parameters of an buffer resource.
struct BufferCreateInfo
{
    /// @brief Usage flags.
    Flags<BufferUsage> usageFlags;

    /// @brief The size of the buffer.
    size_t byteSize;

    inline bool operator==(const BufferCreateInfo& other) const
    {
        return usageFlags == other.usageFlags && byteSize == other.byteSize;
    }

    inline bool operator!=(const BufferCreateInfo& other) const
    {
        return !(usageFlags == other.usageFlags && byteSize == other.byteSize);
    }
};

/// @brief Represent a subview into a an image resource.
struct ImageSubresource
{
    ImageSubresource() = default;

    uint32_t           arrayBase    = 0;
    uint32_t           arrayCount   = 1;
    uint32_t           mipBase      = 0;
    uint32_t           mipCount     = 1;
    Flags<ImageAspect> imageAspects = ImageAspect::All;

    inline bool operator==(const ImageSubresource& other) const
    {
        return arrayBase == other.arrayBase && arrayCount == other.arrayCount && mipBase == other.mipBase && mipCount == other.mipCount;
    }

    inline bool operator!=(const ImageSubresource& other) const
    {
        return !(arrayBase == other.arrayBase && arrayCount == other.arrayCount && mipBase == other.mipBase && mipCount == other.mipCount);
    }
};

/// @brief Represent a subview into a an buffer resource.
struct BufferSubregion
{
    BufferSubregion() = default;

    size_t byteSize;

    size_t byteOffset;

    Format format;

    inline bool operator==(const BufferSubregion& other) const
    {
        return byteSize == other.byteSize && byteOffset == other.byteOffset && format == other.format;
    }

    inline bool operator!=(const BufferSubregion& other) const
    {
        return !(byteSize == other.byteSize && byteOffset == other.byteOffset && format == other.format);
    }
};

/// @brief Represent the texel color swizzling operation
struct ComponentMapping
{
    ComponentMapping() = default;

    ComponentSwizzle r = ComponentSwizzle::R;

    ComponentSwizzle g = ComponentSwizzle::G;

    ComponentSwizzle b = ComponentSwizzle::B;

    ComponentSwizzle a = ComponentSwizzle::A;

    inline bool operator==(const ComponentMapping& other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    inline bool operator!=(const ComponentMapping& other) const
    {
        return !(r == other.r && g == other.g && b == other.b && a == other.a);
    }
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
    MemoryType heapType;

    AllocationAlgorithm allocationAlgorithm;

    size_t capacity;

    size_t alignment;
};

// clang-format off
struct Image { };
struct Buffer { };

// clang-format on

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
};

}  // namespace RHI
