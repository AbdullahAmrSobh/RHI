#pragma once

#include <RHI/Resources.hpp>

#include <TL/String.hpp>

namespace RHI::Debug
{
    inline static TL::String ToString(TL::Flags<RHI::Access> access)
    {
        if (access == RHI::Access::ReadWrite) return "ReadWrite";
        else if (access == RHI::Access::Read) return "Read";
        else if (access == RHI::Access::Write) return "Write";
        else return "None";
    }

    inline static TL::String ToString(TL::Flags<RHI::ShaderStage> stages)
    {
        TL::String result;
        bool       first = true;

        auto       append = [&](const char* name)
        {
            if (!first) result += " | ";
            result += name;
            first = false;
        };

        if (stages == RHI::ShaderStage::None)
            return "ShaderStage::None";
        else if ((stages & RHI::ShaderStage::AllStages) == RHI::ShaderStage::AllStages)
            append("ShaderStage::AllStages");
        else if ((stages & RHI::ShaderStage::AllGraphics) == RHI::ShaderStage::AllGraphics)
            append("ShaderStage::AllGraphics");
        else
        {
            if (stages & RHI::ShaderStage::Vertex) append("ShaderStage::Vertex");
            if (stages & RHI::ShaderStage::Pixel) append("ShaderStage::Pixel");
            if (stages & RHI::ShaderStage::Compute) append("ShaderStage::Compute");
            if (stages & RHI::ShaderStage::Hull) append("ShaderStage::Hull");
            if (stages & RHI::ShaderStage::Domain) append("ShaderStage::Domain");
            if (stages & RHI::ShaderStage::RayGen) append("ShaderStage::RayGen");
            if (stages & RHI::ShaderStage::RayIntersect) append("ShaderStage::RayIntersect");
            if (stages & RHI::ShaderStage::RayAnyHit) append("ShaderStage::RayAnyHit");
            if (stages & RHI::ShaderStage::RayClosestHit) append("ShaderStage::RayClosestHit");
            if (stages & RHI::ShaderStage::RayMiss) append("ShaderStage::RayMiss");
            if (stages & RHI::ShaderStage::RayCallable) append("ShaderStage::RayCallable");
            if (stages & RHI::ShaderStage::Mesh) append("ShaderStage::Mesh");
            if (stages & RHI::ShaderStage::Amplification) append("ShaderStage::Amplification");
        }

        return result;
    }

