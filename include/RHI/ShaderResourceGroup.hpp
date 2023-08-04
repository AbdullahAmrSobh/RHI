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

enum class ShaderResourceType
{
    None,
    ConstantBuffer,
    UniformBuffer,
    StorageBuffer,
    UniformTexelBuffer,
    StorageTexelBuffer,
    Image,
    Sampler,
};

enum class ShaderResourceAccess
{
    None,
    Read,
    ReadWrite,
};

struct ShaderResource
{
    std::string_view     name;
    ShaderResourceType   type;
    ShaderResourceAccess access;
    Flags<ShaderStage>    stages;
    uint32_t             arrayCount;
};

struct ShaderResourceGroupLayout
{
    ShaderResourceGroupLayout(std::initializer_list<ShaderResource> bindings)
        : bindings(bindings)
        , hash(0)
    {
        for (auto binding : bindings)
            hash = HashCombine(hash, HashAny(binding));
    }

    const std::vector<ShaderResource> bindings;
    size_t                      hash;
};

class RHI_EXPORT ShaderResourceGroup
{
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
    void SetConstant(uint32_t inputIndex, const T& value);

protected:
    struct Binding
    {
        Binding() = default;

        uint32_t arrayCount;

        uint32_t arrayOffset;

        ShaderResourceType type;

        union
        {
            const Sampler*    sampler;
            const ImageView*  image;
            const BufferView* bufferView;
            const Buffer*     buffer;
        };
    };

    RHI_SUPPRESS_C4251
    std::shared_ptr<ShaderResourceGroupLayout> m_layout;
    std::unordered_map<uint32_t, Binding>      m_bindingMaps;
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

    virtual std::unique_ptr<ShaderResourceGroup> Allocate(const ShaderResourceGroupLayout& layout) = 0;

private:
    Context* m_context;
};

}  // namespace RHI
