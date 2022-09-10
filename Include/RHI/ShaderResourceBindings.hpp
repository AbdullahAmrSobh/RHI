#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "RHI/Common.hpp"
#include "RHI/Resource.hpp"

#pragma once

namespace RHI
{

class ISampler;
class IImage;
class IImageView;
class IBuffer;
class IBufferView;

enum class EShaderResourceType
{
    Constant,
    Buffer,
    TexelBuffer,
    Image,
    Sampler,
    StaticSampler,
};

struct ShaderBindingIndex
{
    ShaderBindingIndex(uint32_t index);
    ShaderBindingIndex(std::string name);
    
    uint32_t    index;
    std::string name;
};

class ShaderResourceGroupLayout
{
public:
    class Builder;

    struct Binding
    {

        std::string         name;
        EShaderResourceType type;
        EAccess             access;
        ShaderStageFlags    stages;
        uint32_t            count;

        union
        {
            size_t     byteSize;
            EImageType imageType;
            ISampler*  pStaticSampler;
        };
    };

    size_t GetHash() const;

    Binding GetBinding(uint32_t index) const;
    Binding GetBinding(std::string_view name) const;

    const std::vector<Binding>& GetBindings() const;

    std::string GetBindingName(uint32_t index) const;
    uint32_t    GetBindingIndex(std::string_view name) const;

    void AddImageBinding(std::string name, EAccess access, ShaderStageFlags stages, uint32_t count);
    void AddSamplerBinding(std::string name, EAccess access, ShaderStageFlags stages, uint32_t count);
    void AddStaticSampler(std::string name, EAccess access, ShaderStageFlags stages, uint32_t count, ISampler& sampler);
    void AddBufferBinding(std::string name, EAccess access, ShaderStageFlags stages, uint32_t count, size_t byteSize);
    void AddTexelBufferBinding(std::string name, EAccess access, ShaderStageFlags stages, uint32_t count, EImageType imageType);

private:
    size_t CalculateHash(const Binding& binding) const;

private:
    size_t               m_hash;
    std::vector<Binding> m_bindings;
};

class IShaderResourceGroup
{
public:
    class Id;
    class Data;
    
    virtual ~IShaderResourceGroup() = default;

    virtual EResultCode BindData(const Data& data) = 0;
};

class IShaderResourceGroup::Data
{
private:
    struct ResourceBindingInfo
    {
    };

public:
    // Bind image resources
    void BindImage(ShaderBindingIndex bindingIndex, const IImageView& image);
    void BindImage(ShaderBindingIndex bindingIndex, uint32_t dstArrayElementIndex, const IImageView& imageView);
    void BindImages(ShaderBindingIndex bindingIndex, uint32_t dstArrayBeginIndex, const std::vector<const IImageView*>& imageViews);

    // Bind buffer resources
    void BindBuffer(ShaderBindingIndex bindingIndex, const IBuffer& buffer);
    void BindBuffer(ShaderBindingIndex bindingIndex, uint32_t dstArrayElementIndex, const IBuffer& buffer);
    void BindBuffers(ShaderBindingIndex bindingIndex, uint32_t dstArrayBeginIndex, const std::vector<const IBufferView*>& bufferViews);

    // Bind texel buffers resources
    void BindTexelBuffer(ShaderBindingIndex bindingIndex, const IBufferView& bufferView);
    void BindTexelBuffer(ShaderBindingIndex bindingIndex, uint32_t dstArrayElementIndex, const IBufferView& bufferView);
    void BindTexelBuffers(ShaderBindingIndex bindingIndex, uint32_t dstArrayBeginIndex, const std::vector<const IBufferView*>& bufferViews);

    // Bind samplers
    void BindSampler(ShaderBindingIndex bindingIndex, const ISampler& sampler);
    void BindSampler(ShaderBindingIndex bindingIndex, uint32_t dstArrayElementIndex, const ISampler& sampler);
    void BindSampler(ShaderBindingIndex bindingIndex, uint32_t dstArrayBeginIndex, const std::vector<const ISampler*> samplers);

    // SetConstant
    template <typename T>
    void SetConstant(ShaderBindingIndex bindingIndex, const T& data);

    template <typename T>
    void SetConstant(ShaderBindingIndex bindingIndex, uint32_t dstArrayElementIndex, const T& sampler);

    template <typename T>
    void SetConstantsArray(ShaderBindingIndex bindingIndex, uint32_t dstArrayBeginIndex, const std::vector<const T*> samplers);

    void SetConstantRaw(ShaderBindingIndex bindingIndex, void* pData, size_t byteSize);
    void SetConstantRaw(ShaderBindingIndex bindingIndex, void* pData, size_t byteOffset, size_t byteSize);
};

class IShaderResourceGroupAllocator
{
public:
    virtual ~IShaderResourceGroupAllocator() = default;

    virtual EResultCode Free(Unique<IShaderResourceGroup>& bindingGroup) = 0;

    virtual Unique<IShaderResourceGroup> Allocate(const ShaderResourceGroupLayout& layout) = 0;
};

} // namespace RHI
