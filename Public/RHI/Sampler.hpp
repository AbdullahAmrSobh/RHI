#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

struct SamplerDesc
{
    EFilter             filter;
    ESamplerAddressMode addressModeU;
    ESamplerAddressMode addressModeV;
    ESamplerAddressMode addressModeW;
    float               mipLodBias;
    float               maxAnisotropy;
    ECompareOp          compareOp;
    float               minLod;
    float               maxLod;
    EBorderColor        borderColor;
};

class ISampler
{
public:
    virtual ~ISampler() = default;
};
using SamplerPtr = Unique<ISampler>;

} // namespace RHI
