#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/DescriptorSetLayout.hpp"

namespace RHI
{

struct ConstantBufferDesc
{
	ConstantBufferDesc() = default;
    ConstantBufferDesc(EShaderStageFlagBits stage, size_t offset, size_t range)
        : stage(stage)
        , offset(offset)
        , range(range)
    {
    }
    
    EShaderStageFlagBits stage;
    size_t               offset;
    size_t               range;
};

struct PipelineLayoutDesc
{
	PipelineLayoutDesc() = default;
    std::vector<IDescriptorSetLayout*> descriptorSetLayouts;
    std::vector<ConstantBufferDesc>    constantBufferDescs;
};

class IPipelineLayout
{
public:
    virtual ~IPipelineLayout() = default;
};
using PipelineLayoutPtr = Unique<IPipelineLayout>;
} // namespace RHI
