#pragma once

#include <RHI/RHI.h>

#include <TL/Assert.hpp>

#include <vk_mem_alloc.h>

namespace RHI::Vulkan
{
    struct VulkanResult
    {
        VkResult result;

        VulkanResult()
            : result(VK_SUCCESS)
        {
        }

        VulkanResult(VkResult result)
            : result(result)
        {
        }

        const char* AsString() const
        {
            switch (result)
            {
            case VK_SUCCESS:                                            return "VK_SUCCESS";
            case VK_NOT_READY:                                          return "VK_NOT_READY";
            case VK_TIMEOUT:                                            return "VK_TIMEOUT";
            case VK_EVENT_SET:                                          return "VK_EVENT_SET";
            case VK_EVENT_RESET:                                        return "VK_EVENT_RESET";
            case VK_INCOMPLETE:                                         return "VK_INCOMPLETE";
            case VK_ERROR_OUT_OF_HOST_MEMORY:                           return "VK_ERROR_OUT_OF_HOST_MEMORY";
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:                         return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
            case VK_ERROR_INITIALIZATION_FAILED:                        return "VK_ERROR_INITIALIZATION_FAILED";
            case VK_ERROR_DEVICE_LOST:                                  return "VK_ERROR_DEVICE_LOST";
            case VK_ERROR_MEMORY_MAP_FAILED:                            return "VK_ERROR_MEMORY_MAP_FAILED";
            case VK_ERROR_LAYER_NOT_PRESENT:                            return "VK_ERROR_LAYER_NOT_PRESENT";
            case VK_ERROR_EXTENSION_NOT_PRESENT:                        return "VK_ERROR_EXTENSION_NOT_PRESENT";
            case VK_ERROR_FEATURE_NOT_PRESENT:                          return "VK_ERROR_FEATURE_NOT_PRESENT";
            case VK_ERROR_INCOMPATIBLE_DRIVER:                          return "VK_ERROR_INCOMPATIBLE_DRIVER";
            case VK_ERROR_TOO_MANY_OBJECTS:                             return "VK_ERROR_TOO_MANY_OBJECTS";
            case VK_ERROR_FORMAT_NOT_SUPPORTED:                         return "VK_ERROR_FORMAT_NOT_SUPPORTED";
            case VK_ERROR_FRAGMENTED_POOL:                              return "VK_ERROR_FRAGMENTED_POOL";
            case VK_ERROR_UNKNOWN:                                      return "VK_ERROR_UNKNOWN";
            case VK_ERROR_OUT_OF_POOL_MEMORY:                           return "VK_ERROR_OUT_OF_POOL_MEMORY";
            case VK_ERROR_INVALID_EXTERNAL_HANDLE:                      return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
            case VK_ERROR_FRAGMENTATION:                                return "VK_ERROR_FRAGMENTATION";
            case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:               return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
            case VK_PIPELINE_COMPILE_REQUIRED:                          return "VK_PIPELINE_COMPILE_REQUIRED";
            case VK_ERROR_SURFACE_LOST_KHR:                             return "VK_ERROR_SURFACE_LOST_KHR";
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:                     return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
            case VK_SUBOPTIMAL_KHR:                                     return "VK_SUBOPTIMAL_KHR";
            case VK_ERROR_OUT_OF_DATE_KHR:                              return "VK_ERROR_OUT_OF_DATE_KHR";
            case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:                     return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
            case VK_ERROR_VALIDATION_FAILED_EXT:                        return "VK_ERROR_VALIDATION_FAILED_EXT";
            case VK_ERROR_INVALID_SHADER_NV:                            return "VK_ERROR_INVALID_SHADER_NV";
            case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:                return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:       return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:    return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:       return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:        return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:          return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
            case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
            case VK_ERROR_NOT_PERMITTED_KHR:                            return "VK_ERROR_NOT_PERMITTED_KHR";
            case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:          return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
            case VK_THREAD_IDLE_KHR:                                    return "VK_THREAD_IDLE_KHR";
            case VK_THREAD_DONE_KHR:                                    return "VK_THREAD_DONE_KHR";
            case VK_OPERATION_DEFERRED_KHR:                             return "VK_OPERATION_DEFERRED_KHR";
            case VK_OPERATION_NOT_DEFERRED_KHR:                         return "VK_OPERATION_NOT_DEFERRED_KHR";
            case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:             return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
            case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:                    return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
            case VK_INCOMPATIBLE_SHADER_BINARY_EXT:                     return "VK_INCOMPATIBLE_SHADER_BINARY_EXT";
            case VK_PIPELINE_BINARY_MISSING_KHR:                        return "VK_PIPELINE_BINARY_MISSING_KHR";
            case VK_ERROR_NOT_ENOUGH_SPACE_KHR:                         return "VK_ERROR_NOT_ENOUGH_SPACE_KHR";
            case VK_RESULT_MAX_ENUM:                                    return "VK_RESULT_MAX_ENUM";
            default:                                                    TL_UNREACHABLE(); return "Unknown";
            }
        }