    inline static const char* ToString(Format format)
    {
        switch (format)
        {
        case Format::Unknown:           return "Format::Unknown";
        case Format::R8_UINT:           return "Format::R8_UINT";
        case Format::R8_SINT:           return "Format::R8_SINT";
        case Format::R8_UNORM:          return "Format::R8_UNORM";
        case Format::R8_SNORM:          return "Format::R8_SNORM";
        case Format::RG8_UINT:          return "Format::RG8_UINT";
        case Format::RG8_SINT:          return "Format::RG8_SINT";
        case Format::RG8_UNORM:         return "Format::RG8_UNORM";
        case Format::RG8_SNORM:         return "Format::RG8_SNORM";
        case Format::R16_UINT:          return "Format::R16_UINT";
        case Format::R16_SINT:          return "Format::R16_SINT";
        case Format::R16_UNORM:         return "Format::R16_UNORM";
        case Format::R16_SNORM:         return "Format::R16_SNORM";
        case Format::R16_FLOAT:         return "Format::R16_FLOAT";
        case Format::BGRA4_UNORM:       return "Format::BGRA4_UNORM";
        case Format::B5G6R5_UNORM:      return "Format::B5G6R5_UNORM";
        case Format::B5G5R5A1_UNORM:    return "Format::B5G5R5A1_UNORM";
        case Format::RGBA8_UINT:        return "Format::RGBA8_UINT";
        case Format::RGBA8_SINT:        return "Format::RGBA8_SINT";
        case Format::RGBA8_UNORM:       return "Format::RGBA8_UNORM";
        case Format::RGBA8_SNORM:       return "Format::RGBA8_SNORM";
        case Format::BGRA8_UNORM:       return "Format::BGRA8_UNORM";
        case Format::SRGBA8_UNORM:      return "Format::SRGBA8_UNORM";
        case Format::SBGRA8_UNORM:      return "Format::SBGRA8_UNORM";
        case Format::R10G10B10A2_UNORM: return "Format::R10G10B10A2_UNORM";
        case Format::R11G11B10_FLOAT:   return "Format::R11G11B10_FLOAT";
        case Format::RG16_UINT:         return "Format::RG16_UINT";
        case Format::RG16_SINT:         return "Format::RG16_SINT";
        case Format::RG16_UNORM:        return "Format::RG16_UNORM";
        case Format::RG16_SNORM:        return "Format::RG16_SNORM";
        case Format::RG16_FLOAT:        return "Format::RG16_FLOAT";
        case Format::R32_UINT:          return "Format::R32_UINT";
        case Format::R32_SINT:          return "Format::R32_SINT";
        case Format::R32_FLOAT:         return "Format::R32_FLOAT";
        case Format::RGBA16_UINT:       return "Format::RGBA16_UINT";
        case Format::RGBA16_SINT:       return "Format::RGBA16_SINT";
        case Format::RGBA16_FLOAT:      return "Format::RGBA16_FLOAT";
        case Format::RGBA16_UNORM:      return "Format::RGBA16_UNORM";
        case Format::RGBA16_SNORM:      return "Format::RGBA16_SNORM";
        case Format::RG32_UINT:         return "Format::RG32_UINT";
        case Format::RG32_SINT:         return "Format::RG32_SINT";
        case Format::RG32_FLOAT:        return "Format::RG32_FLOAT";
        case Format::RGB32_UINT:        return "Format::RGB32_UINT";
        case Format::RGB32_SINT:        return "Format::RGB32_SINT";
        case Format::RGB32_FLOAT:       return "Format::RGB32_FLOAT";
        case Format::RGBA32_UINT:       return "Format::RGBA32_UINT";
        case Format::RGBA32_SINT:       return "Format::RGBA32_SINT";
        case Format::RGBA32_FLOAT:      return "Format::RGBA32_FLOAT";
        case Format::D16:               return "Format::D16";
        case Format::D24S8:             return "Format::D24S8";
        case Format::X24G8_UINT:        return "Format::X24G8_UINT";
        case Format::D32:               return "Format::D32";
        case Format::D32S8:             return "Format::D32S8";
        case Format::X32G8_UINT:        return "Format::X32G8_UINT";
        case Format::BC1_UNORM:         return "Format::BC1_UNORM";
        case Format::BC1_UNORM_SRGB:    return "Format::BC1_UNORM_SRGB";
        case Format::BC2_UNORM:         return "Format::BC2_UNORM";
        case Format::BC2_UNORM_SRGB:    return "Format::BC2_UNORM_SRGB";
        case Format::BC3_UNORM:         return "Format::BC3_UNORM";
        case Format::BC3_UNORM_SRGB:    return "Format::BC3_UNORM_SRGB";
        case Format::BC4_UNORM:         return "Format::BC4_UNORM";
        case Format::BC4_SNORM:         return "Format::BC4_SNORM";
        case Format::BC5_UNORM:         return "Format::BC5_UNORM";
        case Format::BC5_SNORM:         return "Format::BC5_SNORM";
        case Format::BC6H_UFLOAT:       return "Format::BC6H_UFLOAT";
        case Format::BC6H_SFLOAT:       return "Format::BC6H_SFLOAT";
        case Format::BC7_UNORM:         return "Format::BC7_UNORM";
        case Format::BC7_UNORM_SRGB:    return "Format::BC7_UNORM_SRGB";
        case Format::COUNT:             return "Format::COUNT";
        }
    }

