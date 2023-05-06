#pragma once

#include <initializer_list>
#include <memory>
#include <numeric>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "RHI/Export.hpp"
#include "RHI/Flag.hpp"
#include "RHI/LRUCache.hpp"
#include "RHI/Pipeline.hpp"

namespace RHI
{

class Context;
class ImageView;
class BufferView;
class Buffer;
class Sampler;

enum class ShaderBindingResourceType
{
    None,
    Buffer,
    ConstantBuffer,
    TexelBuffer,
    Image,
    Sampler,
};

enum class ShaderBindingResourceAccess
{
    None,
    Read,
    ReadWrite,
};

struct ShaderBinding
{
    std::string_view            name;
    ShaderBindingResourceType   type;
    ShaderBindingResourceAccess access;
    Flags<ShaderType>           stages;
    uint32_t                    arrayCount;
};

struct ShaderResourceGroupLayout
{
    ShaderResourceGroupLayout(std::initializer_list<ShaderBinding> bindings)
        : bindings(bindings)
        , hash(std::accumulate(bindings.begin(),
                               bindings.end(),
                               0,
                               [&](size_t h, const auto& binding) { return HashCombine(h, HashAny(binding)); }))
    {
    }

    const size_t                     hash;
    const std::vector<ShaderBinding> bindings;
};

class RHI_EXPORT ShaderResourceGroupAllocator
{
public:
    ShaderResourceGroupAllocator(Context& context)
        : m_context(&context)
    {
    }

    virtual ~ShaderResourceGroupAllocator() = default;

    virtual ResultCode Init() = 0;

    virtual std::unique_ptr<class ShaderResourceGroup> Allocate(const ShaderResourceGroupLayout& layout);

private:
    Context* m_context;
};

class RHI_EXPORT ShaderResourceGroup
{
protected:
    struct Binding
    {
        Binding() = default;

        Binding(uint32_t arrayOffset, uint32_t count, const Sampler** sampler)
            : arrayCount(count)
            , arrayOffset(arrayOffset)
            , type(ShaderBindingResourceType::Buffer)
            , samplers(sampler)
        {
        }

        Binding(uint32_t arrayOffset, uint32_t count, const ImageView** imageView)
            : arrayCount(count)
            , arrayOffset(arrayOffset)
            , type(ShaderBindingResourceType::Buffer)
            , images(imageView)
        {
        }

        Binding(uint32_t arrayOffset, uint32_t count, const BufferView** bufferView)
            : arrayCount(count)
            , arrayOffset(arrayOffset)
            , type(ShaderBindingResourceType::Buffer)
            , bufferViews(bufferView)
        {
        }

        Binding(uint32_t arrayOffset, uint32_t count, const Buffer** buffer)
            : arrayCount(count)
            , arrayOffset(arrayOffset)
            , type(ShaderBindingResourceType::Buffer)
            , buffers(buffer)
        {
        }

        Binding(uint32_t arrayOffset, const Sampler* sampler)
            : arrayCount(1)
            , arrayOffset(arrayOffset)
            , type(ShaderBindingResourceType::Buffer)
            , sampler(sampler)
        {
        }

        Binding(uint32_t arrayOffset, const ImageView* imageView)
            : arrayCount(1)
            , arrayOffset(arrayOffset)
            , type(ShaderBindingResourceType::Buffer)
            , image(imageView)
        {
        }

        Binding(uint32_t arrayOffset, const BufferView* bufferView)
            : arrayCount(1)
            , arrayOffset(arrayOffset)
            , type(ShaderBindingResourceType::Buffer)
            , bufferView(bufferView)
        {
        }

        Binding(uint32_t arrayOffset, const Buffer* buffer)
            : arrayCount(1)
            , arrayOffset(arrayOffset)
            , type(ShaderBindingResourceType::Buffer)
            , buffer(buffer)
        {
        }

        const uint32_t                  arrayCount;
        const uint32_t                  arrayOffset;
        const ShaderBindingResourceType type;

        union
        {
            const Sampler**    samplers;
            const Sampler*     sampler;

            const ImageView**  images;
            const ImageView*   image;

            const BufferView** bufferViews;
            const BufferView*  bufferView;

            const Buffer**     buffers;
            const Buffer*      buffer;
        };
    };

public:
    ShaderResourceGroup() = default;

    void SetImageView(uint32_t inputIndex, const ImageView* imageView, uint32_t arrayIndex = 0);

    void SetImageViewArray(uint32_t inputIndex, std::span<const ImageView* const> imageViews, uint32_t arrayIndex = 0);

    void SetBufferView(uint32_t inputIndex, const BufferView* bufferView, uint32_t arrayIndex = 0);

    void SetBufferViewArray(uint32_t inputIndex, std::span<const BufferView* const> bufferViews, uint32_t arrayIndex = 0);

    void SetSampler(uint32_t inputIndex, const Sampler& sampler, uint32_t arrayIndex = 0);

    void SetSamplerArray(uint32_t inputIndex, std::span<const Sampler> samplers, uint32_t arrayIndex = 0);

    void SetConstantRaw(uint32_t inputIndex, const void* bytes, uint32_t byteCount);

    template<typename T>
    void SetConstant(uint32_t inputIndex, const T& valueblac);

protected:
    RHI_SUPPRESS_C4251
    std::shared_ptr<ShaderResourceGroupLayout> m_layout;
    std::unordered_map<uint32_t, Binding>      m_bindingMaps;
};

}  // namespace RHI