        operator const char*() const
        {
            return AsString();
        }

        bool IsSuccess() const
        {
            return result == VK_SUCCESS;
        }

        bool IsError() const
        {
            return !IsSuccess();
        }

        bool IsSwapchainSuccess() const
        {
            return result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR;
        }

        operator VkResult() const
        {
            return result;
        }

        operator ResultCode() const
        {
            switch (result)
            {
            case VK_SUCCESS:                                            return ResultCode::Success;
            case VK_NOT_READY:                                          return ResultCode::ErrorUnknown;
            case VK_TIMEOUT:                                            return ResultCode::ErrorTimeout;
            case VK_EVENT_SET:                                          return ResultCode::ErrorUnknown;
            case VK_EVENT_RESET:                                        return ResultCode::ErrorUnknown;
            case VK_INCOMPLETE:                                         return ResultCode::ErrorUnknown;
            case VK_ERROR_OUT_OF_HOST_MEMORY:                           return ResultCode::ErrorOutOfMemory;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:                         return ResultCode::ErrorDeviceOutOfMemory;
            case VK_ERROR_INITIALIZATION_FAILED:                        return ResultCode::ErrorInvalidArguments;
            case VK_ERROR_DEVICE_LOST:                                  return ResultCode::ErrorDeviceLost;
            case VK_ERROR_MEMORY_MAP_FAILED:                            return ResultCode::ErrorUnknown;
            case VK_ERROR_LAYER_NOT_PRESENT:                            return ResultCode::ErrorUnknown;
            case VK_ERROR_EXTENSION_NOT_PRESENT:                        return ResultCode::ErrorInvalidArguments;
            case VK_ERROR_FEATURE_NOT_PRESENT:                          return ResultCode::ErrorInvalidArguments;
            case VK_ERROR_INCOMPATIBLE_DRIVER:                          return ResultCode::ErrorUnknown;
            case VK_ERROR_TOO_MANY_OBJECTS:                             return ResultCode::ErrorUnknown;
            case VK_ERROR_FORMAT_NOT_SUPPORTED:                         return ResultCode::ErrorUnknown;
            case VK_ERROR_FRAGMENTED_POOL:                              return ResultCode::ErrorUnknown;
            case VK_ERROR_UNKNOWN:                                      return ResultCode::ErrorUnknown;
            case VK_ERROR_OUT_OF_POOL_MEMORY:                           return ResultCode::ErrorPoolOutOfMemory;
            case VK_ERROR_INVALID_EXTERNAL_HANDLE:                      return ResultCode::ErrorUnknown;
            case VK_ERROR_FRAGMENTATION:                                return ResultCode::ErrorUnknown;
            case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:               return ResultCode::ErrorUnknown;
            case VK_PIPELINE_COMPILE_REQUIRED:                          return ResultCode::ErrorUnknown;
            case VK_ERROR_SURFACE_LOST_KHR:                             return ResultCode::ErrorSurfaceLost;
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:                     return ResultCode::ErrorUnknown;
            case VK_SUBOPTIMAL_KHR:                                     return ResultCode::SuccessSuboptimal;
            case VK_ERROR_OUT_OF_DATE_KHR:                              return ResultCode::ErrorOutdated;
            case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:                     return ResultCode::ErrorUnknown;
            case VK_ERROR_VALIDATION_FAILED_EXT:                        return ResultCode::ErrorUnknown;
            case VK_ERROR_INVALID_SHADER_NV:                            return ResultCode::ErrorUnknown;
            case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:                return ResultCode::ErrorUnknown;
            case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:       return ResultCode::ErrorUnknown;
            case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:    return ResultCode::ErrorUnknown;
            case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:       return ResultCode::ErrorUnknown;
            case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:        return ResultCode::ErrorUnknown;
            case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:          return ResultCode::ErrorUnknown;
            case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return ResultCode::ErrorUnknown;
            case VK_ERROR_NOT_PERMITTED_KHR:                            return ResultCode::ErrorUnknown;
            case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:          return ResultCode::ErrorUnknown;
            case VK_THREAD_IDLE_KHR:                                    return ResultCode::ErrorUnknown;
            case VK_THREAD_DONE_KHR:                                    return ResultCode::ErrorUnknown;
            case VK_OPERATION_DEFERRED_KHR:                             return ResultCode::ErrorUnknown;
            case VK_OPERATION_NOT_DEFERRED_KHR:                         return ResultCode::ErrorUnknown;
            case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:             return ResultCode::ErrorUnknown;
            case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:                    return ResultCode::ErrorUnknown;
            case VK_INCOMPATIBLE_SHADER_BINARY_EXT:                     return ResultCode::ErrorUnknown;
            case VK_PIPELINE_BINARY_MISSING_KHR:                        return ResultCode::ErrorUnknown;
            case VK_ERROR_NOT_ENOUGH_SPACE_KHR:                         return ResultCode::ErrorUnknown;
            case VK_RESULT_MAX_ENUM:                                    return ResultCode::ErrorUnknown;
            default:                                                    return ResultCode::ErrorUnknown;
            }
        }

