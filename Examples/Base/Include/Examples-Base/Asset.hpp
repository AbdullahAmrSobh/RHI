#pragma once

#include <RHI/RHI.hpp>

#include "dds_image/dds.hpp"

namespace Examples
{
    struct ImageAsset
    {
        RHI::ImageType type;
        RHI::ImageSize3D size;
        RHI::Format format;
        const std::vector<uint8_t> data;
    };

} // namespace Examples