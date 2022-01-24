#pragma once
#include "RHI/Buffer.hpp"
#include "RHI/Definitions.hpp"
#include "RHI/Sampler.hpp"
#include "RHI/Texture.hpp"

namespace RHI
{

enum class EShaderResourceType
{
    Sampler              = 0,
    SampledImage         = 2,
    StorageImage         = 3,
    UniformTexelBuffer   = 4,
    UniformStorageBuffer = 5,
    UniformBuffer        = 6,
    StorageBuffer        = 7,
    UniformBufferDynamic = 8,
    StorageBufferDynamic = 9,
};

enum class EShaderStageFlagBits
{

    VertexBit   = 0x00000001,
    DomainBit   = 0x00000002,
    HullBit     = 0x00000004,
    GeometryBit = 0x00000008,
    PixelBit    = 0x00000010,
    COMPUTE_BIT = 0x00000020,
    AllGraphics = 0x0000001F,
    All         = 0x7FFFFFFF,
};
using ShaderStageFlags = Flags<EShaderStageFlagBits>;

struct DescriptorSetLayoutBinding
{
    explicit DescriptorSetLayoutBinding(EShaderStageFlagBits stage, EShaderResourceType type);
    explicit DescriptorSetLayoutBinding(EShaderStageFlagBits stage, EShaderResourceType type, uint32_t arrayCount);

    EShaderResourceType  resourceType;
    uint32_t             resourceCount;
    EShaderStageFlagBits stageFlags;
    ISampler*            pImmutableSamplers;
};
using DescriptorSetLayout = ArrayView<DescriptorSetLayoutBinding>;

class IDescriptorSet
{
public:
    virtual ~IDescriptorSet() = default;

    virtual void BeginUpdate()                                                    = 0;
    virtual void EndUpdate()                                                      = 0;
    virtual void BindResource(uint32_t dstBinding, ITexture& texture)             = 0;
    virtual void BindResource(uint32_t dstBinding, ArrayView<ITexture*> textures) = 0;
    virtual void BindResource(uint32_t dstBinding, IBuffer& buffer)               = 0;
    virtual void BindResource(uint32_t dstBinding, ArrayView<IBuffer*> buffers)   = 0;
};
using DescriptorSetPtr = Unique<IDescriptorSet>;

struct DescriptorPoolDesc
{
    ArrayView<DescriptorSetLayout> descriptorSetLayouts;
    uint32_t                       maxSets;
};

class IDescriptorPool
{
public:
    virtual ~IDescriptorPool() = default;

    virtual Expected<DescriptorSetPtr> AllocateDescriptorSet(const DescriptorSetLayout& layout) = 0;
};
using DescriptorPoolPtr = Unique<IDescriptorPool>;

} // namespace RHI
