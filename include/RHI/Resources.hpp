#pragma once

#include <cstdint>
#include <memory>

#include "RHI/Export.hpp"
#include "RHI/Flag.hpp"
#include "RHI/Format.hpp"
#include "RHI/LRUCache.hpp"
#include "RHI/Result.hpp"

namespace RHI
{

class Context;

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

enum class ImageAspect
{
    None    = 0 << 0,
    Color   = 1 << 1,
    Depth   = 1 << 2,
    Stencil = 1 << 3,
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

enum class ResourceMemoryType
{
    None,
    Stage,
    DeviceLocal,
};

enum class ImageType
{
    None,
    Image1D,
    Image2D,
    Image3D,
    ImageCubeMap,
    Count,
};

enum class ComponentSwizzle
{
    None,
    Zero,
    One,
    R,
    G,
    B,
    A,
};

enum class FenceState
{
    None = 0,
    Signaled,
    Unsignaled
};

enum class SamplerFilter
{
    Linear,
    Nearest
};

enum class SamplerAddressMode
{
    Repeat,
    Clamp
};

enum class SamplerCompareOp
{
    None,
    Never,
    Equal,
    NotEqual,
    Always,
    Less,
    LessEq,
    Greater,
    GreaterEq,
};

struct ImageSize
{
    uint32_t width;
    uint32_t height;
    uint32_t depth;
};

struct ComponentMapping
{
    ComponentSwizzle r;
    ComponentSwizzle g;
    ComponentSwizzle b;
    ComponentSwizzle a;
};

struct ImageSubresource
{
    Flags<ImageAspect> aspectMask;
    uint32_t           mipLevel;
    uint32_t           baseArrayLayer;
    uint32_t           layerCount;
};

struct ResourceAllocationInfo
{
    ResourceMemoryType usage;
    size_t             alignment;
};

struct ImageCreateInfo
{
    Flags<ImageUsage> usageFlags;
    ImageType         type;
    ImageSize         size;
    SampleCount       sampleCount;
    Format            format;
    uint32_t          mipLevels;
    uint32_t          arrayCount;
    void*             content;
};

struct BufferCreateInfo
{
    Format             format;
    size_t             byteSize;
    Flags<BufferUsage> usageFlags;
    void*              content;
};

struct ImageViewCreateInfo
{
    ComponentMapping components;
    ImageSubresource subresource;
};

struct BufferViewCreateInfo
{
    Format format;
    size_t byteOffset;
    size_t byteSize;
};

struct SwapchainCreateInfo
{
    ImageSize         imageSize;
    Flags<ImageUsage> imageUsage;
    Format            imageFormat;
    uint32_t          imageCount;
    void*             nativeWindowHandle;
};

struct SamplerCreateInfo
{
    SamplerFilter      filter;
    SamplerCompareOp   compare;
    float              mipLodBias;
    SamplerAddressMode addressU;
    SamplerAddressMode addressV;
    SamplerAddressMode addressW;
    float              minLod;
    float              maxLod;
    float              maxAnisotropy;
};

class RHI_EXPORT Buffer
{
public:
    Buffer(Context& context)
        : m_context(&context)
    {
    }

    virtual ~Buffer() = default;

    virtual ResultCode Init(const RHI::ResourceAllocationInfo& allocationInfo, const BufferCreateInfo& createInfo) = 0;

    std::shared_ptr<class BufferView> CreateView(const BufferViewCreateInfo& createInfo);

protected:
    Context* m_context;
};

class RHI_EXPORT Image
{
public:
    Image(Context& context)
        : m_context(&context)
    {
    }

    virtual ~Image() = default;

    virtual ResultCode Init(const RHI::ResourceAllocationInfo& allocationInfo, const ImageCreateInfo& createInfo) = 0;

    std::shared_ptr<class ImageView> CreateView(const ImageViewCreateInfo& createInfo);

protected:
    Context* m_context;
};

class RHI_EXPORT BufferView
{
public:
    BufferView(Context& context)
        : m_context(&context)
    {
    }

    virtual ~BufferView() = default;

    virtual ResultCode Init(Buffer& buffer, const BufferViewCreateInfo& createInfo) = 0;

protected:
    Context* m_context;
};

class RHI_EXPORT ImageView
{
public:
    ImageView(Context& context)
        : m_context(&context)
    {
    }

    virtual ~ImageView() = default;

    virtual ResultCode Init(Image& image, const ImageViewCreateInfo& createInfo) = 0;

protected:
    Context* m_context;
};

class RHI_EXPORT Swapchain
{
public:
    Swapchain(Context& context)
        : m_context(&context)
    {
    }

    virtual ~Swapchain() = default;

    virtual ResultCode Init(const SwapchainCreateInfo& createInfo) = 0;

    const Image& GetCurrentImage() const
    {
        return *m_images[m_currentImageIndex];
    }

    Image& GetCurrentImage()
    {
        return *m_images[m_currentImageIndex];
    }

    virtual ResultCode Resize(uint32_t newWidth, uint32_t newHeight) = 0;

    virtual ResultCode SwapImages() = 0;

protected:
    constexpr static uint32_t           MaxBackImagesCount = 3u;
    Context*                            m_context;
    std::vector<std::unique_ptr<Image>> m_images;
    uint32_t                            m_currentImageIndex;
};

class RHI_EXPORT Sampler
{
public:
    Sampler(Context& context)
        : m_context(&context)
    {
    }

    virtual ~Sampler() = default;

    virtual ResultCode Init(const SamplerCreateInfo& createInfo) = 0;

protected:
    Context* m_context;
};

class RHI_EXPORT Fence
{
public:
    Fence(Context& context)
        : m_context(&context)
    {
    }

    virtual ~Fence() = default;

    virtual ResultCode Init() = 0;

    virtual ResultCode Reset()          = 0;
    virtual ResultCode Wait() const     = 0;
    virtual FenceState GetState() const = 0;

protected:
    Context* m_context;
};

}  // namespace RHI