#pragma once
#include "RHI/Resource.hpp"

namespace RHI
{

class ISampler;

enum class ShaderInputResourceType
{
    Uninitalized,
    ConstantBuffer,
    Buffer,
    Image,
    TexelBuffer,
    Sampler
};

enum class ShaderStagesFlagBits
{
    Uninitalized,
    Vertex,
    Pixel,
    Geometry,
    TessCtrl,
    TessEval,
    Compute,
};
using ShaderStagesFlags = Flags<ShaderStagesFlagBits>;

using ShaderBindingIndex = uint32_t;

struct ShaderBindingReference
{
    ShaderBindingReference(ShaderBindingIndex index, std::string_view name)
        : name(name)
        , index(index)
    {
    }

    std::string_view   name;
    ShaderBindingIndex index = UINT32_MAX;
};

struct ShaderInputResourceBindingDescBase
{
    std::string             name;
    ShaderInputResourceType type;
    ShaderStageFlags        stages;
    uint32_t                count;
};

struct ShaderInputResourceBindingDesc : ShaderInputResourceBindingDescBase
{
    AccessType access;
};

struct ShaderStaticSamplerResourceBindingDesc : ShaderInputResourceBindingDescBase
{
    std::vector<ISampler*> staticSamplersPtrs;
};

struct ShaderConstantBufferBindingDesc : ShaderInputResourceBindingDescBase
{
    size_t byteSize;
    size_t alignment;
};

class ShaderResourceGroupLayout
{
public:
    size_t GetHash() const
    {
        return m_hash;
    }

    const std::vector<ShaderBindingReference>& GetBindingReferencs() const
    {
        return m_bindingReferences;
    }

    const std::vector<ShaderInputResourceBindingDesc>& GetShaderInputResourceBindings() const
    {
        return m_resourceBindings;
    }

    const std::vector<ShaderConstantBufferBindingDesc>& GetShaderConstantBufferBindings() const
    {
        return m_constantBuffersBindings;
    }

    const std::vector<ShaderStaticSamplerResourceBindingDesc>& GetShaderStaticSamplerResourceBindings() const
    {
        return m_staticSamplers;
    }

    ShaderBindingReference AddInputResource(const ShaderInputResourceBindingDesc& bindingDesc)
    {
        m_bindingReferences.emplace_back(++m_resourceBindingCount, bindingDesc.name);
        m_resourceBindings.push_back(bindingDesc);
        return m_bindingReferences.back();
    }

    ShaderBindingReference AddStaticSampler(const ShaderStaticSamplerResourceBindingDesc& bindingDesc)
    {
        m_bindingReferences.emplace_back(++m_resourceBindingCount, bindingDesc.name);
        m_staticSamplers.push_back(bindingDesc);
        return m_bindingReferences.back();
    }

    ShaderBindingReference AddConstantBuffer(const ShaderConstantBufferBindingDesc& bindingDesc)
    {
        m_bindingReferences.emplace_back(++m_constantBufferBindingCount, bindingDesc.name);
        m_constantBuffersBindings.push_back(bindingDesc);
        return m_bindingReferences.back();
    }

private:
    template<typename T>
    static constexpr size_t hasher(T x)
    {
        return std::hash<T> {}(x);
    }

    void UpdateHashValue(const ShaderInputResourceBindingDescBase& desc)
    {
        m_hash =
            hash_combine(m_hash, hash_combine(ShaderResourceGroupLayout::hasher(desc.count), ShaderResourceGroupLayout::hasher(desc.name)));
    }

private:
    size_t m_hash = 0;

    uint32_t                                            m_resourceBindingCount       = 0;
    uint32_t                                            m_constantBufferBindingCount = 0;
    std::vector<ShaderBindingReference>                 m_bindingReferences;
    std::vector<ShaderInputResourceBindingDesc>         m_resourceBindings;
    std::vector<ShaderConstantBufferBindingDesc>        m_constantBuffersBindings;
    std::vector<ShaderStaticSamplerResourceBindingDesc> m_staticSamplers;
};

class ShaderResourceGroupData
{
public:
    struct ConstantBuffer
    {
        size_t byteSize;
        void*  pConstantData;
    };

    const ShaderResourceGroupLayout& GetLayout() const
    {
        return *m_pLayout;
    }

    const std::map<ShaderBindingIndex, std::vector<IImageView*>>& GetImageBinds() const
    {
        return m_imageBinds;
    }

    const std::map<ShaderBindingIndex, std::vector<IBuffer*>>& GetBuffersBinds() const
    {
        return m_buffersBinds;
    }

    const std::map<ShaderBindingIndex, std::vector<IBufferView*>>& GetTexelBufferBinds() const
    {
        return m_texelBufferBinds;
    }

    const std::map<ShaderBindingIndex, std::vector<ISampler*>>& GetSamplersBinds() const
    {
        return m_samplersBinds;
    }

    const std::map<ShaderBindingIndex, ConstantBuffer>& GetConstants() const
    {
        return m_constants;
    }

    void BindImages(const ShaderBindingReference& bindingReference, const std::vector<IImageView*>& images)
    {
        m_imageBinds[bindingReference.index] = images;
    }

    // TODO Take BufferRangeView
    void BindBuffers(const ShaderBindingReference& bindingReference, const std::vector<IBuffer*>& buffers)
    {
        m_buffersBinds[bindingReference.index] = buffers;
    }

    void BindTexelBuffers(const ShaderBindingReference& bindingReference, const std::vector<IBufferView*>& bufferViews)
    {
        m_texelBufferBinds[bindingReference.index] = bufferViews;
    }

    void BindSamplers(const ShaderBindingReference& bindingReference, const std::vector<ISampler*>& samplers)
    {
        m_samplersBinds[bindingReference.index] = samplers;
    }

    template<typename T>
    void SetConstant(const ShaderBindingReference& bindingReference, const std::vector<T>& data)
    {
        m_constants[bindingReference.index] = ConstantBuffer(data.size(), data.data());
    }

private:
    ShaderResourceGroupLayout* m_pLayout;

    std::map<ShaderBindingIndex, std::vector<IImageView*>> m_imageBinds;

    std::map<ShaderBindingIndex, std::vector<IBuffer*>> m_buffersBinds;

    std::map<ShaderBindingIndex, std::vector<IBufferView*>> m_texelBufferBinds;

    std::map<ShaderBindingIndex, std::vector<ISampler*>> m_samplersBinds;

    std::map<ShaderBindingIndex, ConstantBuffer> m_constants;
};

class IShaderResourceGroup
{
public:
    virtual ~IShaderResourceGroup() = default;

    Expected<std::string_view> GetBindingName(uint32_t index) const
    {
        for (auto& reference : m_bindingReferencs)
        {
            if (reference.index == index)
            {
                return reference.name;
            }
        }
        return Unexpected(ResultCode::InvalidArguments);
    }

    Expected<uint32_t> GetBindingIndex(std::string_view name) const
    {
        for (auto& reference : m_bindingReferencs)
        {
            if (reference.name == name)
            {
                return reference.index;
            }
        }
        return Unexpected(ResultCode::InvalidArguments);
    }

    virtual ResultCode Update(const ShaderResourceGroupData& data) = 0;

protected:
    std::vector<ShaderBindingReference> m_bindingReferencs;
    Unique<ShaderResourceGroupLayout>   m_layout;
};

class IShaderResourceGroupAllocator
{
public:
    virtual ~IShaderResourceGroupAllocator() = default;

    virtual Expected<Unique<IShaderResourceGroup>> Allocate(const ShaderResourceGroupLayout& layout) = 0;
};

}  // namespace RHI
