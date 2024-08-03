#pragma once
#include <RHI/CommandList.hpp>
#include <RHI/Resources.hpp>
#include <RHI/RenderGraph.hpp>

#include <RHI/Common/Result.hpp>

#include <vk_mem_alloc.h>

#define TryValidateVk(result)           \
    if (Validate(result) != VK_SUCCESS) \
        return ConvertResult(result);

#define TryValidate(result)            \
    if (result != ResultCode::Success) \
        return result;

namespace RHI::Vulkan
{
    [[maybe_unused]] inline static bool Validate(ResultCode result)
    {
        RHI_ASSERT(IsSucess(result));
        return IsSucess(result);
    }

    [[maybe_unused]] inline static bool Validate(VkResult result)
    {
        RHI_ASSERT(result == VK_SUCCESS);
        return result;
    }

    inline static ResultCode ConvertResult(VkResult result)
    {
        switch (result)
        {
        case VK_SUCCESS:
            return ResultCode::Success;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return ResultCode::ErrorOutOfMemory;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return ResultCode::ErrorDeviceOutOfMemory;
        default:
            return ResultCode::ErrorUnkown;
        }
    }

    inline static VkFormat ConvertFormat(Format format)
    {
        RHI_ASSERT(format < Format::COUNT);
        switch (format)
        {
        case Format::Unknown:           return VK_FORMAT_UNDEFINED;
        case Format::R8_UINT:           return VK_FORMAT_R8_UINT;
        case Format::R8_SINT:           return VK_FORMAT_R8_SINT;
        case Format::R8_UNORM:          return VK_FORMAT_R8_UNORM;
        case Format::R8_SNORM:          return VK_FORMAT_R8_SNORM;
        case Format::RG8_UINT:          return VK_FORMAT_R8G8_UINT;
        case Format::RG8_SINT:          return VK_FORMAT_R8G8_SINT;
        case Format::RG8_UNORM:         return VK_FORMAT_R8G8_UNORM;
        case Format::RG8_SNORM:         return VK_FORMAT_R8G8_SNORM;
        case Format::R16_UINT:          return VK_FORMAT_R16_UINT;
        case Format::R16_SINT:          return VK_FORMAT_R16_SINT;
        case Format::R16_UNORM:         return VK_FORMAT_R16_UNORM;
        case Format::R16_SNORM:         return VK_FORMAT_R16_SNORM;
        case Format::R16_FLOAT:         return VK_FORMAT_R16_SFLOAT;
        case Format::BGRA4_UNORM:       return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
        case Format::B5G6R5_UNORM:      return VK_FORMAT_B5G6R5_UNORM_PACK16;
        case Format::B5G5R5A1_UNORM:    return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
        case Format::RGBA8_UINT:        return VK_FORMAT_R8G8B8A8_UINT;
        case Format::RGBA8_SINT:        return VK_FORMAT_R8G8B8A8_SINT;
        case Format::RGBA8_UNORM:       return VK_FORMAT_R8G8B8A8_UNORM;
        case Format::RGBA8_SNORM:       return VK_FORMAT_R8G8B8A8_SNORM;
        case Format::BGRA8_UNORM:       return VK_FORMAT_B8G8R8A8_UNORM;
        case Format::SRGBA8_UNORM:      return VK_FORMAT_R8G8B8A8_SRGB;
        case Format::SBGRA8_UNORM:      return VK_FORMAT_B8G8R8A8_SRGB;
        case Format::R10G10B10A2_UNORM: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case Format::R11G11B10_FLOAT:   return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case Format::RG16_UINT:         return VK_FORMAT_R16G16_UINT;
        case Format::RG16_SINT:         return VK_FORMAT_R16G16_SINT;
        case Format::RG16_UNORM:        return VK_FORMAT_R16G16_UNORM;
        case Format::RG16_SNORM:        return VK_FORMAT_R16G16_SNORM;
        case Format::RG16_FLOAT:        return VK_FORMAT_R16G16_SFLOAT;
        case Format::R32_UINT:          return VK_FORMAT_R32_UINT;
        case Format::R32_SINT:          return VK_FORMAT_R32_SINT;
        case Format::R32_FLOAT:         return VK_FORMAT_R32_SFLOAT;
        case Format::RGBA16_UINT:       return VK_FORMAT_R16G16B16A16_UINT;
        case Format::RGBA16_SINT:       return VK_FORMAT_R16G16B16A16_SINT;
        case Format::RGBA16_FLOAT:      return VK_FORMAT_R16G16B16A16_SFLOAT;
        case Format::RGBA16_UNORM:      return VK_FORMAT_R16G16B16A16_UNORM;
        case Format::RGBA16_SNORM:      return VK_FORMAT_R16G16B16A16_SNORM;
        case Format::RG32_UINT:         return VK_FORMAT_R32G32_UINT;
        case Format::RG32_SINT:         return VK_FORMAT_R32G32_SINT;
        case Format::RG32_FLOAT:        return VK_FORMAT_R32G32_SFLOAT;
        case Format::RGB32_UINT:        return VK_FORMAT_R32G32B32_UINT;
        case Format::RGB32_SINT:        return VK_FORMAT_R32G32B32_SINT;
        case Format::RGB32_FLOAT:       return VK_FORMAT_R32G32B32_SFLOAT;
        case Format::RGBA32_UINT:       return VK_FORMAT_R32G32B32A32_UINT;
        case Format::RGBA32_SINT:       return VK_FORMAT_R32G32B32A32_SINT;
        case Format::RGBA32_FLOAT:      return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Format::D16:               return VK_FORMAT_D16_UNORM;
        case Format::D24S8:             return VK_FORMAT_D24_UNORM_S8_UINT;
        case Format::X24G8_UINT:        return VK_FORMAT_D24_UNORM_S8_UINT;
        case Format::D32:               return VK_FORMAT_D32_SFLOAT;
        case Format::D32S8:             return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case Format::X32G8_UINT:        return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case Format::BC1_UNORM:         return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case Format::BC1_UNORM_SRGB:    return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case Format::BC2_UNORM:         return VK_FORMAT_BC2_UNORM_BLOCK;
        case Format::BC2_UNORM_SRGB:    return VK_FORMAT_BC2_SRGB_BLOCK;
        case Format::BC3_UNORM:         return VK_FORMAT_BC3_UNORM_BLOCK;
        case Format::BC3_UNORM_SRGB:    return VK_FORMAT_BC3_SRGB_BLOCK;
        case Format::BC4_UNORM:         return VK_FORMAT_BC4_UNORM_BLOCK;
        case Format::BC4_SNORM:         return VK_FORMAT_BC4_SNORM_BLOCK;
        case Format::BC5_UNORM:         return VK_FORMAT_BC5_UNORM_BLOCK;
        case Format::BC5_SNORM:         return VK_FORMAT_BC5_SNORM_BLOCK;
        case Format::BC6H_UFLOAT:       return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case Format::BC6H_SFLOAT:       return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case Format::BC7_UNORM:         return VK_FORMAT_BC7_UNORM_BLOCK;
        case Format::BC7_UNORM_SRGB:    return VK_FORMAT_BC7_SRGB_BLOCK;
        default:                        return VK_FORMAT_UNDEFINED;
        }
    }

