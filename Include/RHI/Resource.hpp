#pragma once
#include <cstdint>
#include "RHI/Common.hpp"
#include "RHI/Format.hpp"

namespace RHI
{

enum class EShaderStageFlagBits
{
    Vertex = 0x01,
    Pixel  = 0x02
};
using ShaderStageFlags = Flags<EShaderStageFlagBits>;

struct ShaderProgramDesc
{
    EShaderStageFlagBits   stage;
    std::vector<std::byte> binShader;
};

class IShaderProgram
{
public:
    virtual ~IShaderProgram();
};

class IFence
{
public:
    virtual ~IFence() = default;

    virtual EResultCode Wait() const      = 0;
    virtual EResultCode Reset() const     = 0;
    virtual EResultCode GetStatus() const = 0;
};

enum class EMemoryPhysicalType 
{
    Host, 
    Device,
}; 

struct ResourceMemoryRequirement 
{
    size_t byteSize;
    size_t byteAlignment;
};

struct MemoryAllocationDesc 
{
    EMemoryPhysicalType       type;
    size_t                    byteOffset; 
    ResourceMemoryRequirement memoryRequirement; 
};

struct SamplerDesc
{
    enum class EFilter
    {
        Point,
        Bilinear,
        Trilinear,
        Anisotropic
    };

    enum class EAddressMode
    {
        Repeat,
        Clamp
    };

    enum class ECompareOp
    {
        Never,
        Less,
        LessEq,
        Greater,
        GreaterEq,
    };

    EFilter      filter;
    ECompareOp   compare;
    float        mipLodBias;
    EAddressMode addressU;
    EAddressMode addressV;
    EAddressMode addressW;
    float        minLod;
    float        maxLod;
};

class ISampler
{
public:
    virtual ~ISampler() = default;
};

enum class ESampleCount
{
    Undefined = 0,
    Count1    = 1,
    Count2    = 2,
    Count4    = 4,
    Count8    = 8,
    Count16   = 16,
    Count32   = 32,
    Count64   = 64
};

enum class EImageViewAspectFlagBits
{
    Undefined = 0x00000000,
    Color     = 0x00000001,
    Depth     = 0x00000002,
    Stencil   = 0x00000004,
};
using ImageViewAspectFlags = Flags<EImageViewAspectFlagBits>;

enum class EImageType
{
    Type1D,
    Type2D,
    Type3D,
    TypeCubeMap
};

struct ImageViewRange
{
    uint32_t baseArrayElement;
    uint32_t arraySize;
    uint32_t baseMipLevel;
    uint32_t mipLevelsCount;
};

struct BufferViewRange
{
    size_t byteOffset;
    size_t byteRange;
};

struct ImageDesc
{
    Extent3D     extent;
    EFormat      format;
    ESampleCount sampleCount;
    uint32_t     mipLevelsCount;
    uint32_t     arraySize;
};

enum class EResourceType
{
    Image,
    Buffer
};

class IImage
{
public:
    virtual ~IImage() = default;
};

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

struct BufferDesc
{
    size_t size;
};

class IBuffer
{
public:
    virtual ~IBuffer() = default;
};

struct BufferViewDesc
{
    EFormat format;
    size_t  byteOffset;
    size_t  byteRange;
};

class IBufferView
{
public:
    virtual ~IBufferView() = default;
};

} // namespace RHI