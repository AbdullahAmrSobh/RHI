#include "Resources.hpp"

#include "Common.hpp"
#include "Device.hpp"

namespace RHI::WebGPU
{
    WGPUTextureFormat ConvertToTextureFormat(Format format)
    {
        switch (format)
        {
        case Format::Unknown:           return WGPUTextureFormat_Undefined;
        case Format::R8_UNORM:          return WGPUTextureFormat_R8Unorm;
        case Format::R8_SNORM:          return WGPUTextureFormat_R8Snorm;
        case Format::R8_UINT:           return WGPUTextureFormat_R8Uint;
        case Format::R8_SINT:           return WGPUTextureFormat_R8Sint;
        case Format::R16_UINT:          return WGPUTextureFormat_R16Uint;
        case Format::R16_SINT:          return WGPUTextureFormat_R16Sint;
        case Format::R16_FLOAT:         return WGPUTextureFormat_R16Float;
        case Format::RG8_UNORM:         return WGPUTextureFormat_RG8Unorm;
        case Format::RG8_SNORM:         return WGPUTextureFormat_RG8Snorm;
        case Format::RG8_UINT:          return WGPUTextureFormat_RG8Uint;
        case Format::RG8_SINT:          return WGPUTextureFormat_RG8Sint;
        case Format::R32_FLOAT:         return WGPUTextureFormat_R32Float;
        case Format::R32_UINT:          return WGPUTextureFormat_R32Uint;
        case Format::R32_SINT:          return WGPUTextureFormat_R32Sint;
        case Format::RG16_UINT:         return WGPUTextureFormat_RG16Uint;
        case Format::RG16_SINT:         return WGPUTextureFormat_RG16Sint;
        case Format::RG16_FLOAT:        return WGPUTextureFormat_RG16Float;
        case Format::RGBA8_UNORM:       return WGPUTextureFormat_RGBA8Unorm;
        // case Format::RGBA8UnormSrgb:              return WGPUTextureFormat_RGBA8UnormSrgb;
        case Format::RGBA8_SNORM:       return WGPUTextureFormat_RGBA8Snorm;
        case Format::RGBA8_UINT:        return WGPUTextureFormat_RGBA8Uint;
        case Format::RGBA8_SINT:        return WGPUTextureFormat_RGBA8Sint;
        case Format::BGRA8_UNORM:       return WGPUTextureFormat_BGRA8Unorm;
        // case Format::BGRA8UnormSrgb:              return WGPUTextureFormat_BGRA8UnormSrgb;
        // case Format::RGB10A2Uint:                 return WGPUTextureFormat_RGB10A2Uint;
        case Format::R10G10B10A2_UNORM: return WGPUTextureFormat_RGB10A2Unorm;
        // case Format::RG11B10Ufloat:               return WGPUTextureFormat_RG11B10Ufloat;
        // case Format::RGB9E5Ufloat:                return WGPUTextureFormat_RGB9E5Ufloat;
        case Format::RG32_FLOAT:        return WGPUTextureFormat_RG32Float;
        case Format::RG32_UINT:         return WGPUTextureFormat_RG32Uint;
        case Format::RG32_SINT:         return WGPUTextureFormat_RG32Sint;
        case Format::RGBA16_UINT:       return WGPUTextureFormat_RGBA16Uint;
        case Format::RGBA16_SINT:       return WGPUTextureFormat_RGBA16Sint;
        case Format::RGBA16_FLOAT:      return WGPUTextureFormat_RGBA16Float;
        case Format::RGBA32_FLOAT:      return WGPUTextureFormat_RGBA32Float;
        case Format::RGBA32_UINT:       return WGPUTextureFormat_RGBA32Uint;
        case Format::RGBA32_SINT:       return WGPUTextureFormat_RGBA32Sint;
        // case Format::Stencil8:                    return WGPUTextureFormat_Stencil8;
        case Format::D16:               return WGPUTextureFormat_Depth16Unorm;
        case Format::D24S8:             return WGPUTextureFormat_Depth24Plus;
        // case Format::D24S8:             return WGPUTextureFormat_Depth24PlusStencil8;
        case Format::D32:               return WGPUTextureFormat_Depth32Float;
        // case Format::D24S8:             return WGPUTextureFormat_Depth32FloatStencil8;
        case Format::BC1_UNORM:         return WGPUTextureFormat_BC1RGBAUnorm;
        case Format::BC1_UNORM_SRGB:    return WGPUTextureFormat_BC1RGBAUnormSrgb;
        case Format::BC2_UNORM:         return WGPUTextureFormat_BC2RGBAUnorm;
        case Format::BC2_UNORM_SRGB:    return WGPUTextureFormat_BC2RGBAUnormSrgb;
        case Format::BC3_UNORM:         return WGPUTextureFormat_BC3RGBAUnorm;
        case Format::BC3_UNORM_SRGB:    return WGPUTextureFormat_BC3RGBAUnormSrgb;
        case Format::BC4_UNORM:         return WGPUTextureFormat_BC4RUnorm;
        case Format::BC4_SNORM:         return WGPUTextureFormat_BC4RSnorm;
        case Format::BC5_UNORM:         return WGPUTextureFormat_BC5RGUnorm;
        case Format::BC5_SNORM:         return WGPUTextureFormat_BC5RGSnorm;
        case Format::BC6H_SFLOAT:       return WGPUTextureFormat_BC6HRGBUfloat;
        case Format::BC6H_UFLOAT:       return WGPUTextureFormat_BC6HRGBFloat;
        case Format::BC7_UNORM:         return WGPUTextureFormat_BC7RGBAUnorm;
        case Format::BC7_UNORM_SRGB:    return WGPUTextureFormat_BC7RGBAUnormSrgb;
        // case Format::ETC2RGB8Unorm:               return WGPUTextureFormat_ETC2RGB8Unorm;
        // case Format::ETC2RGB8UnormSrgb:           return WGPUTextureFormat_ETC2RGB8UnormSrgb;
        // case Format::ETC2RGB8A1Unorm:             return WGPUTextureFormat_ETC2RGB8A1Unorm;
        // case Format::ETC2RGB8A1UnormSrgb:         return WGPUTextureFormat_ETC2RGB8A1UnormSrgb;
        // case Format::ETC2RGBA8Unorm:              return WGPUTextureFormat_ETC2RGBA8Unorm;
        // case Format::ETC2RGBA8UnormSrgb:          return WGPUTextureFormat_ETC2RGBA8UnormSrgb;
        // case Format::EACR11Unorm:                 return WGPUTextureFormat_EACR11Unorm;
        // case Format::EACR11Snorm:                 return WGPUTextureFormat_EACR11Snorm;
        // case Format::EACRG11Unorm:                return WGPUTextureFormat_EACRG11Unorm;
        // case Format::EACRG11Snorm:                return WGPUTextureFormat_EACRG11Snorm;
        // case Format::ASTC4x4Unorm:                return WGPUTextureFormat_ASTC4x4Unorm;
        // case Format::ASTC4x4UnormSrgb:            return WGPUTextureFormat_ASTC4x4UnormSrgb;
        // case Format::ASTC5x4Unorm:                return WGPUTextureFormat_ASTC5x4Unorm;
        // case Format::ASTC5x4UnormSrgb:            return WGPUTextureFormat_ASTC5x4UnormSrgb;
        // case Format::ASTC5x5Unorm:                return WGPUTextureFormat_ASTC5x5Unorm;
        // case Format::ASTC5x5UnormSrgb:            return WGPUTextureFormat_ASTC5x5UnormSrgb;
        // case Format::ASTC6x5Unorm:                return WGPUTextureFormat_ASTC6x5Unorm;
        // case Format::ASTC6x5UnormSrgb:            return WGPUTextureFormat_ASTC6x5UnormSrgb;
        // case Format::ASTC6x6Unorm:                return WGPUTextureFormat_ASTC6x6Unorm;
        // case Format::ASTC6x6UnormSrgb:            return WGPUTextureFormat_ASTC6x6UnormSrgb;
        // case Format::ASTC8x5Unorm:                return WGPUTextureFormat_ASTC8x5Unorm;
        // case Format::ASTC8x5UnormSrgb:            return WGPUTextureFormat_ASTC8x5UnormSrgb;
        // case Format::ASTC8x6Unorm:                return WGPUTextureFormat_ASTC8x6Unorm;
        // case Format::ASTC8x6UnormSrgb:            return WGPUTextureFormat_ASTC8x6UnormSrgb;
        // case Format::ASTC8x8Unorm:                return WGPUTextureFormat_ASTC8x8Unorm;
        // case Format::ASTC8x8UnormSrgb:            return WGPUTextureFormat_ASTC8x8UnormSrgb;
        // case Format::ASTC10x5Unorm:               return WGPUTextureFormat_ASTC10x5Unorm;
        // case Format::ASTC10x5UnormSrgb:           return WGPUTextureFormat_ASTC10x5UnormSrgb;
        // case Format::ASTC10x6Unorm:               return WGPUTextureFormat_ASTC10x6Unorm;
        // case Format::ASTC10x6UnormSrgb:           return WGPUTextureFormat_ASTC10x6UnormSrgb;
        // case Format::ASTC10x8Unorm:               return WGPUTextureFormat_ASTC10x8Unorm;
        // case Format::ASTC10x8UnormSrgb:           return WGPUTextureFormat_ASTC10x8UnormSrgb;
        // case Format::ASTC10x10Unorm:              return WGPUTextureFormat_ASTC10x10Unorm;
        // case Format::ASTC10x10UnormSrgb:          return WGPUTextureFormat_ASTC10x10UnormSrgb;
        // case Format::ASTC12x10Unorm:              return WGPUTextureFormat_ASTC12x10Unorm;
        // case Format::ASTC12x10UnormSrgb:          return WGPUTextureFormat_ASTC12x10UnormSrgb;
        // case Format::ASTC12x12Unorm:              return WGPUTextureFormat_ASTC12x12Unorm;
        // case Format::ASTC12x12UnormSrgb:          return WGPUTextureFormat_ASTC12x12UnormSrgb;
        case Format::R16_UNORM:         return WGPUTextureFormat_R16Unorm;
        case Format::RG16_UNORM:        return WGPUTextureFormat_RG16Unorm;
        case Format::RGBA16_UNORM:      return WGPUTextureFormat_RGBA16Unorm;
        case Format::R16_SNORM:         return WGPUTextureFormat_R16Snorm;
        case Format::RG16_SNORM:        return WGPUTextureFormat_RG16Snorm;
        case Format::RGBA16_SNORM:
            return WGPUTextureFormat_RGBA16Snorm;
            // case Format::R8BG8Biplanar420Unorm:       return WGPUTextureFormat_R8BG8Biplanar420Unorm;
            // case Format::R10X6BG10X6Biplanar420Unorm: return WGPUTextureFormat_R10X6BG10X6Biplanar420Unorm;
            // case Format::R8BG8A8Triplanar420Unorm:    return WGPUTextureFormat_R8BG8A8Triplanar420Unorm;
            // case Format::R8BG8Biplanar422Unorm:       return WGPUTextureFormat_R8BG8Biplanar422Unorm;
            // case Format::R8BG8Biplanar444Unorm:       return WGPUTextureFormat_R8BG8Biplanar444Unorm;
            // case Format::R10X6BG10X6Biplanar422Unorm: return WGPUTextureFormat_R10X6BG10X6Biplanar422Unorm;
            // case Format::R10X6BG10X6Biplanar444Unorm: return WGPUTextureFormat_R10X6BG10X6Biplanar444Unorm;
            // case Format::External:                    return WGPUTextureFormat_External;
            // case Format::Force32:                     return WGPUTextureFormat_Force32;
        }
        TL_UNREACHABLE();
        return WGPUTextureFormat_Undefined;
    }

