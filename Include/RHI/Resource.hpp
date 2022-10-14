#pragma once
#include "RHI/Common.hpp"
#include "RHI/Format.hpp"
#include "RHI/Memory.hpp"

namespace RHI
{

enum class EShaderStageFlagBits
{
    Vertex = 0x000001,
    Pixel  = 0x000002
};
using ShaderStageFlags = Flags<EShaderStageFlagBits>;

struct ShaderProgramDesc
{
    EShaderStageFlagBits  stage;
    std::vector<uint32_t> shaderCode;
    std::string           entryName;
};

class IShaderProgram
{
public:
    IShaderProgram(std::string name)
        : m_name(std::move(name))
    {
    }
    virtual ~IShaderProgram();

    inline std::string GetFunctionName() const
    {
        return m_name;
    }

private:
    std::string m_name;
};

class IFence
{
public:
    virtual ~IFence() = default;

    virtual EResultCode Wait() const      = 0;
    virtual EResultCode Reset() const     = 0;
    virtual EResultCode GetStatus() const = 0;
};

enum class ESamplerFilter
{
    Linear,
    Nearest
};

enum class ESamplerAddressMode
{
    Repeat,
    Clamp
};

enum class ESamplerCompareOp
{
    Never,
    Less,
    LessEq,
    Greater,
    GreaterEq,
};

struct SamplerDesc
{

    ESamplerFilter      filter;
    ESamplerCompareOp   compare;
    float               mipLodBias;
    ESamplerAddressMode addressU;
    ESamplerAddressMode addressV;
    ESamplerAddressMode addressW;
    float               minLod;
    float               maxLod;
    float               maxAnisotropy;
};

class ISampler
{
public:
    virtual ~ISampler() = default;
};

enum class EResourceType
{
    Image,
    Buffer
};

class IResource
{
public:
    virtual ~IResource() = default;

    size_t GetSize() const;

    Expected<MappedAllocationPtr> Map(size_t byteOffset, size_t byteSize);
    void                          Unmap();

protected:
    size_t m_memorySize = SIZE_MAX;
};

enum class ESampleCount
{
    Undefined = 0x0000000,
    Count1    = 0x0000001,
    Count2    = 0x0000002,
    Count4    = 0x0000004,
    Count8    = 0x0000008,
    Count16   = 0x000000F,
    Count32   = 0x0000020,
    Count64   = 0x0000040
};

enum class EImageUsageFlagBits
{
    Undefined    = 0x000000,
    Color        = 0x000001,
    DepthStencil = 0x000002,
    Transfer     = 0x000004,
    ShaderInput  = 0x000008,
};
using ImageUsageFlags = Flags<EImageUsageFlagBits>;

enum class EImageType
{
    Type1D,
    Type2D,
    Type3D,
    TypeCubeMap
};

struct ImageDesc
{
    ImageUsageFlags usage;
    Extent3D        extent;
    EFormat         format;
    ESampleCount    sampleCount;
    uint32_t        mipLevelsCount;
    uint32_t        arraySize;
};

class IImage : public IResource
{
public:
    virtual ~IImage() = default;
};

struct ImageViewRange
{
    uint32_t baseArrayElement;
    uint32_t arraySize;
    uint32_t baseMipLevel;
    uint32_t mipLevelsCount;
};

enum class EImageViewAspectFlagBits
{
    Undefined = 0x00000000,
    Color     = 0x00000001,
    Depth     = 0x00000002,
    Stencil   = 0x00000004,
};
using ImageViewAspectFlags = Flags<EImageViewAspectFlagBits>;

struct ImageViewDesc
{
    EFormat              format;
    EImageType           type;
    ImageViewAspectFlags viewAspect;
    ImageViewRange       range;
};

class IImageView
{
public:
    virtual ~IImageView() = default;
};

enum class EBufferUsageFlagBits
{
    Instance = 0x000001,
    Vertex   = 0x000002,
    Index    = 0x000004,
    Transfer = 0x000008,
};
using BufferUsageFlags = Flags<EBufferUsageFlagBits>;

struct BufferDesc
{
    BufferUsageFlags usage;
    size_t           size;
};

class IBuffer : public IResource
{
public:
    virtual ~IBuffer() = default;
};

struct BufferRange
{
    size_t byteOffset;
    size_t byteRange;
};

struct BufferViewDesc
{
    EFormat     format;
    BufferRange range;
};

class IBufferView
{
public:
    virtual ~IBufferView() = default;
};

} // namespace RHI