    inline static const char* ToString(BindingType e)
    {
        switch (e)
        {
        case BindingType::None:                 return "BindingType::None";
        case BindingType::Sampler:              return "BindingType::Sampler";
        case BindingType::SampledImage:         return "BindingType::SampledImage";
        case BindingType::StorageImage:         return "BindingType::StorageImage";
        case BindingType::UniformBuffer:        return "BindingType::UniformBuffer";
        case BindingType::StorageBuffer:        return "BindingType::StorageBuffer";
        case BindingType::DynamicUniformBuffer: return "BindingType::DynamicUniformBuffer";
        case BindingType::DynamicStorageBuffer: return "BindingType::DynamicStorageBuffer";
        case BindingType::BufferView:           return "BindingType::BufferView";
        case BindingType::StorageBufferView:    return "BindingType::StorageBufferView";
        // case BindingType::Count:                return "BindingType::Count";
        default:                                return "BindingType::Count";
        }
    }

    inline static const char* ToString(ShaderStage e)
    {
        switch (e)
        {
        case ShaderStage::None:    return "ShaderStage::None";
        case ShaderStage::Vertex:  return "ShaderStage::Vertex";
        case ShaderStage::Pixel:   return "ShaderStage::Pixel";
        case ShaderStage::Compute: return "ShaderStage::Compute";
        }
    }

    inline static const char* ToString(BufferUsage e)
    {
        switch (e)
        {
        case BufferUsage::None:     return "BufferUsage::None";
        case BufferUsage::Storage:  return "BufferUsage::Storage";
        case BufferUsage::Uniform:  return "BufferUsage::Uniform";
        case BufferUsage::Vertex:   return "BufferUsage::Vertex";
        case BufferUsage::Index:    return "BufferUsage::Index";
        case BufferUsage::CopySrc:  return "BufferUsage::CopySrc";
        case BufferUsage::CopyDst:  return "BufferUsage::CopyDst";
        case BufferUsage::Indirect: return "BufferUsage::Indirect";
        }
    }

    inline static TL::String ToString(TL::Flags<BufferUsage> f)
    {
        TL::String result;

        if (f == BufferUsage::None)
            return "BufferUsage::None";

        bool first = true;
        if (f & BufferUsage::Storage)
        {
            if (!first) result += " | ";
            result += "BufferUsage::Storage";
            first = false;
        }
        if (f & BufferUsage::Uniform)
        {
            if (!first) result += " | ";
            result += "BufferUsage::Uniform";
            first = false;
        }
        if (f & BufferUsage::Vertex)
        {
            if (!first) result += " | ";
            result += "BufferUsage::Vertex";
            first = false;
        }
        if (f & BufferUsage::Index)
        {
            if (!first) result += " | ";
            result += "BufferUsage::Index";
            first = false;
        }
        if (f & BufferUsage::CopySrc)
        {
            if (!first) result += " | ";
            result += "BufferUsage::CopySrc";
            first = false;
        }
        if (f & BufferUsage::CopyDst)
        {
            if (!first) result += " | ";
            result += "BufferUsage::CopyDst";
            first = false;
        }
        if (f & BufferUsage::Indirect)
        {
            if (!first) result += " | ";
            result += "BufferUsage::Indirect";
            first = false;
        }

        return result;
    }

    inline static const char* ToString(IndexType e)
    {
        switch (e)
        {
        case IndexType::uint16: return "IndexType::uint16";
        case IndexType::uint32: return "IndexType::uint32";
        }
    }

    inline static const char* ToString(ImageUsage e)
    {
        switch (e)
        {
        case ImageUsage::None:            return "ImageUsage::None";
        case ImageUsage::ShaderResource:  return "ImageUsage::ShaderResource";
        case ImageUsage::StorageResource: return "ImageUsage::StorageResource";
        case ImageUsage::Color:           return "ImageUsage::Color";
        case ImageUsage::Depth:           return "ImageUsage::Depth";
        case ImageUsage::Stencil:         return "ImageUsage::Stencil";
        case ImageUsage::DepthStencil:    return "ImageUsage::DepthStencil";
        case ImageUsage::CopySrc:         return "ImageUsage::CopySrc";
        case ImageUsage::CopyDst:         return "ImageUsage::CopyDst";
        case ImageUsage::Present:         return "ImageUsage::Present";
        }
    }