    WGPUVertexFormat ConvertToVertexFormat(Format format)
    {
        switch (format)
        {
        case Format::R8_UINT:           return WGPUVertexFormat_Uint8;
        case Format::RG8_UINT:          return WGPUVertexFormat_Uint8x2;
        case Format::RGBA8_UINT:        return WGPUVertexFormat_Uint8x4;
        case Format::R8_SINT:           return WGPUVertexFormat_Sint8;
        case Format::RG8_SINT:          return WGPUVertexFormat_Sint8x2;
        case Format::RGBA8_SINT:        return WGPUVertexFormat_Sint8x4;
        case Format::R8_UNORM:          return WGPUVertexFormat_Unorm8;
        case Format::RG8_UNORM:         return WGPUVertexFormat_Unorm8x2;
        case Format::RGBA8_UNORM:       return WGPUVertexFormat_Unorm8x4;
        case Format::R8_SNORM:          return WGPUVertexFormat_Snorm8;
        case Format::RG8_SNORM:         return WGPUVertexFormat_Snorm8x2;
        case Format::RGBA8_SNORM:       return WGPUVertexFormat_Snorm8x4;
        case Format::R16_UINT:          return WGPUVertexFormat_Uint16;
        case Format::RG16_UINT:         return WGPUVertexFormat_Uint16x2;
        case Format::RGBA16_UINT:       return WGPUVertexFormat_Uint16x4;
        case Format::R16_SINT:          return WGPUVertexFormat_Sint16;
        case Format::RG16_SINT:         return WGPUVertexFormat_Sint16x2;
        case Format::RGBA16_SINT:       return WGPUVertexFormat_Sint16x4;
        case Format::R16_UNORM:         return WGPUVertexFormat_Unorm16;
        case Format::RG16_UNORM:        return WGPUVertexFormat_Unorm16x2;
        case Format::RGBA16_UNORM:      return WGPUVertexFormat_Unorm16x4;
        case Format::R16_SNORM:         return WGPUVertexFormat_Snorm16;
        case Format::RG16_SNORM:        return WGPUVertexFormat_Snorm16x2;
        case Format::RGBA16_SNORM:      return WGPUVertexFormat_Snorm16x4;
        case Format::R16_FLOAT:         return WGPUVertexFormat_Float16;
        case Format::RG16_FLOAT:        return WGPUVertexFormat_Float16x2;
        case Format::RGBA16_FLOAT:      return WGPUVertexFormat_Float16x4;
        case Format::R32_FLOAT:         return WGPUVertexFormat_Float32;
        case Format::RG32_FLOAT:        return WGPUVertexFormat_Float32x2;
        case Format::RGB32_FLOAT:       return WGPUVertexFormat_Float32x3;
        case Format::RGBA32_FLOAT:      return WGPUVertexFormat_Float32x4;
        case Format::R32_UINT:          return WGPUVertexFormat_Uint32;
        case Format::RG32_UINT:         return WGPUVertexFormat_Uint32x2;
        case Format::RGB32_UINT:        return WGPUVertexFormat_Uint32x3;
        case Format::RGBA32_UINT:       return WGPUVertexFormat_Uint32x4;
        case Format::R32_SINT:          return WGPUVertexFormat_Sint32;
        case Format::RG32_SINT:         return WGPUVertexFormat_Sint32x2;
        case Format::RGB32_SINT:        return WGPUVertexFormat_Sint32x3;
        case Format::RGBA32_SINT:       return WGPUVertexFormat_Sint32x4;
        case Format::R10G10B10A2_UNORM: return WGPUVertexFormat_Unorm10_10_10_2;
        // case Format::: return WGPUVertexFormat_Unorm8x4BGRA;
        default:                        TL_UNREACHABLE(); return WGPUVertexFormat_Force32;
        }
    }

