#pragma once

#include <vulkan/vulkan.hpp>

#include "RHI/Result.hpp"
#include "RHI/Format.hpp"

namespace Vulkan
{

inline static RHI::ResultCode ConvertResult(vk::Result result)
{
    return {};
}

inline static vk::Format ConvertFormat(RHI::Format format)
{
    return {};
}

inline static vk::ImageSubresourceLayers ConvertImageSubresourceLayers(const RHI::ImageSubresource& subresource)
{
    return {};
}

inline static vk::Offset3D ConvertOffset3D(RHI::ImageSize imageSize);
inline static vk::Extent3D ConvertExtent3D(RHI::ImageSize imageSize);

}  // namespace Vulkan