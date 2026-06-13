#include "Resources.hpp"

#include "Device.hpp"

namespace RHI::WebGPU
{
    ///////////////////////////////////////////////////////////
    // Conversion helpers
    ///////////////////////////////////////////////////////////

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
        WGPUBufferUsage res = WGPUBufferUsage_None;
        if (usage & BufferUsage::Storage)    res |= WGPUBufferUsage_Storage;
        if (usage & BufferUsage::Uniform)    res |= WGPUBufferUsage_Uniform;
        if (usage & BufferUsage::Vertex)     res |= WGPUBufferUsage_Vertex;
        if (usage & BufferUsage::Index)      res |= WGPUBufferUsage_Index;
        if (usage & BufferUsage::CopySrc)    res |= WGPUBufferUsage_CopySrc;
        if (usage & BufferUsage::CopyDst)    res |= WGPUBufferUsage_CopyDst;
        if (usage & BufferUsage::Indirect)   res |= WGPUBufferUsage_Indirect;
        // Host-mapped buffers need MapWrite; also allow them as a copy source for staging.
        if (usage & BufferUsage::HostMapped) res |= WGPUBufferUsage_MapWrite | WGPUBufferUsage_CopySrc;
        return res;
    }

    WGPUTextureDimension ConvertToTextureDimension(ImageType imageType)
    {
        switch (imageType)
        {
        case ImageType::None:
        case ImageType::Image1D: return WGPUTextureDimension_1D;
        case ImageType::Image2D: return WGPUTextureDimension_2D;
        case ImageType::Image3D: return WGPUTextureDimension_3D;
        default:                 return WGPUTextureDimension_Undefined;
        }
    }

    WGPUTextureViewDimension ConvertToTextureViewDimension(ImageViewType imageViewType, bool asArray)
    {
        switch (imageViewType)
        {
        case ImageViewType::View1D:      return WGPUTextureViewDimension_1D;
        case ImageViewType::View1DArray: return WGPUTextureViewDimension_1D;
        case ImageViewType::View2D:      return asArray ? WGPUTextureViewDimension_2DArray : WGPUTextureViewDimension_2D;
        case ImageViewType::View2DArray: return WGPUTextureViewDimension_2DArray;
        case ImageViewType::View3D:      return WGPUTextureViewDimension_3D;
        case ImageViewType::CubeMap:     return WGPUTextureViewDimension_Cube;
        case ImageViewType::None:
        default:                         return WGPUTextureViewDimension_Undefined;
        }
    }

    WGPUExtent2D ConvertToExtent2D(ImageSize2D size)
    {
        return {size.width, size.height};
    }

    WGPUExtent3D ConvertToExtent3D(ImageSize3D size)
    {
        return {size.width, size.height, size.depth};
    }

    WGPUExtent2D ConvertToExtent2D(ImageSize3D size)
    {
        return {size.width, size.height};
    }

    WGPUOrigin2D ConvertToOffset2D(ImageOffset2D offset)
    {
        return {(uint32_t)offset.x, (uint32_t)offset.y};
    }

    WGPUOrigin3D ConvertToOffset3D(ImageOffset3D offset)
    {
        return {(uint32_t)offset.x, (uint32_t)offset.y, (uint32_t)offset.z};
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

    WGPUTextureAspect ConvertToTextureAspect(TL::Flags<ImageAspect> imageAspect, Format format)
    {
        (void)format;
        const bool depth   = imageAspect & ImageAspect::Depth;
        const bool stencil = imageAspect & ImageAspect::Stencil;
        if (depth && !stencil) return WGPUTextureAspect_DepthOnly;
        if (stencil && !depth) return WGPUTextureAspect_StencilOnly;
        return WGPUTextureAspect_All;
    }

    WGPUShaderStage ConvertToShaderStage(TL::Flags<ShaderStage> shaderStageFlags)
    {
        WGPUShaderStage result = WGPUShaderStage_None;
        if (shaderStageFlags & ShaderStage::Vertex) result |= WGPUShaderStage_Vertex;
        if (shaderStageFlags & ShaderStage::Pixel) result |= WGPUShaderStage_Fragment;
        if (shaderStageFlags & ShaderStage::Compute) result |= WGPUShaderStage_Compute;
        return result;
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
        // case IndexType::uint8: return WGPUIndexFormat_Uint8;
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
        default:                                         return WGPUCullMode_Undefined;
        }
    }

    WGPUCompareFunction ConvertToCompareFunction(CompareOperator compareOperator)
    {
        switch (compareOperator)
        {
        case CompareOperator::Undefined:      return WGPUCompareFunction_Never;
        case CompareOperator::Never:          return WGPUCompareFunction_Never;
        case CompareOperator::Equal:          return WGPUCompareFunction_Equal;
        case CompareOperator::NotEqual:       return WGPUCompareFunction_NotEqual;
        case CompareOperator::Greater:        return WGPUCompareFunction_Greater;
        case CompareOperator::GreaterOrEqual: return WGPUCompareFunction_GreaterEqual;
        case CompareOperator::Less:           return WGPUCompareFunction_Less;
        case CompareOperator::LessOrEqual:    return WGPUCompareFunction_LessEqual;
        case CompareOperator::Always:         return WGPUCompareFunction_Always;
        default:                             return WGPUCompareFunction_Undefined;
        }
    }

    WGPUVertexStepMode ConvertToVertexStepMode(PipelineVertexInputRate rate)
    {
        switch (rate)
        {
        case PipelineVertexInputRate::PerInstance: return WGPUVertexStepMode_Instance;
        case PipelineVertexInputRate::PerVertex:   return WGPUVertexStepMode_Vertex;
        default:                                   return WGPUVertexStepMode_Undefined;
        }
    }

    ///////////////////////////////////////////////////////////
    // IFence
    ///////////////////////////////////////////////////////////

    ResultCode IFence::Init(IDevice* device, const FenceCreateInfo& createInfo)
    {
        // WebGPU has no fence object; fences are no-ops for now (single-queue ordering).
        (void)device;
        value = createInfo.initialValue;
        return ResultCode::Success;
    }

    void IFence::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // IBindGroupLayout
    ///////////////////////////////////////////////////////////

    ResultCode IBindGroupLayout::Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IBindGroupLayout::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // IBindGroup
    ///////////////////////////////////////////////////////////

    ResultCode IBindGroup::Init(IDevice* device, const BindGroupCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IBindGroup::Shutdown(IDevice* device)
    {
        (void)device;
    }

    void IBindGroup::Update(IDevice* device, const BindGroupUpdateInfo& updateInfo)
    {
        (void)device;
        (void)updateInfo;
    }

    ///////////////////////////////////////////////////////////
    // IShaderModule
    ///////////////////////////////////////////////////////////

    ResultCode IShaderModule::Init(IDevice* device, const ShaderModuleCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IShaderModule::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // IPipelineLayout
    ///////////////////////////////////////////////////////////

    ResultCode IPipelineLayout::Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IPipelineLayout::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // IGraphicsPipeline
    ///////////////////////////////////////////////////////////

    ResultCode IGraphicsPipeline::Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IGraphicsPipeline::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // IComputePipeline
    ///////////////////////////////////////////////////////////

    ResultCode IComputePipeline::Init(IDevice* device, const ComputePipelineCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IComputePipeline::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // IRayTracingPipeline (unsupported on WebGPU)
    ///////////////////////////////////////////////////////////

    ResultCode IRayTracingPipeline::Init(IDevice* device, const RayTracingPipelineCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IRayTracingPipeline::Shutdown(IDevice* device)
    {
        (void)device;
    }

    void IRayTracingPipeline::GetShaderBindingTableEntry(IDevice* device, uint32_t group, size_t size, void* dstHandle)
    {
        (void)device;
        (void)group;
        (void)size;
        (void)dstHandle;
    }

    ///////////////////////////////////////////////////////////
    // IQueryPool
    ///////////////////////////////////////////////////////////

    ResultCode IQueryPool::Init(IDevice* device, const QueryPoolCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IQueryPool::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // IBuffer
    ///////////////////////////////////////////////////////////

    ResultCode IBuffer::Init(IDevice* device, const BufferCreateInfo& createInfo)
    {
        WGPUBufferDescriptor desc = {};
        desc.label                = createInfo.name;
        desc.usage                = ConvertToBufferUsage(createInfo.usageFlags);
        desc.size                 = createInfo.byteSize;
        // Host-mapped buffers are created already mapped so the initial upload can use Map().
        desc.mappedAtCreation     = (createInfo.usageFlags & BufferUsage::HostMapped) ? 1u : 0u;

        buffer = wgpuDeviceCreateBuffer(device->m_device, &desc);
        return buffer ? ResultCode::Success : ResultCode::ErrorUnknown;
    }

    void IBuffer::Shutdown(IDevice* device)
    {
        (void)device;
        if (buffer)
        {
            wgpuBufferRelease(buffer);
            buffer = nullptr;
        }
    }

    DeviceMemoryPtr IBuffer::Map(IDevice* device)
    {
        (void)device;
        return wgpuBufferGetMappedRange(buffer, 0, WGPU_WHOLE_MAP_SIZE);
    }

    void IBuffer::Unmap(IDevice* device)
    {
        (void)device;
        wgpuBufferUnmap(buffer);
    }

    ///////////////////////////////////////////////////////////
    // IImage
    ///////////////////////////////////////////////////////////

    ResultCode IImage::Init(IDevice* device, const ImageCreateInfo& createInfo)
    {
        const uint32_t layers = (createInfo.type == ImageType::Image3D) ? createInfo.size.depth : createInfo.arrayCount;

        WGPUTextureDescriptor desc = {};
        desc.label                 = createInfo.name;
        desc.usage                 = ConvertToTextureUsage(createInfo.usageFlags);
        desc.dimension             = ConvertToTextureDimension(createInfo.type);
        desc.size                  = {createInfo.size.width, createInfo.size.height, layers};
        desc.format                = ConvertToTextureFormat(createInfo.format);
        desc.mipLevelCount         = createInfo.mipLevels;
        desc.sampleCount           = ConvertToSampleCount(createInfo.sampleCount);

        texture = wgpuDeviceCreateTexture(device->m_device, &desc);
        if (!texture)
            return ResultCode::ErrorUnknown;

        view   = wgpuTextureCreateView(texture, nullptr);
        size   = createInfo.size;
        format = createInfo.format;
        return ResultCode::Success;
    }

    ResultCode IImage::Init(IDevice* device, const ImageViewCreateInfo& createInfo)
    {
        (void)device;
        auto* source = (IImage*)createInfo.image;

        const auto& sub = createInfo.subresource;
        WGPUTextureViewDescriptor desc = {};
        desc.label                     = createInfo.name;
        desc.format                    = ConvertToTextureFormat(createInfo.format != Format::Unknown ? createInfo.format : source->format);
        desc.dimension                 = ConvertToTextureViewDimension(createInfo.viewType, sub.arrayCount > 1);
        desc.baseMipLevel              = sub.mipBase;
        desc.mipLevelCount             = (sub.mipLevelCount == AllMipLevels) ? WGPU_MIP_LEVEL_COUNT_UNDEFINED : sub.mipLevelCount;
        desc.baseArrayLayer            = sub.arrayBase;
        desc.arrayLayerCount           = (sub.arrayCount == AllLayers) ? WGPU_ARRAY_LAYER_COUNT_UNDEFINED : sub.arrayCount;
        desc.aspect                    = ConvertToTextureAspect(sub.imageAspects, source->format);

        // View-only image: it references the source texture but does not own it.
        view    = wgpuTextureCreateView(source->texture, &desc);
        texture = nullptr;
        size    = source->size;
        format  = (createInfo.format != Format::Unknown) ? createInfo.format : source->format;
        return view ? ResultCode::Success : ResultCode::ErrorUnknown;
    }

    ResultCode IImage::Init(IDevice* device, WGPUTexture surfaceTexture, const WGPUSurfaceConfiguration& configuration)
    {
        (void)device;
        texture = surfaceTexture;
        view    = wgpuTextureCreateView(surfaceTexture, nullptr);
        size    = {configuration.width, configuration.height, 1};
        return view ? ResultCode::Success : ResultCode::ErrorUnknown;
    }

    void IImage::Shutdown(IDevice* device)
    {
        (void)device;
        if (view)
        {
            wgpuTextureViewRelease(view);
            view = nullptr;
        }
        if (texture)
        {
            wgpuTextureRelease(texture);
            texture = nullptr;
        }
    }

    ///////////////////////////////////////////////////////////
    // ISampler
    ///////////////////////////////////////////////////////////

    ResultCode ISampler::Init(IDevice* device, const SamplerCreateInfo& createInfo)
    {
        WGPUSamplerDescriptor desc = {};
        desc.label                 = createInfo.name;
        desc.addressModeU          = ConvertToAddressMode(createInfo.addressU);
        desc.addressModeV          = ConvertToAddressMode(createInfo.addressV);
        desc.addressModeW          = ConvertToAddressMode(createInfo.addressW);
        desc.magFilter             = ConvertToSamplerFilter(createInfo.filterMag);
        desc.minFilter             = ConvertToSamplerFilter(createInfo.filterMin);
        desc.mipmapFilter          = ConvertToMipmapFilter(createInfo.filterMip);
        desc.lodMinClamp           = createInfo.minLod;
        desc.lodMaxClamp           = createInfo.maxLod;
        desc.compare               = (createInfo.compare == CompareOperator::Undefined) ? WGPUCompareFunction_Undefined : ConvertToCompareFunction(createInfo.compare);
        desc.maxAnisotropy         = 1;

        sampler = wgpuDeviceCreateSampler(device->m_device, &desc);
        return sampler ? ResultCode::Success : ResultCode::ErrorUnknown;
    }

    void ISampler::Shutdown(IDevice* device)
    {
        (void)device;
        if (sampler)
        {
            wgpuSamplerRelease(sampler);
            sampler = nullptr;
        }
    }

    ///////////////////////////////////////////////////////////
    // IAccelerationStructure (unsupported on WebGPU)
    ///////////////////////////////////////////////////////////

    ResultCode IAccelerationStructure::Init(IDevice* device, const AccelerationStructureCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IAccelerationStructure::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // IMicromap (unsupported on WebGPU)
    ///////////////////////////////////////////////////////////

    ResultCode IMicromap::Init(IDevice* device, const MicromapCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IMicromap::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // ISwapchain
    ///////////////////////////////////////////////////////////

    ISwapchain::ISwapchain()  = default;
    ISwapchain::~ISwapchain() = default;

    ResultCode ISwapchain::Init(IDevice* device, const SwapchainCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void ISwapchain::Shutdown(IDevice* device)
    {
        (void)device;
    }

    uint32_t ISwapchain::GetImagesCount() const
    {
        return 0;
    }

    SwapchainAcquireResult ISwapchain::AcquireSwapchainImage()
    {
        return {};
    }

    SurfaceCapabilities ISwapchain::GetSurfaceCapabilities() const
    {
        return {};
    }

    ResultCode ISwapchain::ResizeSwapchain(const ImageSize2D& size)
    {
        (void)size;
        return ResultCode::Success;
    }

    ResultCode ISwapchain::ConfigureSwapchain(const SwapchainConfigureInfo& configInfo)
    {
        (void)configInfo;
        return ResultCode::Success;
    }

    ResultCode ISwapchain::Present()
    {
        return ResultCode::Success;
    }
} // namespace RHI::WebGPU