    WGPUFilterMode ConvertToSamplerFilter(SamplerFilter filter)
    {
        switch (filter)
        {
        case SamplerFilter::Point:  return WGPUFilterMode_Nearest;
        case SamplerFilter::Linear: return WGPUFilterMode_Linear;
        }
        return WGPUFilterMode_Undefined;
    }

    uint32_t ConvertToSampleCount(SampleCount count)
    {
        return (uint32_t)count;
    }

    WGPUTextureUsage ConvertToTextureUsage(TL::Flags<ImageUsage> usage)
    {
        WGPUTextureUsage res{};
        if (usage & ImageUsage::ShaderResource) res |= WGPUTextureUsage_TextureBinding;
        if (usage & ImageUsage::StorageResource) res |= WGPUTextureUsage_StorageBinding;
        if (usage & ImageUsage::Color) res |= WGPUTextureUsage_RenderAttachment;
        if (usage & ImageUsage::Depth) res |= WGPUTextureUsage_RenderAttachment;
        if (usage & ImageUsage::Stencil) res |= WGPUTextureUsage_RenderAttachment;
        if (usage & ImageUsage::DepthStencil) res |= WGPUTextureUsage_RenderAttachment;
        if (usage & ImageUsage::CopySrc) res |= WGPUTextureUsage_CopySrc;
        if (usage & ImageUsage::CopyDst) res |= WGPUTextureUsage_CopyDst;
        if (usage & ImageUsage::Resolve) res |= WGPUTextureUsage_CopyDst;
        return res;
    }

