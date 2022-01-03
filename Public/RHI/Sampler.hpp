#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

enum class EFilter
{
    Nearest = 0,
    Linear  = 1,
};

enum class ESamplerAddressMode
{
    Repeat         = 0,
    MirroredRepeat = 1,
    ClampToEdge    = 2,
    ClampToBorder  = 3,
};

enum class EBorderColor
{
    FloatTransparentBlack = 0,
    IntTransparentBlack   = 1,
    FloatOpaqueBlack      = 2,
    IntOpaqueBlack        = 3,
    FloatOpaqueWhite      = 4,
    IntOpaqueWhite        = 5,
};

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
