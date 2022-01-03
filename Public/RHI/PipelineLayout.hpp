#pragma once
#include "RHI/Definitions.hpp"
#include "RHi/Descriptor.hpp"

namespace RHI
{

struct PipelineLayoutDesc
{
    uint32_t             layoutCount;
    DescriptorSetLayout* pLayouts;
};

class IPipelineLayout
{
public:
    virtual ~IPipelineLayout() = default;
};
using PipelineLayoutPtr = Unique<IPipelineLayout>;

} // namespace RHI