    WGPUBufferUsage ConvertToBufferUsage(TL::Flags<BufferUsage> usage)
    {
        WGPUBufferUsage res{};
        if (usage & BufferUsage::Storage) return WGPUBufferUsage_Storage;
        if (usage & BufferUsage::Uniform) return WGPUBufferUsage_Uniform;
        if (usage & BufferUsage::Vertex) return WGPUBufferUsage_Vertex;
        if (usage & BufferUsage::Index) return WGPUBufferUsage_Index;
        if (usage & BufferUsage::CopySrc) return WGPUBufferUsage_CopySrc;
        if (usage & BufferUsage::CopyDst) return WGPUBufferUsage_CopyDst;
        if (usage & BufferUsage::Indirect) return WGPUBufferUsage_Indirect;
        return res;
    }

    WGPUTextureDimension ConvertToTextureDimension(ImageType type)
    {
        switch (type)
        {
        case ImageType::None:
        case ImageType::Image1D: return WGPUTextureDimension_1D;
        case ImageType::Image2D: return WGPUTextureDimension_2D;
        case ImageType::Image3D: return WGPUTextureDimension_3D;
        }
        return WGPUTextureDimension_Undefined;
    }

    WGPUTextureViewDimension ConvertToTextureViewDimension(ImageType type, bool asArray)
    {
        switch (type)
        {
        case ImageType::None:
        case ImageType::Image1D: return WGPUTextureViewDimension_1D;
        case ImageType::Image2D: return asArray ? WGPUTextureViewDimension_2DArray : WGPUTextureViewDimension_2D;
        case ImageType::Image3D: return WGPUTextureViewDimension_3D;
        }
        return WGPUTextureViewDimension_Undefined;
    }

    WGPUExtent2D ConvertToExtent2D(ImageSize2D extent)
    {
        return {extent.width, extent.height};
    }

    WGPUExtent3D ConvertToExtent3D(ImageSize3D extent)
    {
        return {extent.width, extent.height, extent.depth};
    }

    WGPUOrigin2D ConvertToOffset2D(ImageOffset2D extent)
    {
        return {(uint32_t)extent.x, (uint32_t)extent.y};
    }

    WGPUOrigin3D ConvertToOffset3D(ImageOffset3D extent)
    {
        return {(uint32_t)extent.x, (uint32_t)extent.y, (uint32_t)extent.z};
    }

    WGPUAddressMode ConvertToAddressMode(SamplerAddressMode mode)
    {
        switch (mode)
        {
        case SamplerAddressMode::Repeat: return WGPUAddressMode_Repeat;
        case SamplerAddressMode::Clamp:  return WGPUAddressMode_ClampToEdge;
        }
        return WGPUAddressMode_Undefined;
    }

    WGPUMipmapFilterMode ConvertToMipmapFilter(SamplerFilter filter)
    {
        switch (filter)
        {
        case SamplerFilter::Point:  return WGPUMipmapFilterMode_Nearest;
        case SamplerFilter::Linear: return WGPUMipmapFilterMode_Linear;
        }
        return WGPUMipmapFilterMode_Undefined;
    }

    WGPUTextureAspect ConvertToTextureAspect(TL::Flags<ImageAspect> type)
    {
        if (type == ImageAspect::Color) return WGPUTextureAspect_All;
        if (type == ImageAspect::Depth) return WGPUTextureAspect_DepthOnly;
        if (type == ImageAspect::Stencil) return WGPUTextureAspect_StencilOnly;
        return WGPUTextureAspect_All;
    }

    WGPUPrimitiveTopology ConvertToPrimitiveTopology(PipelineTopologyMode topology)
    {
        switch (topology)
        {
        case PipelineTopologyMode::Points:    return WGPUPrimitiveTopology_PointList;
        case PipelineTopologyMode::Lines:     return WGPUPrimitiveTopology_LineList;
        case PipelineTopologyMode::Triangles: return WGPUPrimitiveTopology_TriangleList;
        }
        return WGPUPrimitiveTopology_Undefined;
    }

