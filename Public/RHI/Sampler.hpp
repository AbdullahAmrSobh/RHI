#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

struct SamplerDesc
{
    explicit SamplerDesc(EFilter filter = EFilter::Linear, ESamplerAddressMode addressModeU = ESamplerAddressMode::ClampToEdge,
                         ESamplerAddressMode addressModeV = ESamplerAddressMode::ClampToEdge,
                         ESamplerAddressMode addressModeW = ESamplerAddressMode::ClampToEdge, float mipLodBias = 0.0f, float maxAnisotropy = 1.0f,
                         ECompareOp compareOp = ECompareOp::Never, float minLod = 0.0f, float maxLod = 0.0f,
                         EBorderColor borderColor = EBorderColor::FloatTransparentBlack)
        : filter(filter)
        , addressModeU(addressModeU)
        , addressModeV(addressModeV)
        , addressModeW(addressModeW)
        , mipLodBias(mipLodBias)
        , maxAnisotropy(maxAnisotropy)
        , compareOp(compareOp)
        , minLod(minLod)
        , maxLod(maxLod)
        , borderColor(borderColor)
    {
    }

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