    inline static TL::String ToString(TL::Flags<ImageUsage> f)
    {
        TL::String result;

        if (f == ImageUsage::None)
            return "ImageUsage::None";

        bool first = true;
        if (f & ImageUsage::ShaderResource)
        {
            if (!first) result += " | ";
            result += "ImageUsage::ShaderResource";
            first = false;
        }
        if (f & ImageUsage::StorageResource)
        {
            if (!first) result += " | ";
            result += "ImageUsage::StorageResource";
            first = false;
        }
        if (f & ImageUsage::Color)
        {
            if (!first) result += " | ";
            result += "ImageUsage::Color";
            first = false;
        }
        if (f & ImageUsage::Depth)
        {
            if (!first) result += " | ";
            result += "ImageUsage::Depth";
            first = false;
        }
        if (f & ImageUsage::Stencil)
        {
            if (!first) result += " | ";
            result += "ImageUsage::Stencil";
            first = false;
        }
        if (f & ImageUsage::DepthStencil)
        {
            if (!first) result += " | ";
            result += "ImageUsage::DepthStencil";
            first = false;
        }
        if (f & ImageUsage::CopySrc)
        {
            if (!first) result += " | ";
            result += "ImageUsage::CopySrc";
            first = false;
        }
        if (f & ImageUsage::CopyDst)
        {
            if (!first) result += " | ";
            result += "ImageUsage::CopyDst";
            first = false;
        }
        if (f & ImageUsage::Present)
        {
            if (!first) result += " | ";
            result += "ImageUsage::Present";
            first = false;
        }

        return result;
    }

    inline static const char* ToString(ImageType e)
    {
        switch (e)
        {
        case ImageType::None:    return "ImageType::None";
        case ImageType::Image1D: return "ImageType::Image1D";
        case ImageType::Image2D: return "ImageType::Image2D";
        case ImageType::Image3D: return "ImageType::Image3D";
        }
    }

    inline static const char* ToString(ImageViewType e)
    {
        switch (e)
        {
        case ImageViewType::None:        return "ImageViewType::None";
        case ImageViewType::View1D:      return "ImageViewType::View1D";
        case ImageViewType::View1DArray: return "ImageViewType::View1DArray";
        case ImageViewType::View2D:      return "ImageViewType::View2D";
        case ImageViewType::View2DArray: return "ImageViewType::View2DArray";
        case ImageViewType::View3D:      return "ImageViewType::View3D";
        case ImageViewType::CubeMap:     return "ImageViewType::CubeMap";
        }
    }

    inline static const char* ToString(ImageAspect e)
    {
        switch (e)
        {
        case ImageAspect::None:         return "ImageAspect::None";
        case ImageAspect::Color:        return "ImageAspect::Color";
        case ImageAspect::Depth:        return "ImageAspect::Depth";
        case ImageAspect::Stencil:      return "ImageAspect::Stencil";
        case ImageAspect::DepthStencil: return "ImageAspect::DepthStencil";
        case ImageAspect::All:          return "ImageAspect::All";
        }
    }

    inline static const char* ToString(ComponentSwizzle e)
    {
        switch (e)
        {
        case ComponentSwizzle::Identity: return "ComponentSwizzle::Identity";
        case ComponentSwizzle::Zero:     return "ComponentSwizzle::Zero";
        case ComponentSwizzle::One:      return "ComponentSwizzle::One";
        case ComponentSwizzle::R:        return "ComponentSwizzle::R";
        case ComponentSwizzle::G:        return "ComponentSwizzle::G";
        case ComponentSwizzle::B:        return "ComponentSwizzle::B";
        case ComponentSwizzle::A:        return "ComponentSwizzle::A";
        }
    }

