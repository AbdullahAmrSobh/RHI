#pragma once

#include <RHI/RHI.hpp>

#include <vk_mem_alloc.h>

#include <TL/Assert.hpp>

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
        TL_ASSERT(IsSuccess(result));
        return IsSuccess(result);
    }

    [[maybe_unused]] inline static bool Validate(VkResult result)
    {
        TL_ASSERT(result == VK_SUCCESS);
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
            return ResultCode::ErrorUnknown;
        }
    }

    inline static VkFormat ConvertFormat(Format format)
    {
        TL_ASSERT(format < Format::COUNT);
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
        default:                                 TL_UNREACHABLE(); return Format::Unknown;
        }
    }

    inline static VkAttachmentLoadOp ConvertLoadOp(LoadOperation op)
    {
        switch (op)
        {
        case LoadOperation::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        case LoadOperation::Load:     return VK_ATTACHMENT_LOAD_OP_LOAD;
        case LoadOperation::Discard:  return VK_ATTACHMENT_LOAD_OP_CLEAR;
        default:                      TL_UNREACHABLE(); return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
        }
    }

    inline static VkAttachmentStoreOp ConvertStoreOp(StoreOperation op)
    {
        switch (op)
        {
        case StoreOperation::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        case StoreOperation::Store:    return VK_ATTACHMENT_STORE_OP_STORE;
        case StoreOperation::Discard:  return VK_ATTACHMENT_STORE_OP_NONE;
        default:                       TL_UNREACHABLE(); return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
        }
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
        default:                     TL_UNREACHABLE(); return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
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


    inline static VkCommandPoolCreateFlags ConvertCommandPoolFlags(TL::Flags<CommandPoolFlags> flags)
    {
        VkCommandPoolCreateFlags result{};
        if (flags & CommandPoolFlags::Transient)
            result |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        if (flags & CommandPoolFlags::Reset)
            result |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        return result;
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

    template<typename T>
    inline static VkObjectType GetObjectType()
    {
        if constexpr (std::is_same_v<T, VkInstance>) return VK_OBJECT_TYPE_INSTANCE;
        else if constexpr (std::is_same_v<T, VkPhysicalDevice>) return VK_OBJECT_TYPE_PHYSICAL_DEVICE;
        else if constexpr (std::is_same_v<T, VkDevice>) return VK_OBJECT_TYPE_DEVICE;
        else if constexpr (std::is_same_v<T, VkQueue>) return VK_OBJECT_TYPE_QUEUE;
        else if constexpr (std::is_same_v<T, VkSemaphore>) return VK_OBJECT_TYPE_SEMAPHORE;
        else if constexpr (std::is_same_v<T, VkCommandBuffer>) return VK_OBJECT_TYPE_COMMAND_BUFFER;
        else if constexpr (std::is_same_v<T, VkFence>) return VK_OBJECT_TYPE_FENCE;
        else if constexpr (std::is_same_v<T, VkDeviceMemory>) return VK_OBJECT_TYPE_DEVICE_MEMORY;
        else if constexpr (std::is_same_v<T, VkBuffer>) return VK_OBJECT_TYPE_BUFFER;
        else if constexpr (std::is_same_v<T, VkImage>) return VK_OBJECT_TYPE_IMAGE;
        else if constexpr (std::is_same_v<T, VkEvent>) return VK_OBJECT_TYPE_EVENT;
        else if constexpr (std::is_same_v<T, VkQueryPool>) return VK_OBJECT_TYPE_QUERY_POOL;
        else if constexpr (std::is_same_v<T, VkBufferView>) return VK_OBJECT_TYPE_BUFFER_VIEW;
        else if constexpr (std::is_same_v<T, VkImageView>) return VK_OBJECT_TYPE_IMAGE_VIEW;
        else if constexpr (std::is_same_v<T, VkShaderModule>) return VK_OBJECT_TYPE_SHADER_MODULE;
        else if constexpr (std::is_same_v<T, VkPipelineCache>) return VK_OBJECT_TYPE_PIPELINE_CACHE;
        else if constexpr (std::is_same_v<T, VkPipelineLayout>) return VK_OBJECT_TYPE_PIPELINE_LAYOUT;
        else if constexpr (std::is_same_v<T, VkRenderPass>) return VK_OBJECT_TYPE_RENDER_PASS;
        else if constexpr (std::is_same_v<T, VkPipeline>) return VK_OBJECT_TYPE_PIPELINE;
        else if constexpr (std::is_same_v<T, VkDescriptorSetLayout>) return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
        else if constexpr (std::is_same_v<T, VkSampler>) return VK_OBJECT_TYPE_SAMPLER;
        else if constexpr (std::is_same_v<T, VkDescriptorPool>) return VK_OBJECT_TYPE_DESCRIPTOR_POOL;
        else if constexpr (std::is_same_v<T, VkDescriptorSet>) return VK_OBJECT_TYPE_DESCRIPTOR_SET;
        else if constexpr (std::is_same_v<T, VkFramebuffer>) return VK_OBJECT_TYPE_FRAMEBUFFER;
        else if constexpr (std::is_same_v<T, VkCommandPool>) return VK_OBJECT_TYPE_COMMAND_POOL;
        else if constexpr (std::is_same_v<T, VkSurfaceKHR>) return VK_OBJECT_TYPE_SURFACE_KHR;
        else if constexpr (std::is_same_v<T, VkSwapchainKHR>) return VK_OBJECT_TYPE_SWAPCHAIN_KHR;
        else if constexpr (std::is_same_v<T, VkDebugReportCallbackEXT>) return VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT;
        else if constexpr (std::is_same_v<T, VkDisplayKHR>) return VK_OBJECT_TYPE_DISPLAY_KHR;
        else if constexpr (std::is_same_v<T, VkDisplayModeKHR>) return VK_OBJECT_TYPE_DISPLAY_MODE_KHR;
        else if constexpr (std::is_same_v<T, VkValidationCacheEXT>) return VK_OBJECT_TYPE_VALIDATION_CACHE_EXT;
        else if constexpr (std::is_same_v<T, VkSamplerYcbcrConversion>) return VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION;
        else if constexpr (std::is_same_v<T, VkDescriptorUpdateTemplate>) return VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE;
        else if constexpr (std::is_same_v<T, VkCuModuleNVX>) return VK_OBJECT_TYPE_CU_MODULE_NVX;
        else if constexpr (std::is_same_v<T, VkCuFunctionNVX>) return VK_OBJECT_TYPE_CU_FUNCTION_NVX;
        else if constexpr (std::is_same_v<T, VkAccelerationStructureKHR>) return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
        else if constexpr (std::is_same_v<T, VkAccelerationStructureNV>) return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV;
        else if constexpr (std::is_same_v<T, VkCudaModuleNV>) return VK_OBJECT_TYPE_CUDA_MODULE_NV;
        else if constexpr (std::is_same_v<T, VkCudaFunctionNV>) return VK_OBJECT_TYPE_CUDA_FUNCTION_NV;
        else if constexpr (std::is_same_v<T, VkDebugReportCallbackEXT>) return VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT;
        else if constexpr (std::is_same_v<T, VkValidationCacheEXT>) return VK_OBJECT_TYPE_VALIDATION_CACHE_EXT;
        else if constexpr (std::is_same_v<T, VkDescriptorUpdateTemplateKHR>) return VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR;
        else if constexpr (std::is_same_v<T, VkSamplerYcbcrConversionKHR>) return VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_KHR;
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