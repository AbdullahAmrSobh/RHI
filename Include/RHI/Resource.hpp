#pragma once

#include "RHI/Common.hpp"

#include "RHI/Format.hpp"
#include "RHI/Memory.hpp"

namespace RHI
{

enum class ShaderStageFlagBits
{
    Vertex = 0x000001,
    Pixel  = 0x000002
};
using ShaderStageFlags = Flags<ShaderStageFlagBits>;

struct ShaderProgramDesc
{
    ShaderStageFlagBits   stage;
    std::vector<uint32_t> shaderCode;
    std::string           entryName;
};

class IShaderProgram
{
public:
    IShaderProgram(std::string name)
        : m_functionName(std::move(name))
    {
    }
    virtual ~IShaderProgram() = default;

    const std::string m_functionName;
};

class IFence
{
public:
    virtual ~IFence() = default;

    virtual ResultCode Reset()           = 0;
    virtual ResultCode Wait() const      = 0;
    virtual ResultCode GetStatus() const = 0;
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
    Never,
    Equal,
    NotEqual,
    Always,
    Less,
    LessEq,
    Greater,
    GreaterEq,
};

struct SamplerDesc
{
    SamplerDesc() = default;

    size_t GetHash() const
    {
        return 0;
    }

    SamplerFilter      filter  = SamplerFilter::Linear;
    SamplerCompareOp   compare = SamplerCompareOp::Always;
    float              mipLodBias;
    SamplerAddressMode addressU = SamplerAddressMode::Clamp;
    SamplerAddressMode addressV = SamplerAddressMode::Clamp;
    SamplerAddressMode addressW = SamplerAddressMode::Clamp;
    float              minLod;
    float              maxLod;
    float              maxAnisotropy;
};

class ISampler
{
public:
    virtual ~ISampler() = default;
};

using MappedAllocationPtr = void*;

class IResource
{
public:
    virtual ~IResource() = default;

    size_t GetSize() const
    {
        return m_memorySize;
    }

    template<typename T>
    ResultCode SetData(size_t byteOffset, std::span<T> bufferData)
    {
        return SetDataInternal(byteOffset, reinterpret_cast<const uint8_t*>(bufferData.data()), bufferData.size() * sizeof(T));
    }

protected:
    virtual ResultCode SetDataInternal(size_t byteOffset, const uint8_t* bufferData, size_t bufferDataByteSize) = 0;

protected:
    size_t m_memorySize = SIZE_MAX;
};

}  // namespace RHI