    inline static const char* ToString(PipelineVertexInputRate e)
    {
        switch (e)
        {
        case PipelineVertexInputRate::None:        return "PipelineVertexInputRate::None";
        case PipelineVertexInputRate::PerInstance: return "PipelineVertexInputRate::PerInstance";
        case PipelineVertexInputRate::PerVertex:   return "PipelineVertexInputRate::PerVertex";
        }
    }

    inline static const char* ToString(PipelineRasterizerStateCullMode e)
    {
        switch (e)
        {
        case PipelineRasterizerStateCullMode::None:      return "PipelineRasterizerStateCullMode::None";
        case PipelineRasterizerStateCullMode::FrontFace: return "PipelineRasterizerStateCullMode::FrontFace";
        case PipelineRasterizerStateCullMode::BackFace:  return "PipelineRasterizerStateCullMode::BackFace";
        case PipelineRasterizerStateCullMode::Discard:   return "PipelineRasterizerStateCullMode::Discard";
        }
    }

    inline static const char* ToString(PipelineRasterizerStateFillMode e)
    {
        switch (e)
        {
        case PipelineRasterizerStateFillMode::Point:    return "PipelineRasterizerStateFillMode::Point";
        case PipelineRasterizerStateFillMode::Triangle: return "PipelineRasterizerStateFillMode::Triangle";
        case PipelineRasterizerStateFillMode::Line:     return "PipelineRasterizerStateFillMode::Line";
        }
    }

    inline static const char* ToString(PipelineTopologyMode e)
    {
        switch (e)
        {
        case PipelineTopologyMode::Points:    return "PipelineTopologyMode::Points";
        case PipelineTopologyMode::Lines:     return "PipelineTopologyMode::Lines";
        case PipelineTopologyMode::Triangles: return "PipelineTopologyMode::Triangles";
        }
    }

    inline static const char* ToString(PipelineRasterizerStateFrontFace e)
    {
        switch (e)
        {
        case PipelineRasterizerStateFrontFace::Clockwise:        return "PipelineRasterizerStateFrontFace::Clockwise";
        case PipelineRasterizerStateFrontFace::CounterClockwise: return "PipelineRasterizerStateFrontFace::CounterClockwise";
        }
    }

    inline static const char* ToString(BlendFactor e)
    {
        switch (e)
        {
        case BlendFactor::Zero:                  return "BlendFactor::Zero";
        case BlendFactor::One:                   return "BlendFactor::One";
        case BlendFactor::SrcColor:              return "BlendFactor::SrcColor";
        case BlendFactor::OneMinusSrcColor:      return "BlendFactor::OneMinusSrcColor";
        case BlendFactor::DstColor:              return "BlendFactor::DstColor";
        case BlendFactor::OneMinusDstColor:      return "BlendFactor::OneMinusDstColor";
        case BlendFactor::SrcAlpha:              return "BlendFactor::SrcAlpha";
        case BlendFactor::OneMinusSrcAlpha:      return "BlendFactor::OneMinusSrcAlpha";
        case BlendFactor::DstAlpha:              return "BlendFactor::DstAlpha";
        case BlendFactor::OneMinusDstAlpha:      return "BlendFactor::OneMinusDstAlpha";
        case BlendFactor::ConstantColor:         return "BlendFactor::ConstantColor";
        case BlendFactor::OneMinusConstantColor: return "BlendFactor::OneMinusConstantColor";
        case BlendFactor::ConstantAlpha:         return "BlendFactor::ConstantAlpha";
        case BlendFactor::OneMinusConstantAlpha: return "BlendFactor::OneMinusConstantAlpha";
        }
    }