    WGPUIndexFormat ConvertToIndexFormat(IndexType indexType)
    {
        switch (indexType)
        {
        case IndexType::uint16: return WGPUIndexFormat_Uint16;
        case IndexType::uint32: return WGPUIndexFormat_Uint32;
        }
        return WGPUIndexFormat_Undefined;
    }

    WGPUFrontFace ConvertToFrontFace(PipelineRasterizerStateFrontFace frontFace)
    {
        switch (frontFace)
        {
        case PipelineRasterizerStateFrontFace::Clockwise:        return WGPUFrontFace_CW;
        case PipelineRasterizerStateFrontFace::CounterClockwise: return WGPUFrontFace_CCW;
        }
        return WGPUFrontFace_Undefined;
    }

    WGPUCullMode ConvertToCullMode(PipelineRasterizerStateCullMode cullMode)
    {
        switch (cullMode)
        {
        case PipelineRasterizerStateCullMode::None:      return WGPUCullMode_None;
        case PipelineRasterizerStateCullMode::FrontFace: return WGPUCullMode_Front;
        case PipelineRasterizerStateCullMode::BackFace:  return WGPUCullMode_Back;
        case PipelineRasterizerStateCullMode::Discard:   return WGPUCullMode_Force32;
        }
        return WGPUCullMode_Undefined;
    }

    WGPUCompareFunction ConvertToCompareFunction(CompareOperator compareOp)
    {
        switch (compareOp)
        {
        case CompareOperator::Never:          return WGPUCompareFunction_Never;
        case CompareOperator::Equal:          return WGPUCompareFunction_Equal;
        case CompareOperator::NotEqual:       return WGPUCompareFunction_NotEqual;
        case CompareOperator::Greater:        return WGPUCompareFunction_Greater;
        case CompareOperator::GreaterOrEqual: return WGPUCompareFunction_GreaterEqual;
        case CompareOperator::Less:           return WGPUCompareFunction_Less;
        case CompareOperator::LessOrEqual:    return WGPUCompareFunction_LessEqual;
        case CompareOperator::Always:         return WGPUCompareFunction_Always;
        }
        return WGPUCompareFunction_Undefined;
    }

    WGPUVertexStepMode ConvertToVertexStepMode(PipelineVertexInputRate rate)
    {
        switch (rate)
        {
        case PipelineVertexInputRate::None:        return WGPUVertexStepMode_Undefined;
        case PipelineVertexInputRate::PerInstance: return WGPUVertexStepMode_Instance;
        case PipelineVertexInputRate::PerVertex:   return WGPUVertexStepMode_Vertex;
        }
        return WGPUVertexStepMode_Undefined;
    }

    WGPUShaderStage ConvertToShaderStage(TL::Flags<ShaderStage> stage)
    {
        WGPUShaderStage res = WGPUShaderStage_None;
        if (stage & ShaderStage::Vertex)
            res |= WGPUShaderStage_Vertex;
        if (stage & ShaderStage::Pixel)
            res |= WGPUShaderStage_Fragment;
        if (stage & ShaderStage::Compute)
            res |= WGPUShaderStage_Compute;
        return res;
    }