        operator bool() const
        {
            return IsSuccess();
        }

        bool operator==(VkResult other) const
        {
            return result == other;
        }
    };

#define VkResultTry(x, ...)    \
    if (!x)                    \
    {                          \
        Shutdown(__VA_ARGS__); \
        return x;              \
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

    inline static VkIndexType convertIndexType(IndexType indexType)
    {
        switch (indexType)
        {
        case IndexType::uint8:  return VK_INDEX_TYPE_UINT8;
        case IndexType::uint16: return VK_INDEX_TYPE_UINT16;
        case IndexType::uint32: return VK_INDEX_TYPE_UINT32;
        }
        TL_UNREACHABLE();
        return VK_INDEX_TYPE_MAX_ENUM;
    }

    inline static VkAccelerationStructureGeometryKHR
    convertGeometryData(const AccelerationStructureGeometry& trianglesData)
    {
        switch (trianglesData.geometryType)
        {
        case GeometryType::Aabbs:
            return VkAccelerationStructureGeometryKHR{
                .sType        = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                .pNext        = nullptr,
                .geometryType = VK_GEOMETRY_TYPE_AABBS_KHR,
                .geometry     = {
                        .aabbs = {
                            .sType  = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR,
                            .pNext  = nullptr,
                            .data   = {.deviceAddress = trianglesData.data},
                            .stride = trianglesData.stride,
                    },
                },
                .flags = {},
            };
        case GeometryType::Triangles:
            return VkAccelerationStructureGeometryKHR{
                .sType        = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                .pNext        = nullptr,
                .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
                .geometry     = {
                        .triangles = {
                            .sType         = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
                            .pNext         = nullptr,
                            .vertexFormat  = ConvertFormat(trianglesData.vertexFormat),
                            .vertexData    = {.deviceAddress = trianglesData.data},
                            .vertexStride  = trianglesData.stride,
                            .maxVertex     = trianglesData.count,
                            .indexType     = convertIndexType(trianglesData.indexType),
                            .indexData     = {.deviceAddress = trianglesData.indexData},
                            .transformData = {.deviceAddress = trianglesData.transformData},
                    },
                },
                .flags = {},
            };
        }
        TL_UNREACHABLE();
        return {};
    }

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
        else if constexpr (std::is_same_v<T, VkDescriptorUpdateTemplateKHR>) return VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR;
        else if constexpr (std::is_same_v<T, VkSamplerYcbcrConversionKHR>) return VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_KHR;
        else
        {
            return VK_OBJECT_TYPE_UNKNOWN;
        }
    }

