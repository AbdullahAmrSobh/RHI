#pragma once

#include <RHI/RHI.hpp>
#include <slang/slang.h>

namespace BGC
{
    inline static SlangStage ConvertShaderStage(RHI::ShaderStage stage)
    {
        switch (stage)
        {
        case RHI::ShaderStage::None:
        case RHI::ShaderStage::Vertex:  return SLANG_STAGE_VERTEX;
        case RHI::ShaderStage::Pixel:   return SLANG_STAGE_PIXEL;
        case RHI::ShaderStage::Compute: return SLANG_STAGE_COMPUTE;
        default:
            TL_UNREACHABLE();
        }
    }

    inline static RHI::ShaderStage ConvertShaderStage(SlangStage stage)
    {
        switch (stage)
        {
        case SLANG_STAGE_VERTEX:         return RHI::ShaderStage::Vertex;
        case SLANG_STAGE_HULL:           return RHI::ShaderStage::Hull;
        case SLANG_STAGE_DOMAIN:         return RHI::ShaderStage::Domain;
        case SLANG_STAGE_FRAGMENT:       return RHI::ShaderStage::Pixel;
        case SLANG_STAGE_COMPUTE:        return RHI::ShaderStage::Compute;
        case SLANG_STAGE_RAY_GENERATION: return RHI::ShaderStage::RayGen;
        case SLANG_STAGE_INTERSECTION:   return RHI::ShaderStage::RayIntersect;
        case SLANG_STAGE_ANY_HIT:        return RHI::ShaderStage::RayAnyHit;
        case SLANG_STAGE_CLOSEST_HIT:    return RHI::ShaderStage::RayClosestHit;
        case SLANG_STAGE_MISS:           return RHI::ShaderStage::RayMiss;
        case SLANG_STAGE_CALLABLE:       return RHI::ShaderStage::RayCallable;
        case SLANG_STAGE_MESH:           return RHI::ShaderStage::Mesh;
        case SLANG_STAGE_AMPLIFICATION:  return RHI::ShaderStage::Amplification;
        default:                         TL_UNREACHABLE();
        }
    }

    inline static TL::Flags<RHI::ShaderStage> ConvertShaderStageFlags(SlangStage stage)
    {
        switch (stage)
        {
        case SLANG_STAGE_VERTEX:         return RHI::ShaderStage::Vertex;
        case SLANG_STAGE_HULL:           return RHI::ShaderStage::Hull;
        case SLANG_STAGE_DOMAIN:         return RHI::ShaderStage::Domain;
        case SLANG_STAGE_FRAGMENT:       return RHI::ShaderStage::Pixel;
        case SLANG_STAGE_COMPUTE:        return RHI::ShaderStage::Compute;
        case SLANG_STAGE_RAY_GENERATION: return RHI::ShaderStage::RayGen;
        case SLANG_STAGE_ANY_HIT:        return RHI::ShaderStage::RayAnyHit;
        case SLANG_STAGE_CLOSEST_HIT:    return RHI::ShaderStage::RayClosestHit;
        case SLANG_STAGE_MISS:           return RHI::ShaderStage::RayMiss;
        case SLANG_STAGE_INTERSECTION:   return RHI::ShaderStage::RayIntersect;
        case SLANG_STAGE_CALLABLE:       return RHI::ShaderStage::RayCallable;
        case SLANG_STAGE_MESH:           return RHI::ShaderStage::Mesh;
        case SLANG_STAGE_AMPLIFICATION:  return RHI::ShaderStage::Amplification;
        default:                         return RHI::ShaderStage::AllStages;
        }
    }