    inline static const char* ToString(BlendEquation e)
    {
        switch (e)
        {
        case BlendEquation::Add:             return "BlendEquation::Add";
        case BlendEquation::Subtract:        return "BlendEquation::Subtract";
        case BlendEquation::ReverseSubtract: return "BlendEquation::ReverseSubtract";
        case BlendEquation::Min:             return "BlendEquation::Min";
        case BlendEquation::Max:             return "BlendEquation::Max";
        }
    }

    inline static const char* ToString(ColorWriteMask e)
    {
        switch (e)
        {
        case ColorWriteMask::Red:   return "ColorWriteMask::Red";
        case ColorWriteMask::Green: return "ColorWriteMask::Green";
        case ColorWriteMask::Blue:  return "ColorWriteMask::Blue";
        case ColorWriteMask::Alpha: return "ColorWriteMask::Alpha";
        case ColorWriteMask::All:   return "ColorWriteMask::All";
        }
    }

    inline static const char* ToString(SampleCount e)
    {
        switch (e)
        {
        case SampleCount::None:      return "SampleCount::None";
        case SampleCount::Samples1:  return "SampleCount::Samples1";
        case SampleCount::Samples2:  return "SampleCount::Samples2";
        case SampleCount::Samples4:  return "SampleCount::Samples4";
        case SampleCount::Samples8:  return "SampleCount::Samples8";
        case SampleCount::Samples16: return "SampleCount::Samples16";
        case SampleCount::Samples32: return "SampleCount::Samples32";
        case SampleCount::Samples64: return "SampleCount::Samples64";
        }
    }

    inline static const char* ToString(SamplerFilter e)
    {
        switch (e)
        {
        case SamplerFilter::Point:  return "SamplerFilter::Point";
        case SamplerFilter::Linear: return "SamplerFilter::Linear";
        }
    }

    inline static const char* ToString(SamplerAddressMode e)
    {
        switch (e)
        {
        case SamplerAddressMode::Repeat: return "SamplerAddressMode::Repeat";
        case SamplerAddressMode::Clamp:  return "SamplerAddressMode::Clamp";
        }
    }

    inline static const char* ToString(CompareOperator e)
    {
        switch (e)
        {
        case CompareOperator::Undefined:      return "CompareOperator::Undefined";
        case CompareOperator::Never:          return "CompareOperator::Never";
        case CompareOperator::Equal:          return "CompareOperator::Equal";
        case CompareOperator::NotEqual:       return "CompareOperator::NotEqual";
        case CompareOperator::Greater:        return "CompareOperator::Greater";
        case CompareOperator::GreaterOrEqual: return "CompareOperator::GreaterOrEqual";
        case CompareOperator::Less:           return "CompareOperator::Less";
        case CompareOperator::LessOrEqual:    return "CompareOperator::LessOrEqual";
        case CompareOperator::Always:         return "CompareOperator::Always";
        }
    }

    inline static TL::String ToString(const ImageCreateInfo& ci)
    {
        TL::String result;
        result += "ImageCreateInfo{\n";
        result += "    name: ";
        result += ci.name;
        result += ",\n    usageFlags: ";
        result += ToString(ci.usageFlags);
        result += ",\n    type: ";
        result += ToString(ci.type);
        result += ",\n    size: ";
        result += std::format("({}, {}, {})", ci.size.width, ci.size.height, ci.size.depth);
        result += ",\n    format: ";
        result += ToString(ci.format);
        result += ",\n    sampleCount: ";
        result += ToString(ci.sampleCount);
        result += ",\n    mipLevels: ";
        result += std::to_string(ci.mipLevels);
        result += ",\n    arrayCount: ";
        result += std::to_string(ci.arrayCount);
        result += "\n}";
        return result;
    }

