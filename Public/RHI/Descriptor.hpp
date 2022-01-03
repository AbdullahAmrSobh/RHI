#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/Sampler.hpp"
#include "RHI/Texture.hpp"
#include "RHI/Buffer.hpp"

namespace RHI
{

enum class EResourceViewType
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
    EResourceViewType     resourceType;
    uint32_t              resourceCount;
    ShaderStageFlags      stageFlags;
	ISampler*             pImmutableSamplers;
};

struct DescriptorSetLayout 
{
	uint32_t                    bindingCount;
    DescriptorSetLayoutBinding* pBindings;
};

struct DescriptorSetUpdateInfo
{
	void SetBinding(uint32_t bindingIndex, uint32_t arrayElement, ITexture* pTexture);
	void SetBinding(uint32_t bindingIndex, uint32_t arrayElement, IBuffer* pBuffer);
};

class IDescriptorSet
{
public:
	virtual ~IDescriptorSet() = default;
	
	void Update(DescriptorSetUpdateInfo updateInfo);
};
using DescriptorSetPtr = Unique<IDescriptorSet>;


struct DescriptorPoolDesc 
{};

class IDescriptorPool
{
public: 
	virtual ~IDescriptorPool() = default;
	
	virtual DescriptorSetPtr AllocateDescriptorSet(const DescriptorSetLayout& layout) = 0;
};
using DescriptorPoolPtr = Unique<IDescriptorPool>;

} // namespace RHI
