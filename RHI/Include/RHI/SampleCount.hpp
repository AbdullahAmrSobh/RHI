#pragma once

#include <TL/Flags.hpp>

namespace RHI
{
    /// @brief Specifies the number of samples for multisampling in rendering.
    enum class SampleCount
    {
        None      = 0 << 0, ///< No multisampling.
        Samples1  = 1 << 0, ///< Single sample per pixel.
        Samples2  = 1 << 1, ///< Two samples per pixel.
        Samples4  = 1 << 2, ///< Four samples per pixel.
        Samples8  = 1 << 3, ///< Eight samples per pixel.
        Samples16 = 1 << 4, ///< Sixteen samples per pixel.
        Samples32 = 1 << 5, ///< Thirty-two samples per pixel.
        Samples64 = 1 << 6, ///< Sixty-four samples per pixel.
    };

    /// @brief Enables bitwise flag operations for the SampleCount enum.
    TL_DEFINE_FLAG_OPERATORS(SampleCount);
} // namespace RHI
