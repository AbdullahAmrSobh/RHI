#pragma once

#include "RHI/Handle.hpp"

namespace RHI
{
    /// @brief Declares an opaque resource handle for a Sampler.
    RHI_DECLARE_OPAQUE_RESOURCE(Sampler);

    /// @brief Specifies filtering options for a Sampler.
    enum class SamplerFilter
    {
        Point,  ///< Uses nearest neighbor filtering.
        Linear, ///< Uses linear filtering.
    };

    /// @brief Specifies the addressing modes for a Sampler.
    enum class SamplerAddressMode
    {
        Repeat, ///< Repeats the texture when UV coordinates are outside the range [0,1].
        Clamp,  ///< Clamps UV coordinates to the edge of the texture.
    };

    /// @brief Specifies the compare operation used for texture sampling.
    enum class SamplerCompareOperation
    {
        Never,     ///< Comparison always fails.
        Equal,     ///< Comparison passes if the values are equal.
        NotEqual,  ///< Comparison passes if the values are not equal.
        Always,    ///< Comparison always passes.
        Less,      ///< Comparison passes if the sampled value is less than the reference value.
        LessEq,    ///< Comparison passes if the sampled value is less than or equal to the reference value.
        Greater,   ///< Comparison passes if the sampled value is greater than the reference value.
        GreaterEq, ///< Comparison passes if the sampled value is greater than or equal to the reference value.
    };

    /// @brief Describes the parameters required to create a Sampler.
    struct SamplerCreateInfo
    {
        const char*             name       = nullptr;                         ///< Name of the sampler.
        SamplerFilter           filterMin  = SamplerFilter::Linear;           ///< Filter for minification.
        SamplerFilter           filterMag  = SamplerFilter::Linear;           ///< Filter for magnification.
        SamplerFilter           filterMip  = SamplerFilter::Linear;           ///< Filter for mipmap selection.
        SamplerCompareOperation compare    = SamplerCompareOperation::Always; ///< Compare operation for texture comparison.
        float                   mipLodBias = 0.0f;                            ///< Bias applied to the mip level of detail.
        SamplerAddressMode      addressU   = SamplerAddressMode::Repeat;      ///< Addressing mode for the U (horizontal) coordinate.
        SamplerAddressMode      addressV   = SamplerAddressMode::Repeat;      ///< Addressing mode for the V (vertical) coordinate.
        SamplerAddressMode      addressW   = SamplerAddressMode::Repeat;      ///< Addressing mode for the W (depth) coordinate.
        float                   minLod     = 0.0f;                            ///< Minimum level of detail (LOD) that can be used.
        float                   maxLod     = 1000.0f;                         ///< Maximum level of detail (LOD) that can be used.
    };
} // namespace RHI
