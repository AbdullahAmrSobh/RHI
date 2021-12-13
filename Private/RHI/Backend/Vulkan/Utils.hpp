#pragma once
#include "RHI/Backend//Vulkan/Resources.hpp"
#include "RHI/Backend//Vulkan/vma/vk_mem_alloc.h"

#include <type_traits>

namespace RHI
{
namespace Vulkan
{

    namespace Utils
    {

        inline size_t hashCombine(size_t first, size_t second)
        {
            first ^= second + 0x9e3779b9 + (first << 6) + (first >> 2);
            return first;
        }

        inline VkFormat ToVkFormat(EPixelFormat format)
        {
            switch (format)
            {
            case EPixelFormat::None: return VK_FORMAT_UNDEFINED;
            case EPixelFormat::RGB: return VK_FORMAT_R8G8B8A8_SRGB;
            case EPixelFormat::RGBA: return VK_FORMAT_R8G8B8A8_SRGB;
            case EPixelFormat::BGRA: return VK_FORMAT_B8G8R8A8_SRGB;
            case EPixelFormat::Depth: return VK_FORMAT_D32_SFLOAT;
            case EPixelFormat::DepthStencil: return VK_FORMAT_D32_SFLOAT_S8_UINT;
            }
            return VK_FORMAT_UNDEFINED;
        }

        inline VkSampleCountFlagBits ToVkSampleCountFlagBits(ESampleCount sampleCount) { return static_cast<VkSampleCountFlagBits>(sampleCount); }

        inline VmaMemoryUsage ToVmaMemoryUsage(EDeviceMemoryAllocationUsage usage) { return static_cast<VmaMemoryUsage>(usage); }

        inline VkImageUsageFlags ToVkImageUsageFlags(TextureUsageFlags usage)
        {
            return static_cast<VkImageUsageFlags>(static_cast<TextureUsageFlags::MaskType>(usage));
        }

        inline VkBufferUsageFlags ToVkBufferUsageFlags(BufferUsageFlags usage)
        {
            return static_cast<VkBufferUsageFlags>(static_cast<BufferUsageFlags::MaskType>(usage));
        }

    } // namespace Utils

} // namespace Vulkan
} // namespace RHI