    inline static Format ConvertFormat(VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_UNDEFINED:                return Format::Unknown;
        case VK_FORMAT_R8_UINT:                  return Format::R8_UINT;
        case VK_FORMAT_R8_SINT:                  return Format::R8_SINT;
        case VK_FORMAT_R8_UNORM:                 return Format::R8_UNORM;
        case VK_FORMAT_R8_SNORM:                 return Format::R8_SNORM;
        case VK_FORMAT_R8G8_UINT:                return Format::RG8_UINT;
        case VK_FORMAT_R8G8_SINT:                return Format::RG8_SINT;
        case VK_FORMAT_R8G8_UNORM:               return Format::RG8_UNORM;
        case VK_FORMAT_R8G8_SNORM:               return Format::RG8_SNORM;
        case VK_FORMAT_R16_UINT:                 return Format::R16_UINT;
        case VK_FORMAT_R16_SINT:                 return Format::R16_SINT;
        case VK_FORMAT_R16_UNORM:                return Format::R16_UNORM;
        case VK_FORMAT_R16_SNORM:                return Format::R16_SNORM;
        case VK_FORMAT_R16_SFLOAT:               return Format::R16_FLOAT;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:    return Format::BGRA4_UNORM;
        case VK_FORMAT_B5G6R5_UNORM_PACK16:      return Format::B5G6R5_UNORM;
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:    return Format::B5G5R5A1_UNORM;
        case VK_FORMAT_R8G8B8A8_UINT:            return Format::RGBA8_UINT;
        case VK_FORMAT_R8G8B8A8_SINT:            return Format::RGBA8_SINT;
        case VK_FORMAT_R8G8B8A8_UNORM:           return Format::RGBA8_UNORM;
        case VK_FORMAT_R8G8B8A8_SNORM:           return Format::RGBA8_SNORM;
        case VK_FORMAT_B8G8R8A8_UNORM:           return Format::BGRA8_UNORM;
        case VK_FORMAT_R8G8B8A8_SRGB:            return Format::SRGBA8_UNORM;
        case VK_FORMAT_B8G8R8A8_SRGB:            return Format::SBGRA8_UNORM;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return Format::R10G10B10A2_UNORM;
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:  return Format::R11G11B10_FLOAT;
        case VK_FORMAT_R16G16_UINT:              return Format::RG16_UINT;
        case VK_FORMAT_R16G16_SINT:              return Format::RG16_SINT;
        case VK_FORMAT_R16G16_UNORM:             return Format::RG16_UNORM;
        case VK_FORMAT_R16G16_SNORM:             return Format::RG16_SNORM;
        case VK_FORMAT_R16G16_SFLOAT:            return Format::RG16_FLOAT;
        case VK_FORMAT_R32_UINT:                 return Format::R32_UINT;
        case VK_FORMAT_R32_SINT:                 return Format::R32_SINT;
        case VK_FORMAT_R32_SFLOAT:               return Format::R32_FLOAT;
        case VK_FORMAT_R16G16B16A16_UINT:        return Format::RGBA16_UINT;
        case VK_FORMAT_R16G16B16A16_SINT:        return Format::RGBA16_SINT;
        case VK_FORMAT_R16G16B16A16_SFLOAT:      return Format::RGBA16_FLOAT;
        case VK_FORMAT_R16G16B16A16_UNORM:       return Format::RGBA16_UNORM;
        case VK_FORMAT_R16G16B16A16_SNORM:       return Format::RGBA16_SNORM;
        case VK_FORMAT_R32G32_UINT:              return Format::RG32_UINT;
        case VK_FORMAT_R32G32_SINT:              return Format::RG32_SINT;
        case VK_FORMAT_R32G32_SFLOAT:            return Format::RG32_FLOAT;
        case VK_FORMAT_R32G32B32_UINT:           return Format::RGB32_UINT;
        case VK_FORMAT_R32G32B32_SINT:           return Format::RGB32_SINT;
        case VK_FORMAT_R32G32B32_SFLOAT:         return Format::RGB32_FLOAT;
        case VK_FORMAT_R32G32B32A32_UINT:        return Format::RGBA32_UINT;
        case VK_FORMAT_R32G32B32A32_SINT:        return Format::RGBA32_SINT;
        case VK_FORMAT_R32G32B32A32_SFLOAT:      return Format::RGBA32_FLOAT;
        case VK_FORMAT_D16_UNORM:                return Format::D16;
        case VK_FORMAT_D24_UNORM_S8_UINT:        return Format::D24S8;
        case VK_FORMAT_D32_SFLOAT:               return Format::D32;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:       return Format::D32S8;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:     return Format::BC1_UNORM;
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:      return Format::BC1_UNORM_SRGB;
        case VK_FORMAT_BC2_UNORM_BLOCK:          return Format::BC2_UNORM;
        case VK_FORMAT_BC2_SRGB_BLOCK:           return Format::BC2_UNORM_SRGB;
        case VK_FORMAT_BC3_UNORM_BLOCK:          return Format::BC3_UNORM;
        case VK_FORMAT_BC3_SRGB_BLOCK:           return Format::BC3_UNORM_SRGB;
        case VK_FORMAT_BC4_UNORM_BLOCK:          return Format::BC4_UNORM;
        case VK_FORMAT_BC4_SNORM_BLOCK:          return Format::BC4_SNORM;
        case VK_FORMAT_BC5_UNORM_BLOCK:          return Format::BC5_UNORM;
        case VK_FORMAT_BC5_SNORM_BLOCK:          return Format::BC5_SNORM;
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:        return Format::BC6H_UFLOAT;
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:        return Format::BC6H_SFLOAT;
        case VK_FORMAT_BC7_UNORM_BLOCK:          return Format::BC7_UNORM;
        case VK_FORMAT_BC7_SRGB_BLOCK:           return Format::BC7_UNORM_SRGB;
        default:                                 RHI_UNREACHABLE(); return Format::Unknown;
        }
    }

    inline static VkAttachmentLoadOp ConvertLoadOp(LoadOperation op)
    {
        switch (op)
        {
        case LoadOperation::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        case LoadOperation::Load:     return VK_ATTACHMENT_LOAD_OP_LOAD;
        case LoadOperation::Discard:  return VK_ATTACHMENT_LOAD_OP_CLEAR;
        default:                      RHI_UNREACHABLE(); return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
        }
    }

    inline static VkAttachmentStoreOp ConvertStoreOp(StoreOperation op)
    {
        switch (op)
        {
        case StoreOperation::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        case StoreOperation::Store:    return VK_ATTACHMENT_STORE_OP_STORE;
        case StoreOperation::Discard:  return VK_ATTACHMENT_STORE_OP_NONE;
        default:                       RHI_UNREACHABLE(); return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
        }
    }

    inline static VkPipelineStageFlags2 ConvertPipelineStageFlags(ShaderStage stage)
    {
        switch (stage)
        {
        case ShaderStage::Vertex:  return VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        case ShaderStage::Pixel:   return VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        case ShaderStage::Compute: return VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        default:
            {
                RHI_UNREACHABLE();
            }
        }

        return VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    }

    inline static VkSampleCountFlagBits ConvertSampleCount(SampleCount sampleCount)
    {
        switch (sampleCount)
        {
        case SampleCount::None:      return VK_SAMPLE_COUNT_1_BIT;
        case SampleCount::Samples1:  return VK_SAMPLE_COUNT_1_BIT;
        case SampleCount::Samples2:  return VK_SAMPLE_COUNT_2_BIT;
        case SampleCount::Samples4:  return VK_SAMPLE_COUNT_4_BIT;
        case SampleCount::Samples8:  return VK_SAMPLE_COUNT_8_BIT;
        case SampleCount::Samples16: return VK_SAMPLE_COUNT_16_BIT;
        case SampleCount::Samples32: return VK_SAMPLE_COUNT_32_BIT;
        case SampleCount::Samples64: return VK_SAMPLE_COUNT_64_BIT;
        default:                     RHI_UNREACHABLE(); return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
        }
    }

    inline static VkSampleCountFlags ConvertSampleCountFlags(TL::Flags<SampleCount> sampleCountFlags)
    {
        VkSampleCountFlags result = 0;
        if (sampleCountFlags & SampleCount::Samples1)
            result |= VK_SAMPLE_COUNT_1_BIT;
        if (sampleCountFlags & SampleCount::Samples2)
            result |= VK_SAMPLE_COUNT_2_BIT;
        if (sampleCountFlags & SampleCount::Samples4)
            result |= VK_SAMPLE_COUNT_4_BIT;
        if (sampleCountFlags & SampleCount::Samples8)
            result |= VK_SAMPLE_COUNT_8_BIT;
        if (sampleCountFlags & SampleCount::Samples16)
            result |= VK_SAMPLE_COUNT_16_BIT;
        if (sampleCountFlags & SampleCount::Samples32)
            result |= VK_SAMPLE_COUNT_32_BIT;
        if (sampleCountFlags & SampleCount::Samples64)
            result |= VK_SAMPLE_COUNT_64_BIT;
        return result;
    }

    inline static VkImageUsageFlagBits ConvertImageUsage(ImageUsage imageUsage)
    {
        switch (imageUsage)
        {
        case ImageUsage::None:            return VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
        case ImageUsage::ShaderResource:  return VK_IMAGE_USAGE_SAMPLED_BIT;
        case ImageUsage::StorageResource: return VK_IMAGE_USAGE_STORAGE_BIT;
        case ImageUsage::Color:           return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        case ImageUsage::Depth:           return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        case ImageUsage::Stencil:         return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        case ImageUsage::CopySrc:         return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        case ImageUsage::CopyDst:         return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        default:                          RHI_UNREACHABLE(); return VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
        }
    }

    inline static VkImageUsageFlags ConvertImageUsageFlags(TL::Flags<ImageUsage> imageUsageFlags)
    {
        VkImageUsageFlags result = 0;
        if (imageUsageFlags & ImageUsage::ShaderResource)
            result |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (imageUsageFlags & ImageUsage::StorageResource)
            result |= VK_IMAGE_USAGE_STORAGE_BIT;
        if (imageUsageFlags & ImageUsage::Color)
            result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (imageUsageFlags & ImageUsage::Depth)
            result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (imageUsageFlags & ImageUsage::Stencil)
            result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (imageUsageFlags & ImageUsage::CopySrc)
            result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (imageUsageFlags & ImageUsage::CopyDst)
            result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        return result;
    }

    inline static VkImageType ConvertImageType(ImageType imageType)
    {
        switch (imageType)
        {
        case ImageType::None:    return VK_IMAGE_TYPE_MAX_ENUM;
        case ImageType::Image1D: return VK_IMAGE_TYPE_1D;
        case ImageType::Image2D: return VK_IMAGE_TYPE_2D;
        case ImageType::Image3D: return VK_IMAGE_TYPE_3D;
        default:                 RHI_UNREACHABLE(); return VK_IMAGE_TYPE_MAX_ENUM;
        }
    }

    inline static VkImageAspectFlagBits ConvertImageAspect(TL::Flags<ImageAspect> imageAspect)
    {
        if (imageAspect & ImageAspect::Color)
            return VK_IMAGE_ASPECT_COLOR_BIT;
        if (imageAspect & ImageAspect::Depth)
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        if (imageAspect & ImageAspect::Stencil)
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        RHI_UNREACHABLE();
        return VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
    }

    inline static VkImageAspectFlags ConvertImageAspect(ImageAspect imageAspect)
    {
        switch (imageAspect)
        {
        case ImageAspect::None:         return VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
        case ImageAspect::Color:        return VK_IMAGE_ASPECT_COLOR_BIT;
        case ImageAspect::Depth:        return VK_IMAGE_ASPECT_DEPTH_BIT;
        case ImageAspect::Stencil:      return VK_IMAGE_ASPECT_STENCIL_BIT;
        case ImageAspect::DepthStencil: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        case ImageAspect::All:          return VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:                        RHI_UNREACHABLE(); return VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
        }
    }

    inline static VkImageAspectFlags FormatToAspect(VkFormat format)
    {
        if (format == VK_FORMAT_D32_SFLOAT)
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        else
            return VK_IMAGE_ASPECT_COLOR_BIT;
    }

    inline static VkComponentSwizzle ConvertComponentSwizzle(ComponentSwizzle componentSwizzle)
    {
        switch (componentSwizzle)
        {
        case ComponentSwizzle::Identity: return VK_COMPONENT_SWIZZLE_IDENTITY;
        case ComponentSwizzle::Zero:     return VK_COMPONENT_SWIZZLE_ZERO;
        case ComponentSwizzle::One:      return VK_COMPONENT_SWIZZLE_ONE;
        case ComponentSwizzle::R:        return VK_COMPONENT_SWIZZLE_R;
        case ComponentSwizzle::G:        return VK_COMPONENT_SWIZZLE_G;
        case ComponentSwizzle::B:        return VK_COMPONENT_SWIZZLE_B;
        case ComponentSwizzle::A:        return VK_COMPONENT_SWIZZLE_A;
        default:                         RHI_UNREACHABLE(); return VK_COMPONENT_SWIZZLE_IDENTITY;
        }
    }

    inline static VkBufferUsageFlagBits ConvertBufferUsage(BufferUsage bufferUsage)
    {
        switch (bufferUsage)
        {
        case BufferUsage::None:    return VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
        case BufferUsage::Storage: return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        case BufferUsage::Uniform: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        case BufferUsage::Vertex:  return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case BufferUsage::Index:   return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case BufferUsage::CopySrc: return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        case BufferUsage::CopyDst: return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        default:                   RHI_UNREACHABLE(); return VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
        }
    }

    inline static VkBufferUsageFlags ConvertBufferUsageFlags(TL::Flags<BufferUsage> bufferUsageFlags)
    {
        VkBufferUsageFlags result = 0;
        if (bufferUsageFlags & BufferUsage::Storage)
            result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::Uniform)
            result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::Vertex)
            result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::Index)
            result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (bufferUsageFlags & BufferUsage::CopySrc)
            result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        if (bufferUsageFlags & BufferUsage::CopyDst)
            result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        return result;
    }

    inline static VkShaderStageFlagBits ConvertShaderStage(ShaderStage shaderStage)
    {
        switch (shaderStage)
        {
        case ShaderStage::Vertex:  return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::Pixel:   return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
        default:                   RHI_UNREACHABLE(); return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        }
    }

    inline static VkShaderStageFlags ConvertShaderStage(TL::Flags<ShaderStage> shaderStageFlags)
    {
        VkShaderStageFlags result = 0;
        if (shaderStageFlags & ShaderStage::Vertex)
            result |= VK_SHADER_STAGE_VERTEX_BIT;
        if (shaderStageFlags & ShaderStage::Pixel)
            result |= VK_SHADER_STAGE_FRAGMENT_BIT;
        if (shaderStageFlags & ShaderStage::Compute)
            result |= VK_SHADER_STAGE_COMPUTE_BIT;
        return result;
    }

    inline static VkDescriptorType ConvertDescriptorType(BindingType bindingType)
    {
        switch (bindingType)
        {
        case BindingType::Sampler:              return VK_DESCRIPTOR_TYPE_SAMPLER;
        case BindingType::SampledImage:         return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case BindingType::StorageImage:         return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case BindingType::UniformBuffer:        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case BindingType::StorageBuffer:        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case BindingType::DynamicUniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        case BindingType::DynamicStorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        case BindingType::BufferView:           return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case BindingType::StorageBufferView:    return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        default:                                RHI_UNREACHABLE(); return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        }
    }

    inline static VkVertexInputRate ConvertVertexInputRate(PipelineVertexInputRate inputRate)
    {
        switch (inputRate)
        {
        case PipelineVertexInputRate::PerInstance: return VK_VERTEX_INPUT_RATE_INSTANCE;
        case PipelineVertexInputRate::PerVertex:   return VK_VERTEX_INPUT_RATE_VERTEX;
        default:                                   RHI_UNREACHABLE(); return VK_VERTEX_INPUT_RATE_MAX_ENUM;
        }
    }

    inline static VkCullModeFlags ConvertCullModeFlags(PipelineRasterizerStateCullMode cullMode)
    {
        switch (cullMode)
        {
        case PipelineRasterizerStateCullMode::None:      return VK_CULL_MODE_NONE;
        case PipelineRasterizerStateCullMode::FrontFace: return VK_CULL_MODE_FRONT_BIT;
        case PipelineRasterizerStateCullMode::BackFace:  return VK_CULL_MODE_BACK_BIT;
        case PipelineRasterizerStateCullMode::Discard:   return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
        default:                                         RHI_UNREACHABLE(); return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
        }
    }

    inline static VkPolygonMode ConvertPolygonMode(PipelineRasterizerStateFillMode fillMode)
    {
        switch (fillMode)
        {
        case PipelineRasterizerStateFillMode::Point:    return VK_POLYGON_MODE_POINT;
        case PipelineRasterizerStateFillMode::Triangle: return VK_POLYGON_MODE_FILL;
        case PipelineRasterizerStateFillMode::Line:     return VK_POLYGON_MODE_LINE;
        default:                                        RHI_UNREACHABLE(); return VK_POLYGON_MODE_MAX_ENUM;
        }
    }

    inline static VkPrimitiveTopology ConvertPrimitiveTopology(PipelineTopologyMode topologyMode)
    {
        switch (topologyMode)
        {
        case PipelineTopologyMode::Points:    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PipelineTopologyMode::Lines:     return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PipelineTopologyMode::Triangles: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        default:                              RHI_UNREACHABLE(); return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
        }
    }

    inline static VkFrontFace ConvertFrontFace(PipelineRasterizerStateFrontFace frontFace)
    {
        switch (frontFace)
        {
        case PipelineRasterizerStateFrontFace::Clockwise:        return VK_FRONT_FACE_CLOCKWISE;
        case PipelineRasterizerStateFrontFace::CounterClockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        default:                                                 RHI_UNREACHABLE(); return VK_FRONT_FACE_MAX_ENUM;
        }
    }

    inline static VkCompareOp ConvertCompareOp(CompareOperator compareOperator)
    {
        switch (compareOperator)
        {
        case CompareOperator::Never:          return VK_COMPARE_OP_NEVER;
        case CompareOperator::Equal:          return VK_COMPARE_OP_EQUAL;
        case CompareOperator::NotEqual:       return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOperator::Greater:        return VK_COMPARE_OP_GREATER;
        case CompareOperator::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOperator::Less:           return VK_COMPARE_OP_LESS;
        case CompareOperator::LessOrEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOperator::Always:         return VK_COMPARE_OP_ALWAYS;
        default:                              RHI_UNREACHABLE(); return VK_COMPARE_OP_MAX_ENUM;
        }
    }

    inline static VkFilter ConvertFilter(SamplerFilter samplerFilter)
    {
        switch (samplerFilter)
        {
        case SamplerFilter::Point:  return VK_FILTER_NEAREST;
        case SamplerFilter::Linear: return VK_FILTER_LINEAR;
        default:                    RHI_UNREACHABLE(); return VK_FILTER_MAX_ENUM;
        }
    }

    inline static VkSamplerAddressMode ConvertSamplerAddressMode(SamplerAddressMode addressMode)
    {
        switch (addressMode)
        {
        case SamplerAddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerAddressMode::Clamp:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        default:                         RHI_UNREACHABLE(); return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
        }
    }

    inline static VkCompareOp ConvertCompareOp(SamplerCompareOperation compareOperation)
    {
        switch (compareOperation)
        {
        case SamplerCompareOperation::Never:     return VK_COMPARE_OP_NEVER;
        case SamplerCompareOperation::Equal:     return VK_COMPARE_OP_EQUAL;
        case SamplerCompareOperation::NotEqual:  return VK_COMPARE_OP_NOT_EQUAL;
        case SamplerCompareOperation::Always:    return VK_COMPARE_OP_ALWAYS;
        case SamplerCompareOperation::Less:      return VK_COMPARE_OP_LESS;
        case SamplerCompareOperation::LessEq:    return VK_COMPARE_OP_LESS_OR_EQUAL;
        case SamplerCompareOperation::Greater:   return VK_COMPARE_OP_GREATER;
        case SamplerCompareOperation::GreaterEq: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        default:                                 RHI_UNREACHABLE(); return VK_COMPARE_OP_MAX_ENUM;
        }
    }

    inline static VkBlendFactor ConvertBlendFactor(BlendFactor blendFactor)
    {
        switch (blendFactor)
        {
        case BlendFactor::Zero:                  return VK_BLEND_FACTOR_ZERO;
        case BlendFactor::One:                   return VK_BLEND_FACTOR_ONE;
        case BlendFactor::SrcColor:              return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFactor::OneMinusSrcColor:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DstColor:              return VK_BLEND_FACTOR_DST_COLOR;
        case BlendFactor::OneMinusDstColor:      return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendFactor::SrcAlpha:              return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFactor::OneMinusSrcAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DstAlpha:              return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendFactor::OneMinusDstAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendFactor::ConstantColor:         return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case BlendFactor::OneMinusConstantColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case BlendFactor::ConstantAlpha:         return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case BlendFactor::OneMinusConstantAlpha: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        default:                                 RHI_UNREACHABLE(); return VK_BLEND_FACTOR_MAX_ENUM;
        }
    }

    inline static VkBlendOp ConvertBlendOp(BlendEquation blendEquation)
    {
        switch (blendEquation)
        {
        case BlendEquation::Add:             return VK_BLEND_OP_ADD;
        case BlendEquation::Subtract:        return VK_BLEND_OP_SUBTRACT;
        case BlendEquation::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendEquation::Min:             return VK_BLEND_OP_MIN;
        case BlendEquation::Max:             return VK_BLEND_OP_MAX;
        default:                             RHI_UNREACHABLE(); return VK_BLEND_OP_MAX_ENUM;
        }
    }

    inline static VkCommandPoolCreateFlags ConvertCommandPoolFlags(TL::Flags<CommandPoolFlags> flags)
    {
        VkCommandPoolCreateFlags result{};
        if (flags & CommandPoolFlags::Transient)
            result |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        if (flags & CommandPoolFlags::Reset)
            result |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        return result;
    }

    inline static VkImageSubresource ConvertSubresource(const ImageSubresource& subresource)
    {
        auto vkSubresource = VkImageSubresource{};
        vkSubresource.aspectMask = ConvertImageAspect(subresource.imageAspects);
        vkSubresource.mipLevel = subresource.mipLevel;
        vkSubresource.arrayLayer = subresource.arrayLayer;
        return vkSubresource;
    }

    inline static VkImageSubresourceLayers ConvertSubresourceLayer(const ImageSubresourceLayers& subresource)
    {
        auto vkSubresource = VkImageSubresourceLayers{};
        vkSubresource.aspectMask = ConvertImageAspect(subresource.imageAspects);
        vkSubresource.mipLevel = subresource.mipLevel;
        vkSubresource.baseArrayLayer = subresource.arrayBase;
        vkSubresource.layerCount = subresource.arrayCount;
        return vkSubresource;
    }

    inline static VkImageSubresourceRange ConvertSubresourceRange(const ImageSubresourceRange& subresource)
    {
        auto vkSubresource = VkImageSubresourceRange{};
        vkSubresource.aspectMask = ConvertImageAspect(subresource.imageAspects);
        vkSubresource.baseMipLevel = subresource.mipBase;
        vkSubresource.levelCount = subresource.mipLevelCount;
        vkSubresource.baseArrayLayer = subresource.arrayBase;
        vkSubresource.layerCount = subresource.arrayCount;
        return vkSubresource;
    }

    inline static VkExtent2D ConvertExtent2D(ImageSize2D size)
    {
        return { size.width, size.height };
    }

    inline static VkExtent3D ConvertExtent3D(ImageSize3D size)
    {
        return { size.width, size.height, size.depth };
    }

    inline static VkExtent2D ConvertExtent2D(ImageSize3D size)
    {
        RHI_ASSERT(size.depth == 0);
        return { size.width, size.height };
    }

    inline static VkOffset2D ConvertOffset2D(ImageOffset2D offset)
    {
        return { offset.x, offset.y };
    }

    inline static VkOffset3D ConvertOffset3D(ImageOffset3D offset)
    {
        return { offset.x, offset.y, offset.z };
    }

    template<typename T>
    inline static VkClearColorValue ConvertColorValue(ColorValue<T> value)
    {
        VkClearColorValue clearValue = {};
        clearValue.float32[0] = value.r;
        clearValue.float32[1] = value.g;
        clearValue.float32[2] = value.b;
        clearValue.float32[3] = value.a;
        return clearValue;
    }

    inline static VkClearDepthStencilValue ConvertDepthStencilValue(DepthStencilValue value)
    {
        VkClearDepthStencilValue clearValue = {};
        clearValue.depth = value.depthValue;
        clearValue.stencil = value.stencilValue;
        return clearValue;
    }

    inline static VkClearColorValue ConvertClearValue(ClearValue clearValue)
    {
        return ConvertColorValue(clearValue.f32);
    };

    inline static VkComponentMapping ConvertComponentMapping(ComponentMapping componentMapping)
    {
        VkComponentMapping mapping{};
        mapping.r = ConvertComponentSwizzle(componentMapping.r);
        mapping.g = ConvertComponentSwizzle(componentMapping.g);
        mapping.b = ConvertComponentSwizzle(componentMapping.b);
        mapping.a = ConvertComponentSwizzle(componentMapping.a);
        return mapping;
    }

    template<typename VkResourceTypeT>
    inline static VkObjectType GetObjectType()
    {
        // clang-format off
        if constexpr      (std::is_same_v<VkResourceTypeT, VkInstance>)                    return VK_OBJECT_TYPE_INSTANCE;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkPhysicalDevice>)              return VK_OBJECT_TYPE_PHYSICAL_DEVICE;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkDevice>)                      return VK_OBJECT_TYPE_DEVICE;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkQueue>)                       return VK_OBJECT_TYPE_QUEUE;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkSemaphore>)                   return VK_OBJECT_TYPE_SEMAPHORE;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkCommandBuffer>)               return VK_OBJECT_TYPE_COMMAND_BUFFER;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkFence>)                       return VK_OBJECT_TYPE_FENCE;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkDeviceMemory>)                return VK_OBJECT_TYPE_DEVICE_MEMORY;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkBuffer>)                      return VK_OBJECT_TYPE_BUFFER;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkImage>)                       return VK_OBJECT_TYPE_IMAGE;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkEvent>)                       return VK_OBJECT_TYPE_EVENT;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkQueryPool>)                   return VK_OBJECT_TYPE_QUERY_POOL;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkBufferView>)                  return VK_OBJECT_TYPE_BUFFER_VIEW;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkImageView>)                   return VK_OBJECT_TYPE_IMAGE_VIEW;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkShaderModule>)                return VK_OBJECT_TYPE_SHADER_MODULE;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkPipelineCache>)               return VK_OBJECT_TYPE_PIPELINE_CACHE;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkPipelineLayout>)              return VK_OBJECT_TYPE_PIPELINE_LAYOUT;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkRenderPass>)                  return VK_OBJECT_TYPE_RENDER_PASS;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkPipeline>)                    return VK_OBJECT_TYPE_PIPELINE;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkDescriptorSetLayout>)         return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkSampler>)                     return VK_OBJECT_TYPE_SAMPLER;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkDescriptorPool>)              return VK_OBJECT_TYPE_DESCRIPTOR_POOL;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkDescriptorSet>)               return VK_OBJECT_TYPE_DESCRIPTOR_SET;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkFramebuffer>)                 return VK_OBJECT_TYPE_FRAMEBUFFER;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkCommandPool>)                 return VK_OBJECT_TYPE_COMMAND_POOL;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkSurfaceKHR>)                  return VK_OBJECT_TYPE_SURFACE_KHR;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkSwapchainKHR>)                return VK_OBJECT_TYPE_SWAPCHAIN_KHR;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkDebugReportCallbackEXT>)      return VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkDisplayKHR>)                  return VK_OBJECT_TYPE_DISPLAY_KHR;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkDisplayModeKHR>)              return VK_OBJECT_TYPE_DISPLAY_MODE_KHR;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkValidationCacheEXT>)          return VK_OBJECT_TYPE_VALIDATION_CACHE_EXT;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkSamplerYcbcrConversion>)      return VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkDescriptorUpdateTemplate>)    return VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkCuModuleNVX>)                 return VK_OBJECT_TYPE_CU_MODULE_NVX;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkCuFunctionNVX>)               return VK_OBJECT_TYPE_CU_FUNCTION_NVX;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkAccelerationStructureKHR>)    return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkAccelerationStructureNV>)     return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkCudaModuleNV>)                return VK_OBJECT_TYPE_CUDA_MODULE_NV;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkCudaFunctionNV>)              return VK_OBJECT_TYPE_CUDA_FUNCTION_NV;
        // else if constexpr (std::is_same_v<VkResourceTypeT, VkBufferCollectionFuchsia>)  return VK_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkDebugReportCallbackEXT>)      return VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkValidationCacheEXT>)          return VK_OBJECT_TYPE_VALIDATION_CACHE_EXT;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkDescriptorUpdateTemplateKHR>) return VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR;
        else if constexpr (std::is_same_v<VkResourceTypeT, VkSamplerYcbcrConversionKHR>)   return VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_KHR;
        // clang-format on
    }

    inline static VkSemaphoreSubmitInfo CreateSemaphoreSubmitInfo(VkSemaphore semaphore, VkPipelineStageFlags2 stages, uint64_t value = 0)
    {
        VkSemaphoreSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.semaphore = semaphore;
        submitInfo.value = value;
        submitInfo.stageMask = stages;
        return submitInfo;
    }
} // namespace RHI::Vulkan