    ResultCode IBindGroupLayout::Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo)
    {
        TL::Vector<WGPUBindGroupLayoutEntry> entries;
        uint32_t                             bindingIndex = 0;
        for (const auto& binding : createInfo.bindings)
        {
            WGPUBindGroupLayoutEntry entry{
                .nextInChain    = nullptr,
                .binding        = bindingIndex++,
                .visibility     = ConvertToShaderStage(binding.stages),
                .buffer         = {},
                .sampler        = {},
                .texture        = {},
                .storageTexture = {},
            };

            switch (binding.type)
            {
            case BindingType::None:
            case BindingType::SampledImage:
                entry.texture = {
                    .nextInChain   = nullptr,
                    .sampleType    = WGPUTextureSampleType_Float, // TODO! expose from outside!
                    .viewDimension = WGPUTextureViewDimension_2D, // TODO! expose from outside!
                    .multisampled  = false,
                };
                break;
            case BindingType::Sampler:
                entry.sampler = {
                    .nextInChain = nullptr,
                    .type        = WGPUSamplerBindingType_Filtering,
                };
                break;
            case BindingType::StorageImage:
                entry.storageTexture = {
                    .nextInChain   = nullptr,
                    .access        = ConvertStorageTextureAccess(binding.access),
                    .format        = {},
                    .viewDimension = ConvertToTextureViewDimension(ImageType::Image2D, binding.arrayCount > 1),
                };
                break;
            case BindingType::UniformBuffer:
                entry.buffer = {
                    .nextInChain      = nullptr,
                    .type             = WGPUBufferBindingType_Uniform,
                    .hasDynamicOffset = false,
                    .minBindingSize   = {},
                };
                break;
            case BindingType::StorageBuffer:
                entry.buffer = {
                    .nextInChain      = nullptr,
                    .type             = WGPUBufferBindingType_Storage,
                    .hasDynamicOffset = {},
                    .minBindingSize   = {},
                };
                break;
            case BindingType::DynamicUniformBuffer:
                entry.buffer = {
                    .nextInChain      = nullptr,
                    .type             = WGPUBufferBindingType_Uniform,
                    .hasDynamicOffset = true,
                    .minBindingSize   = {},
                };
                break;
            case BindingType::DynamicStorageBuffer:
                entry.buffer = {
                    .nextInChain      = nullptr,
                    .type             = WGPUBufferBindingType_Storage,
                    .hasDynamicOffset = true,
                    .minBindingSize   = {},
                };
                break;
            case BindingType::BufferView:
            case BindingType::StorageBufferView:
            default:
                TL_UNREACHABLE();
                break;
            }
            entries.push_back(entry);
        }
        WGPUBindGroupLayoutDescriptor desc{
            .nextInChain = nullptr,
            .label       = ConvertToStringView(createInfo.name),
            .entryCount  = entries.size(),
            .entries     = entries.data(),
        };
        this->bindGroupLayout = wgpuDeviceCreateBindGroupLayout(device->m_device, &desc);
        return ResultCode::Success;
    }

    void IBindGroupLayout::Shutdown([[maybe_unused]] IDevice* device)
    {
        wgpuBindGroupLayoutRelease(this->bindGroupLayout);
    }

    ResultCode IBindGroup::Init(IDevice* device, const BindGroupCreateInfo& createInfo)
    {
        bindGroup = {};
        layout    = device->m_bindGroupLayoutsOwner.Get(createInfo.layout)->bindGroupLayout;
        wgpuBindGroupLayoutAddRef(layout);
        return ResultCode::Success;
    }

    void IBindGroup::Shutdown([[maybe_unused]] IDevice* device)
    {
        wgpuBindGroupRelease(this->bindGroup);
        wgpuBindGroupLayoutRelease(this->layout);
    }

    void IBindGroup::Update(IDevice* device, const BindGroupUpdateInfo& updateInfo)
    {
        wgpuBindGroupRelease(bindGroup);

        TL::Vector<WGPUBindGroupEntry> entry{};

        for (auto bufferBindings : updateInfo.buffers)
        {
            uint32_t bindingCounter = 0;
            for (uint32_t i = 0; i < bufferBindings.buffers.size(); i++)
            {
                auto bufferHandle    = bufferBindings.buffers[i];
                auto bufferSubregion = bufferBindings.subregions[i];

                auto buffer = device->m_bufferOwner.Get(bufferHandle);
                entry.push_back({
                    .binding = bufferBindings.dstBinding + (bindingCounter++),
                    .buffer  = buffer->buffer,
                    .offset  = bufferSubregion.offset,
                    .size    = bufferSubregion.size,
                });
            }
        }
        for (auto imageBindings : updateInfo.images)
        {
            uint32_t bindingCounter = 0;
            for (auto imageHandle : imageBindings.images)
            {
                auto image = device->m_imageOwner.Get(imageHandle);
                entry.push_back({
                    .binding     = imageBindings.dstBinding + (bindingCounter++),
                    .textureView = image->view,
                });
            }
        }
        for (auto samplerBindings : updateInfo.samplers)
        {
            uint32_t bindingCounter = 0;
            for (auto samplerHandle : samplerBindings.samplers)
            {
                auto sampler = device->m_samplerOwner.Get(samplerHandle);
                entry.push_back({
                    .binding = samplerBindings.dstArrayElement + (bindingCounter++),
                    .sampler = sampler->sampler,
                });
            }
        }

        if (bindGroup)
            wgpuBindGroupRelease(bindGroup);

        WGPUBindGroupDescriptor desc{
            .nextInChain = nullptr,
            .label       = ConvertToStringView("BindGroup"),
            .layout      = layout,
            .entryCount  = entry.size(),
            .entries     = entry.data(),
        };
        bindGroup = wgpuDeviceCreateBindGroup(device->m_device, &desc);
    }

    ResultCode IShaderModule::Init(IDevice* device, const ShaderModuleCreateInfo& createInfo)
    {
        WGPUShaderSourceSPIRV spirvSourceDesc{
            .chain    = {.next = {}, .sType = WGPUSType_ShaderSourceSPIRV},
            .codeSize = (uint32_t)createInfo.code.size(),
            .code     = createInfo.code.data(),
        };
        WGPUShaderModuleDescriptor desc{
            .nextInChain = &spirvSourceDesc.chain,
            .label       = ConvertToStringView(createInfo.name),
        };
        this->module = wgpuDeviceCreateShaderModule(device->m_device, &desc);
        return ResultCode::Success;
    }

    void IShaderModule::Shutdown()
    {
        wgpuShaderModuleRelease(module);
    }

    ResultCode IPipelineLayout::Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo)
    {
        TL::Vector<WGPUBindGroupLayout> layouts;
        for (size_t i = 0; i < createInfo.layouts.size(); ++i)
        {
            auto bindGroupLayout = device->m_bindGroupLayoutsOwner.Get(createInfo.layouts[i]);
            layouts.push_back(bindGroupLayout->bindGroupLayout);
        }

        WGPUPipelineLayoutDescriptor desc{
            .nextInChain                = nullptr,
            .label                      = ConvertToStringView(createInfo.name),
            .bindGroupLayoutCount       = layouts.size(),
            .bindGroupLayouts           = layouts.data(),
            .immediateDataRangeByteSize = {},
        };
        this->layout = wgpuDeviceCreatePipelineLayout(device->m_device, &desc);
        return ResultCode::Success;
    }

    void IPipelineLayout::Shutdown([[maybe_unused]] IDevice* device)
    {
        wgpuPipelineLayoutRelease(this->layout);
    }

    ResultCode IGraphicsPipeline::Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo)
    {
        auto pipelineLayout = device->m_pipelineLayoutOwner.Get(createInfo.layout);
        auto vs             = (IShaderModule*)createInfo.vertexShaderModule;
        auto ps             = (IShaderModule*)createInfo.pixelShaderModule;

        layout = pipelineLayout->layout;
        wgpuPipelineLayoutAddRef(layout);

        TL::Vector<WGPUVertexAttribute>    attributes{};
        TL::Vector<WGPUVertexBufferLayout> vertexBufferLayouts{};

        uint32_t bindingIndex = 0;
        for (const auto& bindingDesc : createInfo.vertexBufferBindings)
        {
            // Iterate through vertex attributes for this binding
            for (const auto& attributeDesc : bindingDesc.attributes)
            {
                attributes.push_back({
                    .nextInChain    = nullptr,
                    .format         = ConvertToVertexFormat(attributeDesc.format),
                    .offset         = attributeDesc.offset,
                    .shaderLocation = bindingIndex++,
                });
            }

            vertexBufferLayouts.push_back({
                .nextInChain    = nullptr,
                .arrayStride    = bindingDesc.stride,
                .stepMode       = ConvertToVertexStepMode(bindingDesc.stepRate),
                .attributeCount = attributes.size(),
                .attributes     = attributes.data(),
            });
        }

        WGPUDepthStencilState depthStencil{
            .nextInChain         = nullptr,
            .format              = ConvertToTextureFormat(createInfo.renderTargetLayout.depthAttachmentFormat),
            .depthWriteEnabled   = createInfo.depthStencilState.depthWriteEnable ? WGPUOptionalBool_True : WGPUOptionalBool_False,
            .depthCompare        = ConvertToCompareFunction(createInfo.depthStencilState.compareOperator),
            .stencilFront        = {},
            .stencilBack         = {},
            .stencilReadMask     = {},
            .stencilWriteMask    = {},
            .depthBias           = {},
            .depthBiasSlopeScale = {},
            .depthBiasClamp      = {},
        };

        TL::Vector<WGPUColorTargetState> colorTargets;
        for (uint32_t i = 0; i < createInfo.renderTargetLayout.colorAttachmentsFormats.size(); ++i)
        {
            WGPUBlendState blend{};
            auto           format = ConvertToTextureFormat(createInfo.renderTargetLayout.colorAttachmentsFormats[i]);
            colorTargets.push_back({
                .nextInChain = nullptr,
                .format      = format,
                .blend       = &blend,
                .writeMask   = WGPUColorWriteMask_All,
            });
        }

        WGPUFragmentState fragment{
            .nextInChain   = nullptr,
            .module        = ps->module,
            .entryPoint    = ConvertToStringView(createInfo.pixelShaderName),
            .constantCount = {},
            .constants     = {},
            .targetCount   = colorTargets.size(),
            .targets       = colorTargets.data(),
        };

        // TODO: reivew the hardcoded values later
        WGPURenderPipelineDescriptor desc{
            .nextInChain = nullptr,
            .label       = ConvertToStringView(createInfo.name),
            .layout      = pipelineLayout->layout,
            .vertex      = {
                            .nextInChain   = nullptr,
                            .module        = vs->module,
                            .entryPoint    = ConvertToStringView(createInfo.vertexShaderName),
                            .constantCount = {},
                            .constants     = {},
                            .bufferCount   = vertexBufferLayouts.size(),
                            .buffers       = vertexBufferLayouts.data(),
                            },
            .primitive = {
                            .nextInChain      = nullptr,
                            .topology         = ConvertToPrimitiveTopology(createInfo.topologyMode),
                            .stripIndexFormat = WGPUIndexFormat_Undefined,
                            .frontFace        = ConvertToFrontFace(createInfo.rasterizationState.frontFace),
                            .cullMode         = ConvertToCullMode(createInfo.rasterizationState.cullMode),
                            .unclippedDepth   = false,
                            },
            .depthStencil = &depthStencil,
            .multisample  = {
                            .nextInChain            = nullptr,
                            .count                  = ConvertToSampleCount(createInfo.multisampleState.sampleCount),
                            .mask                   = 0xFFFFFFFF,
                            .alphaToCoverageEnabled = false,
                            },
            .fragment = &fragment,
        };
        this->pipeline = wgpuDeviceCreateRenderPipeline(device->m_device, &desc);
        return ResultCode::Success;
    }

    void IGraphicsPipeline::Shutdown([[maybe_unused]] IDevice* device)
    {
        wgpuRenderPipelineRelease(this->pipeline);
        wgpuPipelineLayoutRelease(this->layout);
    }

    ResultCode IComputePipeline::Init(IDevice* device, const ComputePipelineCreateInfo& createInfo)
    {
        auto pipelineLayout = device->m_pipelineLayoutOwner.Get(createInfo.layout);
        auto cs             = (IShaderModule*)createInfo.shaderModule;

        layout = pipelineLayout->layout;
        wgpuPipelineLayoutAddRef(layout);

        WGPUComputePipelineDescriptor desc{
            .nextInChain = nullptr,
            .label       = ConvertToStringView(createInfo.name),
            .layout      = pipelineLayout->layout,
            .compute     = {
                            .nextInChain   = nullptr,
                            .module        = cs->module,
                            .entryPoint    = ConvertToStringView(createInfo.name),
                            .constantCount = {},
                            .constants     = {},
                            }
        };
        this->pipeline = wgpuDeviceCreateComputePipeline(device->m_device, &desc);
        return ResultCode::Success;
    }

    void IComputePipeline::Shutdown([[maybe_unused]] IDevice* device)
    {
        wgpuComputePipelineRelease(this->pipeline);
        wgpuPipelineLayoutRelease(this->layout);
    }

    ResultCode IBuffer::Init(IDevice* device, const BufferCreateInfo& createInfo)
    {
        WGPUBufferDescriptor desc{
            .nextInChain      = nullptr,
            .label            = ConvertToStringView(createInfo.name),
            .usage            = ConvertToBufferUsage(createInfo.usageFlags),
            .size             = createInfo.byteSize,
            .mappedAtCreation = createInfo.hostMapped ? true : false,
        };
        this->buffer = wgpuDeviceCreateBuffer(device->m_device, &desc);
        return ResultCode::Success;
    }

    void IBuffer::Shutdown([[maybe_unused]] IDevice* device)
    {
        wgpuBufferRelease(this->buffer);
    }

    ResultCode IImage::Init(IDevice* device, const ImageCreateInfo& createInfo)
    {
        auto                  format = ConvertToTextureFormat(createInfo.format);
        WGPUTextureDescriptor desc{
            .nextInChain     = nullptr,
            .label           = ConvertToStringView(createInfo.name),
            .usage           = ConvertToTextureUsage(createInfo.usageFlags),
            .dimension       = ConvertToTextureDimension(createInfo.type),
            .size            = ConvertToExtent3D(createInfo.size),
            .format          = format,
            .mipLevelCount   = createInfo.mipLevels,
            .sampleCount     = ConvertToSampleCount(createInfo.sampleCount),
            .viewFormatCount = 1,
            .viewFormats     = &format,
        };
        WGPUTextureViewDescriptor viewDesc{
            .nextInChain     = nullptr,
            .label           = ConvertToStringView(createInfo.name),
            .format          = format,
            .dimension       = ConvertToTextureViewDimension(createInfo.type, createInfo.arrayCount),
            .baseMipLevel    = 0,
            .mipLevelCount   = createInfo.mipLevels,
            .baseArrayLayer  = 0,
            .arrayLayerCount = createInfo.arrayCount,
            .aspect          = WGPUTextureAspect_All, // TODO:
            .usage           = ConvertToTextureUsage(createInfo.usageFlags),
        };
        this->texture = wgpuDeviceCreateTexture(device->m_device, &desc);
        this->view    = wgpuTextureCreateView(this->texture, &viewDesc);
        return ResultCode::Success;
    }

    ResultCode IImage::Init([[maybe_unused]] IDevice* device, WGPUTexture surfaceTexture, WGPUSurfaceConfiguration desc)
    {
        this->texture = surfaceTexture;
        WGPUTextureViewDescriptor viewDesc{
            .nextInChain     = nullptr,
            .label           = {},
            .format          = desc.format,
            .dimension       = WGPUTextureViewDimension_2D,
            .baseMipLevel    = 0,
            .mipLevelCount   = 1,
            .baseArrayLayer  = 0,
            .arrayLayerCount = 1,
            .aspect          = WGPUTextureAspect_All,
            .usage           = desc.usage,
        };
        this->view = wgpuTextureCreateView(this->texture, &viewDesc);
        return ResultCode::Success;
    }

    void IImage::Shutdown([[maybe_unused]] IDevice* device)
    {
        if (view) wgpuTextureViewRelease(view);
        if (texture) wgpuTextureRelease(texture);
    }

    ResultCode ISampler::Init(IDevice* device, const SamplerCreateInfo& createInfo)
    {
        WGPUSamplerDescriptor desc{
            .nextInChain   = nullptr,
            .label         = ConvertToStringView(createInfo.name),
            .addressModeU  = ConvertToAddressMode(createInfo.addressU),
            .addressModeV  = ConvertToAddressMode(createInfo.addressV),
            .addressModeW  = ConvertToAddressMode(createInfo.addressW),
            .magFilter     = ConvertToSamplerFilter(createInfo.filterMag),
            .minFilter     = ConvertToSamplerFilter(createInfo.filterMin),
            .mipmapFilter  = ConvertToMipmapFilter(createInfo.filterMip),
            .lodMinClamp   = createInfo.minLod,
            .lodMaxClamp   = createInfo.maxLod,
            .compare       = ConvertToCompareFunction(createInfo.compare),
            .maxAnisotropy = 0, // TODO: implement anisotropy
        };
        this->sampler = wgpuDeviceCreateSampler(device->m_device, &desc);
        return ResultCode::Success;
    }

    void ISampler::Shutdown([[maybe_unused]] IDevice* device)
    {
        wgpuSamplerRelease(sampler);
    }

} // namespace RHI::WebGPU
