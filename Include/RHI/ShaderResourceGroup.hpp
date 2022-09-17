#pragma once
#include <cstddef>
#include <initializer_list>
#include <map>
#include <string>
#include <string_view>
#include <varient>
#include <vector>

#include "RHI/Common.hpp"
#include "RHI/Resource.hpp"

namespace RHI
{

enum class EShaderInputResourceType
{
    ConstantBuffer,
    Buffer,
    Image,
    TexelBuffer,
    Sampler
};

enum class EShaderStagesFlagBits
{
    Vertex,
    Pixel,
    Geometry,
    TessCtrl,
    TessEval,
    Compute,
};
using ShaderStagesFlags = Flags<EShaderStagesFlagBits>;

struct ShaderBindingReference
{
    ShaderBindingReference(std::string_view name);
    ShaderBindingReference(uint32_t index);

    std::string_view name;
    uint32_t         bindingIndex = UINT32_MAX;
};

struct ShaderInputResourceBindingDescBase
{
    std::string              name;
    EShaderInputResourceType type;
    ShaderStageFlags         stages;
    uint32_t                 count;
};

struct ShaderInputResourceBindingDesc : ShaderInputResourceBindingDescBase 
{
    EAccess access;
}

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
    size_t GetHash() const;

    inline const std::vector<ShaderBindingReference>& GetBindingReferencs() const
    {
        return m_bindingReferences;
    }

    // The order of binding decleration is important

    inline void AddInputResource(const ShaderInputResourceBindingDesc& bindingDesc)
    {
        m_bindingReferences.emplace_back(++m_resourceBindingCount, bindingDesc.name);
        m_resourceBindings.push_back(bindingDesc);
    }

    inline void AddStaticSampler(const ShaderStaticSamplerResourceBindingDesc& bindingDesc)
    {
        m_bindingReferences.emplace_back(++m_resourceBindingCount, bindingDesc.name);
        m_resourceBindings.push_back(bindingDesc);
    }

    inline void AddConstantBuffer(const ShaderConstantBufferBindingDesc& bindingDesc)
    {
        m_bindingReferences.emplace_back(++m_resourceBindingCount, bindingDesc.name);
        m_resourceBindings.push_back(bindingDesc);
    }

    inline const std::vector<ShaderInputResourceBindingDesc>& GetShaderInputResourceBindings() const
    {
        return m_resourceBindings;
    }

    inline const std::vector<ShaderConstantBufferBindingDesc>& GetShaderConstantBufferBindings() const
    {
        return m_constantBuffersBindings;
    }

    inline const std::vector<ShaderStaticSamplerResourceBindingDesc>& GetShaderStaticSamplerResourceBindings() const
    {
        return m_staticSamplers;
    }

private:
    uint32_t m_resourceBindingCount       = 0;
    uint32_t m_constantBufferBindingCount = 0;
    
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

private:
    std::map<std::string, std::vector<IImageView*>> m_imageBinds;
    
    std::map<std::string, std::vector<IBuffer*>> m_buffersBinds;
    
    std::map<std::string, std::vector<IBufferView*>> m_texelBufferBinds;
    
    std::map<std::string, std::vector<ISampler*>> m_samplersBinds;
    
    std::map<std::string, ConstantBuffer> m_constants;

public:
    inline void BindImages(ShaderBindingReference bindingReference, const std::vector<IImageView*>& images)
    {
        m_imageBinds[bindingReference.name] = images.data();
    }

    inline void BindBuffers(ShaderBindingReference bindingReference, const std::vector<IBuffer*>& buffers)
    {
        m_imageBinds[bindingReference.name] = buffers.data();
    }

    inline void BindTexelBuffers(ShaderBindingReference bindingReference, const std::vector<IBufferView*>& bufferViews)
    {
        m_imageBinds[bindingReference.name] = bufferViews.data();
    }

    inline void BindSamplers(ShaderBindingReference bindingReference, const std::vector<ISampelr*>& samplers)
    {
        m_imageBinds[bindingReference.name] = samplers.data();
    }

    template <typename T>
    inline void SetConstant(ShadewrBindingReference bindingReference, const std::vector<T>& data)
    {
        m_imageBinds[bindingReference.name] = ConstantBuffer(data.size(), data.data());
    }
};

class IShaderResourceGroup
{
public:
    ~IShaderResourceGroup() = default;

    Expected<std::string_view> GetBindingName(uint32_t index) const
    {
        for (auto& reference : m_bindingReferencs)
        {
            if (reference.name == name)
            {
                return reference.name;
            }
        }
        return Unexpected(EResultCode::InvalidArguments);
    }

    Expected<uint32_t> GetBindingIndex(std::string_view name) const
    {
        for (auto& reference : m_bindingReferencs)
        {
            if (reference.index == index)
            {
                return reference.name;
            }
        }
        return Unexpected(EResultCode::InvalidArguments);
    }

    virtual EResultCode Update(const ShaderResourceGroupData& data) = 0;

protected:
    std::vector<ShaderBindingReference> m_bindingReferencs;
    Unique<ShaderResourceGroupLayout>   m_layout;
};

class IShaderResourceGroupAllocator
{
public:
    ~IShaderResourceGroupAllocator() = default;

    virtual Expected<Unique<IShaderResourceGroup>> Allocate() const = 0;

    virtual void Free(Unique<IShaderResourceGroup> group) = 0;
};

} // namespace RHI