    inline static const char* ToString(slang::BindingType bindingType)
    {
        switch (bindingType)
        {
        case slang::BindingType::Unknown:                         return "BindingType::Unknown";
        case slang::BindingType::Sampler:                         return "BindingType::Sampler";
        case slang::BindingType::Texture:                         return "BindingType::Texture";
        case slang::BindingType::ConstantBuffer:                  return "BindingType::ConstantBuffer";
        case slang::BindingType::ParameterBlock:                  return "BindingType::ParameterBlock";
        case slang::BindingType::TypedBuffer:                     return "BindingType::TypedBuffer";
        case slang::BindingType::RawBuffer:                       return "BindingType::RawBuffer";
        case slang::BindingType::CombinedTextureSampler:          return "BindingType::CombinedTextureSampler";
        case slang::BindingType::InputRenderTarget:               return "BindingType::InputRenderTarget";
        case slang::BindingType::InlineUniformData:               return "BindingType::InlineUniformData";
        case slang::BindingType::RayTracingAccelerationStructure: return "BindingType::RayTracingAccelerationStructure";
        case slang::BindingType::VaryingInput:                    return "BindingType::VaryingInput";
        case slang::BindingType::VaryingOutput:                   return "BindingType::VaryingOutput";
        case slang::BindingType::ExistentialValue:                return "BindingType::ExistentialValue";
        case slang::BindingType::PushConstant:                    return "BindingType::PushConstant";
        case slang::BindingType::MutableFlag:                     return "BindingType::MutableFlag";
        case slang::BindingType::MutableTexture:                  return "BindingType::MutableTexture";
        case slang::BindingType::MutableTypedBuffer:              return "BindingType::MutableTypedBuffer";
        case slang::BindingType::MutableRawBuffer:                return "BindingType::MutableRawBuffer";
        case slang::BindingType::BaseMask:                        return "BindingType::BaseMask";
        case slang::BindingType::ExtMask:                         return "BindingType::ExtMask";
        }
    }

    inline static RHI::BindingType ConvertShaderBindingType(slang::BindingType bindingType)
    {
        switch (bindingType)
        {
        case slang::BindingType::Sampler:                         return RHI::BindingType::Sampler;
        case slang::BindingType::Texture:                         return RHI::BindingType::SampledImage;
        case slang::BindingType::MutableTexture:                  return RHI::BindingType::StorageImage;
        case slang::BindingType::TypedBuffer:                     return RHI::BindingType::BufferView;
        case slang::BindingType::MutableTypedBuffer:              return RHI::BindingType::StorageBufferView;
        case slang::BindingType::ConstantBuffer:                  return RHI::BindingType::UniformBuffer;
        case slang::BindingType::RawBuffer:                       return RHI::BindingType::StorageBuffer;
        case slang::BindingType::MutableRawBuffer:                return RHI::BindingType::StorageBuffer;
        case slang::BindingType::InputRenderTarget:               return RHI::BindingType::InputAttachment;
        case slang::BindingType::RayTracingAccelerationStructure: return RHI::BindingType::RayTracingAccelerationStructure;
        default:                                                  /*TL_UNREACHABLE();*/ return RHI::BindingType::None;
        }
    }

    inline static RHI::Access ConvertAccess(SlangResourceAccess access)
    {
        switch (access)
        {
        case SLANG_RESOURCE_ACCESS_NONE:       return RHI::Access::None;
        case SLANG_RESOURCE_ACCESS_READ:       return RHI::Access::Read;
        case SLANG_RESOURCE_ACCESS_READ_WRITE: return RHI::Access::ReadWrite;
        // case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
        case SLANG_RESOURCE_ACCESS_APPEND:     return RHI::Access::Write;
        // case SLANG_RESOURCE_ACCESS_CONSUME: return RHI::Access::Read;
        case SLANG_RESOURCE_ACCESS_WRITE:
            return RHI::Access::Write;
            // case SLANG_RESOURCE_ACCESS_FEEDBACK:
            // case SLANG_RESOURCE_ACCESS_UNKNOWN:        break;
        default: TL_UNREACHABLE(); return RHI::Access::None;
        }
    }

} // namespace BGC