    inline static VkPipelineStageFlags2 ConvertPipelineStageFlags(TL::Flags<PipelineStage> pipelineStages)
    {
        VkPipelineStageFlags2 stageFlags = {};
        if (pipelineStages & PipelineStage::TopOfPipe) stageFlags |= VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        if (pipelineStages & PipelineStage::DrawIndirect) stageFlags |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        if (pipelineStages & PipelineStage::VertexInput) stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
        if (pipelineStages & PipelineStage::VertexShader) stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        if (pipelineStages & PipelineStage::TessellationControlShader) stageFlags |= VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT;
        if (pipelineStages & PipelineStage::TessellationEvaluationShader) stageFlags |= VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT;
        if (pipelineStages & PipelineStage::PixelShader) stageFlags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        if (pipelineStages & PipelineStage::EarlyFragmentTests) stageFlags |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
        if (pipelineStages & PipelineStage::LateFragmentTests) stageFlags |= VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        if (pipelineStages & PipelineStage::ColorAttachmentOutput) stageFlags |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        if (pipelineStages & PipelineStage::ComputeShader) stageFlags |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        if (pipelineStages & PipelineStage::Transfer) stageFlags |= VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        if (pipelineStages & PipelineStage::BottomOfPipe) stageFlags |= VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        if (pipelineStages & PipelineStage::Host) stageFlags |= VK_PIPELINE_STAGE_2_HOST_BIT;
        if (pipelineStages & PipelineStage::AllGraphics) stageFlags |= VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
        if (pipelineStages & PipelineStage::AllCommands) stageFlags |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        if (pipelineStages & PipelineStage::Copy) stageFlags |= VK_PIPELINE_STAGE_2_COPY_BIT;
        if (pipelineStages & PipelineStage::Resolve) stageFlags |= VK_PIPELINE_STAGE_2_RESOLVE_BIT;
        if (pipelineStages & PipelineStage::Blit) stageFlags |= VK_PIPELINE_STAGE_2_BLIT_BIT;
        if (pipelineStages & PipelineStage::Clear) stageFlags |= VK_PIPELINE_STAGE_2_CLEAR_BIT;
        if (pipelineStages & PipelineStage::IndexInput) stageFlags |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
        if (pipelineStages & PipelineStage::VertexAttributeInput) stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
        if (pipelineStages & PipelineStage::PreRasterizationShaders) stageFlags |= VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT;
        if (pipelineStages & PipelineStage::TransformFeedback) stageFlags |= VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT;
        if (pipelineStages & PipelineStage::ConditionalRendering) stageFlags |= VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT;
        if (pipelineStages & PipelineStage::FragmentShadingRateAttachment) stageFlags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
        if (pipelineStages & PipelineStage::AccelerationStructureBuild) stageFlags |= VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
        if (pipelineStages & PipelineStage::RayTracingShader) stageFlags |= VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
        if (pipelineStages & PipelineStage::TaskShader) stageFlags |= VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT;
        if (pipelineStages & PipelineStage::MeshShader) stageFlags |= VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;
        if (pipelineStages & PipelineStage::AccelerationStructureCopy) stageFlags |= VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR;
        return stageFlags;
    }

} // namespace RHI::Vulkan