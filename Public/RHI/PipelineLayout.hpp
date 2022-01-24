#pragma once
#include "RHI/Definitions.hpp"
#include "RHi/Descriptor.hpp"

namespace RHI
{

struct PipelineLayoutDesc
{
	ArrayView<DescriptorSetLayout> descriptorSetLayouts;
};

class IPipelineLayout
{
public:
    virtual ~IPipelineLayout() = default;
};
using PipelineLayoutPtr = Unique<IPipelineLayout>;

} // namespace RHI