    inline static TL::String ToString(const ImageViewCreateInfo& ci)
    {
        TL::String result;
        result += "ImageViewCreateInfo{\n";
        result += "    name: ";
        result += ci.name;
        result += ",\n    image: ";
        // result += ci.image ? std::format("{}", static_cast<const void*>(ci.image)) : "nullptr";
        result += "TODO:D";
        result += ",\n    type: ";
        result += "TODO:D";
        result += ",\n    format: ";
        result += ToString(ci.format);
        result += ",\n    aspect: ";
        result += "TODO:D";
        result += ",\n    baseMipLevel: ";
        result += std::to_string(ci.subresource.mipBase);
        result += ",\n    mipLevelCount: ";
        result += std::to_string(ci.subresource.mipLevelCount);
        result += ",\n    baseArrayLayer: ";
        result += std::to_string(ci.subresource.arrayBase);
        result += ",\n    arrayLayerCount: ";
        result += std::to_string(ci.subresource.arrayCount);
        result += "\n}";
        return result;
    }

    inline static TL::String ToString(const BufferCreateInfo& ci)
    {
        TL::String result;
        result += "BufferCreateInfo{\n";
        result += "    name: ";
        result += ci.name;
        result += ",\n    usageFlags: ";
        result += ToString(ci.usageFlags);
        result += ",\n    size: ";
        result += std::to_string(ci.byteSize);
        result += "\n}";
        return result;
    }

    inline static TL::String ToString(const ShaderBinding& shaderBinding)
    {
        TL::String result;
        result += "ShaderBinding{\n";
        result += ",\n    type: ";
        result += ToString(shaderBinding.type);
        result += ",\n    count: ";
        result += std::to_string(shaderBinding.arrayCount);
        result += "\n}";
        return result;
    }
}; // namespace RHI::Debug

#include <format>

#define RHI_DEFINE_ENUM_FORMATTER(RHI_TYPE)                                          \
    namespace std                                                                    \
    {                                                                                \
        template<>                                                                   \
        struct formatter<RHI_TYPE> : formatter<TL::String>                       \
        {                                                                            \
            auto format(RHI_TYPE e, format_context& ctx) const                       \
            {                                                                        \
                return formatter<const char*>::format(RHI::Debug::ToString(e), ctx); \
            }                                                                        \
        }                                                                            \
    }

//
// RHI_DEFINE_ENUM_FORMATTER(RHI::BindingType);
// RHI_DEFINE_ENUM_FORMATTER(RHI::ShaderStage);
// RHI_DEFINE_ENUM_FORMATTER(RHI::BufferUsage);
// RHI_DEFINE_ENUM_FORMATTER(RHI::IndexType);
// RHI_DEFINE_ENUM_FORMATTER(RHI::ImageUsage);
// RHI_DEFINE_ENUM_FORMATTER(RHI::ImageType);
// RHI_DEFINE_ENUM_FORMATTER(RHI::ImageViewType);
// RHI_DEFINE_ENUM_FORMATTER(RHI::ImageAspect);
// RHI_DEFINE_ENUM_FORMATTER(RHI::ComponentSwizzle);
// RHI_DEFINE_ENUM_FORMATTER(RHI::PipelineVertexInputRate);
// RHI_DEFINE_ENUM_FORMATTER(RHI::PipelineRasterizerStateCullMode);
// RHI_DEFINE_ENUM_FORMATTER(RHI::PipelineRasterizerStateFillMode);
// RHI_DEFINE_ENUM_FORMATTER(RHI::PipelineTopologyMode);
// RHI_DEFINE_ENUM_FORMATTER(RHI::PipelineRasterizerStateFrontFace);
// RHI_DEFINE_ENUM_FORMATTER(RHI::BlendFactor);
// RHI_DEFINE_ENUM_FORMATTER(RHI::BlendEquation);
// RHI_DEFINE_ENUM_FORMATTER(RHI::ColorWriteMask);
// RHI_DEFINE_ENUM_FORMATTER(RHI::SampleCount);
// RHI_DEFINE_ENUM_FORMATTER(RHI::SamplerFilter);
// RHI_DEFINE_ENUM_FORMATTER(RHI::SamplerAddressMode);
// RHI_DEFINE_ENUM_FORMATTER(RHI::CompareOperator);
