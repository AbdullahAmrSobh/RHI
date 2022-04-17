#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

struct SamplerDesc
{
    SamplerDesc() = default;
	
    EFilter             filter        = EFilter::Linear;
    ESamplerAddressMode addressModeU  = ESamplerAddressMode::ClampToEdge;
    ESamplerAddressMode addressModeV  = ESamplerAddressMode::ClampToEdge;
    ESamplerAddressMode addressModeW  = ESamplerAddressMode::ClampToEdge;
    float               mipLodBias    = 0.0f;
    float               maxAnisotropy = 1.0f;
    ECompareOp          compareOp     = ECompareOp::Never;
    float               minLod        = 0.0f;
    float               maxLod        = 0.0f;
    EBorderColor        borderColor   = EBorderColor::FloatTransparentBlack;
};

class ISampler
{
public:
    virtual ~ISampler() = default;
};
using SamplerPtr = Unique<ISampler>;

} // namespace RHI
