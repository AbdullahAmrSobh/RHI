#include "resources.h"

#include "device.h"

#include <TL/Containers/InlineVector.hpp>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace RHI::WebGPU
{
    inline static WGPUStringView MakeStringView(const char* value, size_t length = WGPU_STRLEN)
    {
        return WGPUStringView{
            .data = value,
            .length = length,
        };
    }

    inline static WGPUShaderStage ConvertShaderStages(TL::Flags<ShaderStage> stages)
    {
        WGPUShaderStage result = WGPUShaderStage_None;
        if (stages & ShaderStage::Vertex) result |= WGPUShaderStage_Vertex;
        if (stages & ShaderStage::Pixel) result |= WGPUShaderStage_Fragment;
        if (stages & ShaderStage::Compute) result |= WGPUShaderStage_Compute;
        return result;
    }

    inline static WGPUBufferUsage ConvertBufferUsage(TL::Flags<BufferUsage> usage)
    {
        WGPUBufferUsage result = WGPUBufferUsage_None;
        if (usage & BufferUsage::Storage) result |= WGPUBufferUsage_Storage;
        if (usage & BufferUsage::Uniform) result |= WGPUBufferUsage_Uniform;
        if (usage & BufferUsage::Vertex) result |= WGPUBufferUsage_Vertex;
        if (usage & BufferUsage::Index) result |= WGPUBufferUsage_Index;
        if (usage & BufferUsage::CopySrc) result |= WGPUBufferUsage_CopySrc;
        if (usage & BufferUsage::CopyDst) result |= WGPUBufferUsage_CopyDst;
        if (usage & BufferUsage::Indirect) result |= WGPUBufferUsage_Indirect;
        return result;
    }

    inline static WGPUTextureUsage ConvertImageUsage(TL::Flags<ImageUsage> usage)
    {
        WGPUTextureUsage result = WGPUTextureUsage_None;
        if (usage & ImageUsage::ShaderResource) result |= WGPUTextureUsage_TextureBinding;
        if (usage & ImageUsage::StorageResource) result |= WGPUTextureUsage_StorageBinding;
        if (usage & ImageUsage::Color) result |= WGPUTextureUsage_RenderAttachment;
        if (usage & ImageUsage::Depth) result |= WGPUTextureUsage_RenderAttachment;
        if (usage & ImageUsage::Stencil) result |= WGPUTextureUsage_RenderAttachment;
        if (usage & ImageUsage::CopySrc) result |= WGPUTextureUsage_CopySrc;
        if (usage & ImageUsage::CopyDst) result |= WGPUTextureUsage_CopyDst;
        return result;
    }

    inline static WGPUTextureDimension ConvertImageType(ImageType type)
    {
        switch (type)
        {
        case ImageType::Image1D: return WGPUTextureDimension_1D;
        case ImageType::Image2D: return WGPUTextureDimension_2D;
        case ImageType::Image3D: return WGPUTextureDimension_3D;
        default: return WGPUTextureDimension_Undefined;
        }
    }

    inline static WGPUTextureViewDimension ConvertImageViewType(ImageViewType type)
    {
        switch (type)
        {
        case ImageViewType::View1D: return WGPUTextureViewDimension_1D;
        case ImageViewType::View1DArray: return WGPUTextureViewDimension_Undefined;
        case ImageViewType::View2D: return WGPUTextureViewDimension_2D;
        case ImageViewType::View2DArray: return WGPUTextureViewDimension_2DArray;
        case ImageViewType::View3D: return WGPUTextureViewDimension_3D;
        case ImageViewType::CubeMap: return WGPUTextureViewDimension_Cube;
        default: return WGPUTextureViewDimension_Undefined;
        }
    }

    inline static WGPUSamplerBindingType ConvertSamplerBindingType(SamplerBindingType type)
    {
        switch (type)
        {
        case SamplerBindingType::Filtering: return WGPUSamplerBindingType_Filtering;
        case SamplerBindingType::NonFiltering: return WGPUSamplerBindingType_NonFiltering;
        case SamplerBindingType::Comparison: return WGPUSamplerBindingType_Comparison;
        }
        return WGPUSamplerBindingType_BindingNotUsed;
    }

    inline static uint32_t ConvertSampleCount(SampleCount count)
    {
        switch (count)
        {
        case SampleCount::None:
        case SampleCount::Samples1: return 1;
        case SampleCount::Samples2: return 2;
        case SampleCount::Samples4: return 4;
        case SampleCount::Samples8: return 8;
        case SampleCount::Samples16: return 16;
        case SampleCount::Samples32: return 32;
        case SampleCount::Samples64: return 64;
        }
        return 1;
    }

    inline static WGPUTextureFormat ConvertFormat(Format format)
    {
        switch (format)
        {
        case Format::R8_UINT: return WGPUTextureFormat_R8Uint;
        case Format::R8_SINT: return WGPUTextureFormat_R8Sint;
        case Format::R8_UNORM: return WGPUTextureFormat_R8Unorm;
        case Format::R8_SNORM: return WGPUTextureFormat_R8Snorm;
        case Format::RG8_UINT: return WGPUTextureFormat_RG8Uint;
        case Format::RG8_SINT: return WGPUTextureFormat_RG8Sint;
        case Format::RG8_UNORM: return WGPUTextureFormat_RG8Unorm;
        case Format::RG8_SNORM: return WGPUTextureFormat_RG8Snorm;
        case Format::R16_UINT: return WGPUTextureFormat_R16Uint;
        case Format::R16_SINT: return WGPUTextureFormat_R16Sint;
        case Format::R16_UNORM: return WGPUTextureFormat_R16Unorm;
        case Format::R16_SNORM: return WGPUTextureFormat_R16Snorm;
        case Format::R16_FLOAT: return WGPUTextureFormat_R16Float;
        case Format::RGBA8_UINT: return WGPUTextureFormat_RGBA8Uint;
        case Format::RGBA8_SINT: return WGPUTextureFormat_RGBA8Sint;
        case Format::RGBA8_UNORM: return WGPUTextureFormat_RGBA8Unorm;
        case Format::RGBA8_SNORM: return WGPUTextureFormat_RGBA8Snorm;
        case Format::BGRA8_UNORM: return WGPUTextureFormat_BGRA8Unorm;
        case Format::SRGBA8_UNORM: return WGPUTextureFormat_RGBA8UnormSrgb;
        case Format::SBGRA8_UNORM: return WGPUTextureFormat_BGRA8UnormSrgb;
        case Format::R10G10B10A2_UNORM: return WGPUTextureFormat_RGB10A2Unorm;
        case Format::R11G11B10_FLOAT: return WGPUTextureFormat_RG11B10Ufloat;
        case Format::RG16_UINT: return WGPUTextureFormat_RG16Uint;
        case Format::RG16_SINT: return WGPUTextureFormat_RG16Sint;
        case Format::RG16_UNORM: return WGPUTextureFormat_RG16Unorm;
        case Format::RG16_SNORM: return WGPUTextureFormat_RG16Snorm;
        case Format::RG16_FLOAT: return WGPUTextureFormat_RG16Float;
        case Format::R32_UINT: return WGPUTextureFormat_R32Uint;
        case Format::R32_SINT: return WGPUTextureFormat_R32Sint;
        case Format::R32_FLOAT: return WGPUTextureFormat_R32Float;
        case Format::RGBA16_UINT: return WGPUTextureFormat_RGBA16Uint;
        case Format::RGBA16_SINT: return WGPUTextureFormat_RGBA16Sint;
        case Format::RGBA16_FLOAT: return WGPUTextureFormat_RGBA16Float;
        case Format::RGBA16_UNORM: return WGPUTextureFormat_RGBA16Unorm;
        case Format::RGBA16_SNORM: return WGPUTextureFormat_RGBA16Snorm;
        case Format::RG32_UINT: return WGPUTextureFormat_RG32Uint;
        case Format::RG32_SINT: return WGPUTextureFormat_RG32Sint;
        case Format::RG32_FLOAT: return WGPUTextureFormat_RG32Float;
        case Format::RGBA32_UINT: return WGPUTextureFormat_RGBA32Uint;
        case Format::RGBA32_SINT: return WGPUTextureFormat_RGBA32Sint;
        case Format::RGBA32_FLOAT: return WGPUTextureFormat_RGBA32Float;
        case Format::D16: return WGPUTextureFormat_Depth16Unorm;
        case Format::D24S8: return WGPUTextureFormat_Depth24PlusStencil8;
        case Format::D32: return WGPUTextureFormat_Depth32Float;
        case Format::D32S8: return WGPUTextureFormat_Depth32FloatStencil8;
        case Format::BC1_UNORM: return WGPUTextureFormat_BC1RGBAUnorm;
        case Format::BC1_UNORM_SRGB: return WGPUTextureFormat_BC1RGBAUnormSrgb;
        case Format::BC2_UNORM: return WGPUTextureFormat_BC2RGBAUnorm;
        case Format::BC2_UNORM_SRGB: return WGPUTextureFormat_BC2RGBAUnormSrgb;
        case Format::BC3_UNORM: return WGPUTextureFormat_BC3RGBAUnorm;
        case Format::BC3_UNORM_SRGB: return WGPUTextureFormat_BC3RGBAUnormSrgb;
        case Format::BC4_UNORM: return WGPUTextureFormat_BC4RUnorm;
        case Format::BC4_SNORM: return WGPUTextureFormat_BC4RSnorm;
        case Format::BC5_UNORM: return WGPUTextureFormat_BC5RGUnorm;
        case Format::BC5_SNORM: return WGPUTextureFormat_BC5RGSnorm;
        case Format::BC6H_UFLOAT: return WGPUTextureFormat_BC6HRGBUfloat;
        case Format::BC6H_SFLOAT: return WGPUTextureFormat_BC6HRGBFloat;
        case Format::BC7_UNORM: return WGPUTextureFormat_BC7RGBAUnorm;
        case Format::BC7_UNORM_SRGB: return WGPUTextureFormat_BC7RGBAUnormSrgb;
        default: return WGPUTextureFormat_Undefined;
        }
    }

    inline static Format ConvertFormat(WGPUTextureFormat format)
    {
        switch (format)
        {
        case WGPUTextureFormat_RGBA8Unorm: return Format::RGBA8_UNORM;
        case WGPUTextureFormat_RGBA8UnormSrgb: return Format::SRGBA8_UNORM;
        case WGPUTextureFormat_BGRA8Unorm: return Format::BGRA8_UNORM;
        case WGPUTextureFormat_BGRA8UnormSrgb: return Format::SBGRA8_UNORM;
        case WGPUTextureFormat_Depth16Unorm: return Format::D16;
        case WGPUTextureFormat_Depth24PlusStencil8: return Format::D24S8;
        case WGPUTextureFormat_Depth32Float: return Format::D32;
        case WGPUTextureFormat_Depth32FloatStencil8: return Format::D32S8;
        default: return Format::Unknown;
        }
    }

    inline static WGPUVertexFormat ConvertVertexFormat(Format format)
    {
        switch (format)
        {
        case Format::R8_UINT: return WGPUVertexFormat_Uint8;
        case Format::R8_SINT: return WGPUVertexFormat_Sint8;
        case Format::R8_UNORM: return WGPUVertexFormat_Unorm8;
        case Format::R8_SNORM: return WGPUVertexFormat_Snorm8;
        case Format::RG8_UINT: return WGPUVertexFormat_Uint8x2;
        case Format::RG8_SINT: return WGPUVertexFormat_Sint8x2;
        case Format::RG8_UNORM: return WGPUVertexFormat_Unorm8x2;
        case Format::RG8_SNORM: return WGPUVertexFormat_Snorm8x2;
        case Format::RGBA8_UINT: return WGPUVertexFormat_Uint8x4;
        case Format::RGBA8_SINT: return WGPUVertexFormat_Sint8x4;
        case Format::RGBA8_UNORM: return WGPUVertexFormat_Unorm8x4;
        case Format::RGBA8_SNORM: return WGPUVertexFormat_Snorm8x4;
        case Format::RG16_UINT: return WGPUVertexFormat_Uint16x2;
        case Format::RG16_SINT: return WGPUVertexFormat_Sint16x2;
        case Format::RG16_UNORM: return WGPUVertexFormat_Unorm16x2;
        case Format::RG16_SNORM: return WGPUVertexFormat_Snorm16x2;
        case Format::RG16_FLOAT: return WGPUVertexFormat_Float16x2;
        case Format::RGBA16_UINT: return WGPUVertexFormat_Uint16x4;
        case Format::RGBA16_SINT: return WGPUVertexFormat_Sint16x4;
        case Format::RGBA16_UNORM: return WGPUVertexFormat_Unorm16x4;
        case Format::RGBA16_SNORM: return WGPUVertexFormat_Snorm16x4;
        case Format::RGBA16_FLOAT: return WGPUVertexFormat_Float16x4;
        case Format::R32_UINT: return WGPUVertexFormat_Uint32;
        case Format::R32_SINT: return WGPUVertexFormat_Sint32;
        case Format::R32_FLOAT: return WGPUVertexFormat_Float32;
        case Format::RG32_UINT: return WGPUVertexFormat_Uint32x2;
        case Format::RG32_SINT: return WGPUVertexFormat_Sint32x2;
        case Format::RG32_FLOAT: return WGPUVertexFormat_Float32x2;
        case Format::RGB32_UINT: return WGPUVertexFormat_Uint32x3;
        case Format::RGB32_SINT: return WGPUVertexFormat_Sint32x3;
        case Format::RGB32_FLOAT: return WGPUVertexFormat_Float32x3;
        case Format::RGBA32_UINT: return WGPUVertexFormat_Uint32x4;
        case Format::RGBA32_SINT: return WGPUVertexFormat_Sint32x4;
        case Format::RGBA32_FLOAT: return WGPUVertexFormat_Float32x4;
        default: return (WGPUVertexFormat)0;
        }
    }

    inline static WGPUFilterMode ConvertFilter(SamplerFilter filter)
    {
        return filter == SamplerFilter::Linear ? WGPUFilterMode_Linear : WGPUFilterMode_Nearest;
    }

    inline static WGPUMipmapFilterMode ConvertMipmapFilter(SamplerFilter filter)
    {
        return filter == SamplerFilter::Linear ? WGPUMipmapFilterMode_Linear : WGPUMipmapFilterMode_Nearest;
    }

    inline static WGPUAddressMode ConvertAddressMode(SamplerAddressMode mode)
    {
        return mode == SamplerAddressMode::Clamp ? WGPUAddressMode_ClampToEdge : WGPUAddressMode_Repeat;
    }

    inline static WGPUCompareFunction ConvertCompare(CompareOperator compare)
    {
        switch (compare)
        {
        case CompareOperator::Never: return WGPUCompareFunction_Never;
        case CompareOperator::Equal: return WGPUCompareFunction_Equal;
        case CompareOperator::NotEqual: return WGPUCompareFunction_NotEqual;
        case CompareOperator::Greater: return WGPUCompareFunction_Greater;
        case CompareOperator::GreaterOrEqual: return WGPUCompareFunction_GreaterEqual;
        case CompareOperator::Less: return WGPUCompareFunction_Less;
        case CompareOperator::LessOrEqual: return WGPUCompareFunction_LessEqual;
        case CompareOperator::Always: return WGPUCompareFunction_Always;
        default: return WGPUCompareFunction_Undefined;
        }
    }

    inline static WGPUPrimitiveTopology ConvertTopology(PipelineTopologyMode topology)
    {
        switch (topology)
        {
        case PipelineTopologyMode::Points: return WGPUPrimitiveTopology_PointList;
        case PipelineTopologyMode::Lines: return WGPUPrimitiveTopology_LineList;
        case PipelineTopologyMode::Triangles: return WGPUPrimitiveTopology_TriangleList;
        }
        return WGPUPrimitiveTopology_Undefined;
    }

    inline static WGPUCullMode ConvertCullMode(PipelineRasterizerStateCullMode mode)
    {
        switch (mode)
        {
        case PipelineRasterizerStateCullMode::None: return WGPUCullMode_None;
        case PipelineRasterizerStateCullMode::FrontFace: return WGPUCullMode_Front;
        case PipelineRasterizerStateCullMode::BackFace: return WGPUCullMode_Back;
        default: return WGPUCullMode_None;
        }
    }

    inline static WGPUFrontFace ConvertFrontFace(PipelineRasterizerStateFrontFace face)
    {
        return face == PipelineRasterizerStateFrontFace::Clockwise ? WGPUFrontFace_CW : WGPUFrontFace_CCW;
    }

    inline static WGPUBlendFactor ConvertBlendFactor(BlendFactor factor)
    {
        switch (factor)
        {
        case BlendFactor::Zero: return WGPUBlendFactor_Zero;
        case BlendFactor::One: return WGPUBlendFactor_One;
        case BlendFactor::SrcColor: return WGPUBlendFactor_Src;
        case BlendFactor::OneMinusSrcColor: return WGPUBlendFactor_OneMinusSrc;
        case BlendFactor::DstColor: return WGPUBlendFactor_Dst;
        case BlendFactor::OneMinusDstColor: return WGPUBlendFactor_OneMinusDst;
        case BlendFactor::SrcAlpha: return WGPUBlendFactor_SrcAlpha;
        case BlendFactor::OneMinusSrcAlpha: return WGPUBlendFactor_OneMinusSrcAlpha;
        case BlendFactor::DstAlpha: return WGPUBlendFactor_DstAlpha;
        case BlendFactor::OneMinusDstAlpha: return WGPUBlendFactor_OneMinusDstAlpha;
        case BlendFactor::ConstantColor:
        case BlendFactor::ConstantAlpha: return WGPUBlendFactor_Constant;
        case BlendFactor::OneMinusConstantColor:
        case BlendFactor::OneMinusConstantAlpha: return WGPUBlendFactor_OneMinusConstant;
        }
        return WGPUBlendFactor_Undefined;
    }

    inline static WGPUBlendOperation ConvertBlendOperation(BlendEquation operation)
    {
        switch (operation)
        {
        case BlendEquation::Add: return WGPUBlendOperation_Add;
        case BlendEquation::Subtract: return WGPUBlendOperation_Subtract;
        case BlendEquation::ReverseSubtract: return WGPUBlendOperation_ReverseSubtract;
        case BlendEquation::Min: return WGPUBlendOperation_Min;
        case BlendEquation::Max: return WGPUBlendOperation_Max;
        }
        return WGPUBlendOperation_Undefined;
    }

    inline static WGPUColorWriteMask ConvertColorWriteMask(TL::Flags<ColorWriteMask> mask)
    {
        WGPUColorWriteMask result = WGPUColorWriteMask_None;
        if (mask & ColorWriteMask::Red) result |= WGPUColorWriteMask_Red;
        if (mask & ColorWriteMask::Green) result |= WGPUColorWriteMask_Green;
        if (mask & ColorWriteMask::Blue) result |= WGPUColorWriteMask_Blue;
        if (mask & ColorWriteMask::Alpha) result |= WGPUColorWriteMask_Alpha;
        return result;
    }

    inline static WGPUTextureAspect ConvertImageAspect(TL::Flags<ImageAspect> aspects)
    {
        if ((aspects & ImageAspect::Depth) && !(aspects & ImageAspect::Stencil)) return WGPUTextureAspect_DepthOnly;
        if ((aspects & ImageAspect::Stencil) && !(aspects & ImageAspect::Depth)) return WGPUTextureAspect_StencilOnly;
        return WGPUTextureAspect_All;
    }

    inline static WGPUComponentSwizzle ConvertComponentSwizzle(ComponentSwizzle swizzle)
    {
        switch (swizzle)
        {
        case ComponentSwizzle::Zero: return WGPUComponentSwizzle_Zero;
        case ComponentSwizzle::One: return WGPUComponentSwizzle_One;
        case ComponentSwizzle::R: return WGPUComponentSwizzle_R;
        case ComponentSwizzle::G: return WGPUComponentSwizzle_G;
        case ComponentSwizzle::B: return WGPUComponentSwizzle_B;
        case ComponentSwizzle::A: return WGPUComponentSwizzle_A;
        default: return WGPUComponentSwizzle_Undefined;
        }
    }

    inline static bool IsIdentityMapping(const ComponentMapping& mapping)
    {
        return mapping.r == ComponentSwizzle::Identity && mapping.g == ComponentSwizzle::Identity && mapping.b == ComponentSwizzle::Identity && mapping.a == ComponentSwizzle::Identity;
    }

    void IFence::Init(IDevice* device, const FenceCreateInfo& createInfo)
    {
        value.store(createInfo.initialValue);
    }

    void IFence::Shutdown(IDevice* device)
    {
    }

    void IBindGroupLayout::Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo)
    {
        for (ShaderBinding binding : createInfo.bindings)
            bindings.push_back(binding);
        TL::InlineVector<WGPUBindGroupLayoutEntry, MaxBindingsPerGroup> entries;

        for (uint32_t bindingIndex = 0; bindingIndex < bindings.size(); ++bindingIndex)
        {
            ShaderBinding& binding = bindings[bindingIndex];
            WGPUBindGroupLayoutEntry entry{
                .nextInChain = nullptr,
                .binding = bindingIndex,
                .visibility = ConvertShaderStages(binding.stages),
                .bindingArraySize = 0,
                .buffer = WGPUBufferBindingLayout{
                    .nextInChain = nullptr,
                    .type = WGPUBufferBindingType_BindingNotUsed,
                    .hasDynamicOffset = WGPU_FALSE,
                    .minBindingSize = 0,
                },
                .sampler = WGPUSamplerBindingLayout{
                    .nextInChain = nullptr,
                    .type = WGPUSamplerBindingType_BindingNotUsed,
                },
                .texture = WGPUTextureBindingLayout{
                    .nextInChain = nullptr,
                    .sampleType = WGPUTextureSampleType_BindingNotUsed,
                    .viewDimension = WGPUTextureViewDimension_Undefined,
                    .multisampled = WGPU_FALSE,
                },
                .storageTexture = WGPUStorageTextureBindingLayout{
                    .nextInChain = nullptr,
                    .access = WGPUStorageTextureAccess_BindingNotUsed,
                    .format = WGPUTextureFormat_Undefined,
                    .viewDimension = WGPUTextureViewDimension_Undefined,
                },
            };

            switch (binding.type)
            {
            case BindingType::Sampler:
                entry.sampler.type = ConvertSamplerBindingType(binding.samplerType);
                break;
            case BindingType::SampledImage:
                entry.texture.sampleType = WGPUTextureSampleType_Float;
                entry.texture.viewDimension = ConvertImageViewType(binding.imageViewType);
                break;
            case BindingType::UniformBuffer:
            case BindingType::UniformBufferDynamic:
                entry.buffer.type = WGPUBufferBindingType_Uniform;
                entry.buffer.hasDynamicOffset = binding.type == BindingType::UniformBufferDynamic ? WGPU_TRUE : WGPU_FALSE;
                entry.buffer.minBindingSize = binding.bufferStride;
                break;
            case BindingType::StorageBuffer:
            case BindingType::StorageBufferDynamic:
                entry.buffer.type = binding.access == Access::Read ? WGPUBufferBindingType_ReadOnlyStorage : WGPUBufferBindingType_Storage;
                entry.buffer.hasDynamicOffset = binding.type == BindingType::StorageBufferDynamic ? WGPU_TRUE : WGPU_FALSE;
                entry.buffer.minBindingSize = binding.bufferStride;
                break;
            default: TL_UNREACHABLE();
            }
            entries.push_back(entry);
        }

        WGPUBindGroupLayoutDescriptor descriptor{
            .nextInChain = nullptr,
            .label = MakeStringView(createInfo.name),
            .entryCount = entries.size(),
            .entries = entries.data(),
        };
        handle = wgpuDeviceCreateBindGroupLayout(device->m_device, &descriptor);
    }

    void IBindGroupLayout::Shutdown(IDevice* device)
    {
        wgpuBindGroupLayoutRelease(handle);
        handle = nullptr;
        bindings.clear();
    }

    void IBindGroup::Init(IDevice* device, const BindGroupCreateInfo& createInfo)
    {
        layout = (IBindGroupLayout*)createInfo.layout;
        label = createInfo.name;
        entries.resize(layout->bindings.size());
        for (uint32_t binding = 0; binding < entries.size(); ++binding)
        {
            entries[binding] = WGPUBindGroupEntry{
                .nextInChain = nullptr,
                .binding = binding,
                .buffer = nullptr,
                .offset = 0,
                .size = WGPU_WHOLE_SIZE,
                .sampler = nullptr,
                .textureView = nullptr,
            };
        }

    }

    void IBindGroup::Shutdown(IDevice* device)
    {
        wgpuBindGroupRelease(handle);
        handle = nullptr;
        layout = nullptr;
        label = nullptr;
        entries.clear();
    }

    void IBindGroup::Update(IDevice* device, const BindGroupUpdateInfo& updateInfo)
    {
        for (auto update : updateInfo.buffers)
        {
            auto binding = update.buffers[0];
            auto* buffer = (IBuffer*)binding.buffer;
            entries[update.dstBinding].buffer = buffer->handle;
            entries[update.dstBinding].offset = binding.offset;
            entries[update.dstBinding].size = binding.range == 0 ? WGPU_WHOLE_SIZE : binding.range;
        }

        for (auto update : updateInfo.images)
        {
            auto* image = (IImage*)update.images[0];
            entries[update.dstBinding].textureView = image->viewHandle;
        }

        for (auto update : updateInfo.samplers)
        {
            auto* sampler = (ISampler*)update.samplers[0];
            entries[update.dstBinding].sampler = sampler->handle;
        }

        WGPUBindGroupDescriptor descriptor{
            .nextInChain = nullptr,
            .label = MakeStringView(label),
            .layout = layout->handle,
            .entryCount = entries.size(),
            .entries = entries.data(),
        };
        handle = wgpuDeviceCreateBindGroup(device->m_device, &descriptor);
    }

    void IShaderModule::Init(IDevice* device, const ShaderModuleCreateInfo& createInfo)
    {
        WGPUShaderModuleDescriptor descriptor{
            .nextInChain = nullptr,
            .label = MakeStringView(createInfo.name),
        };

        if (createInfo.code[0] == 0x07230203)
        {
            WGPUShaderSourceSPIRV source{
                .chain = WGPUChainedStruct{
                    .next = nullptr,
                    .sType = WGPUSType_ShaderSourceSPIRV,
                },
                .codeSize = (uint32_t)createInfo.code.size(),
                .code = createInfo.code.data(),
            };
            descriptor.nextInChain = &source.chain;
            handle = wgpuDeviceCreateShaderModule(device->m_device, &descriptor);
        }
        else
        {
            auto* code = (const char*)createInfo.code.data();
            size_t codeSize = std::strlen(code);
            WGPUShaderSourceWGSL source{
                .chain = WGPUChainedStruct{
                    .next = nullptr,
                    .sType = WGPUSType_ShaderSourceWGSL,
                },
                .code = MakeStringView(code, codeSize),
            };
            descriptor.nextInChain = &source.chain;
            handle = wgpuDeviceCreateShaderModule(device->m_device, &descriptor);
        }
    }

    void IShaderModule::Shutdown(IDevice* device)
    {
        wgpuShaderModuleRelease(handle);
        handle = nullptr;
    }

    void IPipelineLayout::Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo)
    {
        TL::InlineVector<WGPUBindGroupLayout, MaxBindGroupsPerPipeline> layouts;
        for (BindGroupLayout* layout : createInfo.layouts)
        {
            auto* webGPULayout = (IBindGroupLayout*)layout;
            layouts.push_back(webGPULayout->handle);
        }

        uint32_t immediateSize = 0;
        for (auto pushConstant : createInfo.pushConstants)
            immediateSize = std::max(immediateSize, pushConstant.offset + pushConstant.size);

        WGPUPipelineLayoutDescriptor descriptor{
            .nextInChain = nullptr,
            .label = MakeStringView(createInfo.name),
            .bindGroupLayoutCount = layouts.size(),
            .bindGroupLayouts = layouts.data(),
            .immediateSize = immediateSize,
        };
        handle = wgpuDeviceCreatePipelineLayout(device->m_device, &descriptor);
    }

    void IPipelineLayout::Shutdown(IDevice* device)
    {
        wgpuPipelineLayoutRelease(handle);
        handle = nullptr;
    }

    void IGraphicsPipeline::Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo)
    {
        layout = (IPipelineLayout*)createInfo.layout;

        PipelineShaderStage vertexStage{};
        PipelineShaderStage fragmentStage{};
        for (auto stage : createInfo.shaderStages)
        {
            if (stage.stage == ShaderStage::Vertex)
            {
                vertexStage = stage;
            }
            if (stage.stage == ShaderStage::Pixel)
            {
                fragmentStage = stage;
            }
        }
        TL::InlineVector<TL::InlineVector<WGPUVertexAttribute, MaxVertexAttributesPerBuffer>, MaxVertexBuffers> attributeStorage;
        TL::InlineVector<WGPUVertexBufferLayout, MaxVertexBuffers> vertexBuffers;
        attributeStorage.resize(createInfo.vertexBufferBindings.size());
        vertexBuffers.resize(createInfo.vertexBufferBindings.size());
        uint32_t shaderLocation = 0;
        for (size_t bindingIndex = 0; bindingIndex < createInfo.vertexBufferBindings.size(); ++bindingIndex)
        {
            auto binding = createInfo.vertexBufferBindings[bindingIndex];
            auto& attributes = attributeStorage[bindingIndex];
            for (auto attribute : binding.attributes)
            {
                attributes.push_back(WGPUVertexAttribute{
                    .nextInChain = nullptr,
                    .format = ConvertVertexFormat(attribute.format),
                    .offset = attribute.offset,
                    .shaderLocation = shaderLocation++,
                });
            }
            vertexBuffers[bindingIndex] = WGPUVertexBufferLayout{
                .nextInChain = nullptr,
                .stepMode = binding.stepRate == PipelineVertexInputRate::PerInstance ? WGPUVertexStepMode_Instance : WGPUVertexStepMode_Vertex,
                .arrayStride = binding.stride,
                .attributeCount = attributes.size(),
                .attributes = attributes.data(),
            };
        }

        TL::InlineVector<WGPUBlendState, MaxColorAttachments> blendStates;
        TL::InlineVector<WGPUColorTargetState, MaxColorAttachments> colorTargets;
        blendStates.resize(createInfo.renderTargetLayout.colors.size());
        colorTargets.resize(createInfo.renderTargetLayout.colors.size());
        for (size_t index = 0; index < colorTargets.size(); ++index)
        {
            ColorAttachmentBlendStateDesc blend = createInfo.colorBlendState.blendStates[index];
            blendStates[index] = WGPUBlendState{
                .color = WGPUBlendComponent{
                    .operation = ConvertBlendOperation(blend.colorBlendOp),
                    .srcFactor = ConvertBlendFactor(blend.srcColor),
                    .dstFactor = ConvertBlendFactor(blend.dstColor),
                },
                .alpha = WGPUBlendComponent{
                    .operation = ConvertBlendOperation(blend.alphaBlendOp),
                    .srcFactor = ConvertBlendFactor(blend.srcAlpha),
                    .dstFactor = ConvertBlendFactor(blend.dstAlpha),
                },
            };
            colorTargets[index] = WGPUColorTargetState{
                .nextInChain = nullptr,
                .format = ConvertFormat(createInfo.renderTargetLayout.colors[index]),
                .blend = blend.blendEnable ? &blendStates[index] : nullptr,
                .writeMask = ConvertColorWriteMask(blend.writeMask),
            };
        }

        WGPUFragmentState fragment{
            .nextInChain = nullptr,
            .module = ((IShaderModule*)fragmentStage.module)->handle,
            .entryPoint = MakeStringView(fragmentStage.name),
            .constantCount = 0,
            .constants = nullptr,
            .targetCount = colorTargets.size(),
            .targets = colorTargets.data(),
        };

        Format depthFormat = createInfo.renderTargetLayout.depth != Format::Unknown
                                 ? createInfo.renderTargetLayout.depth
                                 : createInfo.renderTargetLayout.stencil;
        WGPUDepthStencilState depthStencil{
            .nextInChain = nullptr,
            .format = ConvertFormat(depthFormat),
            .depthWriteEnabled = createInfo.depthStencilState.depthWriteEnable ? WGPUOptionalBool_True : WGPUOptionalBool_False,
            .depthCompare = createInfo.depthStencilState.depthTestEnable ? ConvertCompare(createInfo.depthStencilState.compareOperator) : WGPUCompareFunction_Always,
            .stencilFront = WGPUStencilFaceState{
                .compare = WGPUCompareFunction_Always,
                .failOp = WGPUStencilOperation_Keep,
                .depthFailOp = WGPUStencilOperation_Keep,
                .passOp = WGPUStencilOperation_Keep,
            },
            .stencilBack = WGPUStencilFaceState{
                .compare = WGPUCompareFunction_Always,
                .failOp = WGPUStencilOperation_Keep,
                .depthFailOp = WGPUStencilOperation_Keep,
                .passOp = WGPUStencilOperation_Keep,
            },
            .stencilReadMask = 0xFFFFFFFF,
            .stencilWriteMask = 0xFFFFFFFF,
            .depthBias = 0,
            .depthBiasSlopeScale = 0.0f,
            .depthBiasClamp = 0.0f,
        };

        WGPURenderPipelineDescriptor descriptor{
            .nextInChain = nullptr,
            .label = MakeStringView(createInfo.name),
            .layout = layout->handle,
            .vertex = WGPUVertexState{
                .nextInChain = nullptr,
                .module = ((IShaderModule*)vertexStage.module)->handle,
                .entryPoint = MakeStringView(vertexStage.name),
                .constantCount = 0,
                .constants = nullptr,
                .bufferCount = vertexBuffers.size(),
                .buffers = vertexBuffers.data(),
            },
            .primitive = WGPUPrimitiveState{
                .nextInChain = nullptr,
                .topology = ConvertTopology(createInfo.topologyMode),
                .stripIndexFormat = WGPUIndexFormat_Undefined,
                .frontFace = ConvertFrontFace(createInfo.rasterizationState.frontFace),
                .cullMode = ConvertCullMode(createInfo.rasterizationState.cullMode),
                .unclippedDepth = WGPU_FALSE,
            },
            .depthStencil = &depthStencil,
            .multisample = WGPUMultisampleState{
                .nextInChain = nullptr,
                .count = ConvertSampleCount(createInfo.multisampleState.sampleCount),
                .mask = 0xFFFFFFFF,
                .alphaToCoverageEnabled = WGPU_FALSE,
            },
            .fragment = &fragment,
        };
        handle = wgpuDeviceCreateRenderPipeline(device->m_device, &descriptor);
    }

    void IGraphicsPipeline::Shutdown(IDevice* device)
    {
        wgpuRenderPipelineRelease(handle);
        handle = nullptr;
        layout = nullptr;
    }

    void IComputePipeline::Init(IDevice* device, const ComputePipelineCreateInfo& createInfo)
    {
        layout = (IPipelineLayout*)createInfo.layout;
        auto* shader = (IShaderModule*)createInfo.computeShader.module;
        WGPUComputePipelineDescriptor descriptor{
            .nextInChain = nullptr,
            .label = MakeStringView(createInfo.name),
            .layout = layout->handle,
            .compute = WGPUComputeState{
                .nextInChain = nullptr,
                .module = shader->handle,
                .entryPoint = MakeStringView(createInfo.computeShader.name),
                .constantCount = 0,
                .constants = nullptr,
            },
        };
        handle = wgpuDeviceCreateComputePipeline(device->m_device, &descriptor);
    }

    void IComputePipeline::Shutdown(IDevice* device)
    {
        wgpuComputePipelineRelease(handle);
        handle = nullptr;
        layout = nullptr;
    }

    void IRayTracingPipeline::Init(IDevice* device, const RayTracingPipelineCreateInfo& createInfo)
    {
        // WebGPU does not expose ray tracing pipelines.
    }

    void IRayTracingPipeline::Shutdown(IDevice* device)
    {
    }

    void IRayTracingPipeline::GetShaderBindingTableEntry(IDevice* device, uint32_t group, size_t size, void* dstHandle)
    {
    }

    void IQueryPool::Init(IDevice* device, const QueryPoolCreateInfo& createInfo)
    {
        WGPUQueryType type;
        switch (createInfo.type)
        {
        case QueryType::Timestamp: type = WGPUQueryType_Timestamp; break;
        case QueryType::Occlusion: type = WGPUQueryType_Occlusion; break;
        default: TL_UNREACHABLE();
        }

        WGPUQuerySetDescriptor descriptor{
            .nextInChain = nullptr,
            .label = MakeStringView(createInfo.name),
            .type = type,
            .count = createInfo.count,
        };
        handle = wgpuDeviceCreateQuerySet(device->m_device, &descriptor);
    }

    void IQueryPool::Shutdown(IDevice* device)
    {
        wgpuQuerySetDestroy(handle);
        wgpuQuerySetRelease(handle);
        handle = nullptr;
    }

    void IBuffer::Init(IDevice* device, const BufferCreateInfo& createInfo)
    {
        WGPUBufferUsage usage = ConvertBufferUsage(createInfo.usageFlags);
        size = createInfo.byteSize;
        hostMapped = (bool)(createInfo.usageFlags & BufferUsage::HostMapped);
        WGPUBufferDescriptor descriptor{
            .nextInChain = nullptr,
            .label = MakeStringView(createInfo.name),
            .usage = usage,
            .size = createInfo.byteSize,
            .mappedAtCreation = hostMapped ? WGPU_TRUE : WGPU_FALSE,
        };
        handle = wgpuDeviceCreateBuffer(device->m_device, &descriptor);
    }

    void IBuffer::Shutdown(IDevice* device)
    {
        wgpuBufferDestroy(handle);
        wgpuBufferRelease(handle);
        handle = nullptr;
        size = 0;
        hostMapped = false;
    }

    DeviceMemoryPtr IBuffer::Map(IDevice* device)
    {
        // wgpuBufferMapAsync;
        return wgpuBufferGetMappedRange(handle, 0, size);
    }

    void IBuffer::Unmap(IDevice* device)
    {
        wgpuBufferUnmap(handle);
    }

    void IImage::Init(IDevice* device, const ImageCreateInfo& createInfo)
    {
        WGPUTextureFormat webGPUFormat = ConvertFormat(createInfo.format);
        WGPUTextureDimension dimension = ConvertImageType(createInfo.type);
        WGPUTextureUsage usage = ConvertImageUsage(createInfo.usageFlags);
        uint32_t sampleCount = ConvertSampleCount(createInfo.sampleCount);
        WGPUTextureDescriptor descriptor{
            .nextInChain = nullptr,
            .label = MakeStringView(createInfo.name),
            .usage = usage,
            .dimension = dimension,
            .size = WGPUExtent3D{
                .width = createInfo.size.width,
                .height = createInfo.size.height,
                .depthOrArrayLayers = dimension == WGPUTextureDimension_3D ? createInfo.size.depth : createInfo.arrayCount,
            },
            .format = webGPUFormat,
            .mipLevelCount = createInfo.mipLevels,
            .sampleCount = sampleCount,
            .viewFormatCount = 0,
            .viewFormats = nullptr,
        };
        handle = wgpuDeviceCreateTexture(device->m_device, &descriptor);
        WGPUTextureViewDescriptor viewDescriptor{
            .nextInChain = nullptr,
            .label = MakeStringView(createInfo.name),
            .format = webGPUFormat,
            .dimension = dimension == WGPUTextureDimension_1D
                             ? WGPUTextureViewDimension_1D
                         : dimension == WGPUTextureDimension_2D
                             ? (createInfo.arrayCount == 1 ? WGPUTextureViewDimension_2D : WGPUTextureViewDimension_2DArray)
                             : WGPUTextureViewDimension_3D,
            .baseMipLevel = 0,
            .mipLevelCount = createInfo.mipLevels,
            .baseArrayLayer = 0,
            .arrayLayerCount = dimension == WGPUTextureDimension_3D ? 1u : createInfo.arrayCount,
            .aspect = WGPUTextureAspect_All,
            .usage = usage,
        };
        viewHandle = wgpuTextureCreateView(handle, &viewDescriptor);
        ownsTexture = true;
        size = createInfo.size;
        format = createInfo.format;
        type = createInfo.type;
        arrayCount = createInfo.arrayCount;
        FormatInfo formatInfo = GetFormatInfo(format);
        subresources = ImageSubresourceRange{
            .imageAspects = formatInfo.hasDepth || formatInfo.hasStencil
                                ? (formatInfo.hasDepth && formatInfo.hasStencil ? ImageAspect::DepthStencil : formatInfo.hasDepth ? ImageAspect::Depth
                                                                                                                                  : ImageAspect::Stencil)
                                : ImageAspect::Color,
            .mipBase = 0,
            .mipLevelCount = (uint8_t)std::min(createInfo.mipLevels, (uint32_t)UINT8_MAX),
            .arrayBase = 0,
            .arrayCount = (uint8_t)std::min(createInfo.arrayCount, (uint32_t)UINT8_MAX),
        };
    }

    void IImage::Init(IDevice* device, const ImageViewCreateInfo& createInfo)
    {
        auto* source = (IImage*)createInfo.image;
        Format viewFormat = createInfo.format == Format::Unknown ? source->format : createInfo.format;
        WGPUTextureFormat webGPUFormat = ConvertFormat(viewFormat);
        WGPUTextureViewDimension dimension = ConvertImageViewType(createInfo.viewType);
        WGPUTextureComponentSwizzleDescriptor swizzle{
            .chain = WGPUChainedStruct{
                .next = nullptr,
                .sType = WGPUSType_TextureComponentSwizzleDescriptor,
            },
            .swizzle = WGPUTextureComponentSwizzle{
                .r = ConvertComponentSwizzle(createInfo.components.r),
                .g = ConvertComponentSwizzle(createInfo.components.g),
                .b = ConvertComponentSwizzle(createInfo.components.b),
                .a = ConvertComponentSwizzle(createInfo.components.a),
            },
        };
        WGPUTextureViewDescriptor descriptor{
            .nextInChain = IsIdentityMapping(createInfo.components) ? nullptr : &swizzle.chain,
            .label = MakeStringView(createInfo.name),
            .format = webGPUFormat,
            .dimension = dimension,
            .baseMipLevel = createInfo.subresource.mipBase,
            .mipLevelCount = createInfo.subresource.mipLevelCount == AllMipLevels
                                 ? WGPU_MIP_LEVEL_COUNT_UNDEFINED
                                 : createInfo.subresource.mipLevelCount,
            .baseArrayLayer = createInfo.subresource.arrayBase,
            .arrayLayerCount = createInfo.subresource.arrayCount == AllLayers
                                   ? WGPU_ARRAY_LAYER_COUNT_UNDEFINED
                                   : createInfo.subresource.arrayCount,
            .aspect = ConvertImageAspect(createInfo.subresource.imageAspects),
            .usage = WGPUTextureUsage_None,
        };
        viewHandle = wgpuTextureCreateView(source->handle, &descriptor);
        handle = source->handle;
        wgpuTextureAddRef(handle);
        ownsTexture = false;
        size = source->size;
        format = viewFormat;
        type = source->type;
        arrayCount = source->arrayCount;
        subresources = createInfo.subresource;
    }

    void IImage::Shutdown(IDevice* device)
    {
        wgpuTextureViewRelease(viewHandle);
        if (ownsTexture)
            wgpuTextureDestroy(handle);
        wgpuTextureRelease(handle);
        viewHandle = nullptr;
        handle = nullptr;
        ownsTexture = false;
        type = ImageType::None;
        arrayCount = 1;
    }

    void ISampler::Init(IDevice* device, const SamplerCreateInfo& createInfo)
    {
        WGPUSamplerDescriptor descriptor{
            .nextInChain = nullptr,
            .label = MakeStringView(createInfo.name),
            .addressModeU = ConvertAddressMode(createInfo.addressU),
            .addressModeV = ConvertAddressMode(createInfo.addressV),
            .addressModeW = ConvertAddressMode(createInfo.addressW),
            .magFilter = ConvertFilter(createInfo.filterMag),
            .minFilter = ConvertFilter(createInfo.filterMin),
            .mipmapFilter = ConvertMipmapFilter(createInfo.filterMip),
            .lodMinClamp = createInfo.minLod,
            .lodMaxClamp = createInfo.maxLod,
            .compare = ConvertCompare(createInfo.compare),
            .maxAnisotropy = 1,
        };
        handle = wgpuDeviceCreateSampler(device->m_device, &descriptor);
    }

    void ISampler::Shutdown(IDevice* device)
    {
        wgpuSamplerRelease(handle);
        handle = nullptr;
    }

    void IAccelerationStructure::Init(IDevice* device, const AccelerationStructureCreateInfo& createInfo)
    {
        // WebGPU does not expose acceleration structures.
    }

    void IAccelerationStructure::Shutdown(IDevice* device)
    {
    }

    void IMicromap::Init(IDevice* device, const MicromapCreateInfo& createInfo)
    {
        // WebGPU does not expose micromaps.
    }

    void IMicromap::Shutdown(IDevice* device)
    {
    }

    void ISwapchain::Init(IDevice* device, const SwapchainCreateInfo& createInfo)
    {
#if defined(__EMSCRIPTEN__)
        WGPUEmscriptenSurfaceSourceCanvasHTMLSelector source{
            .chain = WGPUChainedStruct{
                .next = nullptr,
                .sType = WGPUSType_EmscriptenSurfaceSourceCanvasHTMLSelector,
            },
            .selector = MakeStringView(createInfo.htmlCanvasSelector),
        };
        WGPUSurfaceDescriptor descriptor{
            .nextInChain = &source.chain,
            .label = MakeStringView(createInfo.name),
        };
        surface = wgpuInstanceCreateSurface(device->m_instance, &descriptor);
#else
        // TODO: Create the surface from the desktop platform descriptor.
        std::abort();
#endif
    }

    void ISwapchain::Shutdown(IDevice* device)
    {
        image.Shutdown(device);
        surfaceTexture = WGPUSurfaceTexture{
            .nextInChain = nullptr,
            .texture = nullptr,
            .status = WGPUSurfaceGetCurrentTextureStatus_Error,
        };
        wgpuSurfaceUnconfigure(surface);
        wgpuSurfaceRelease(surface);
        surface = nullptr;
    }

    uint32_t ISwapchain::GetImagesCount() const
    {
        return configuration.imageCount;
    }

    SwapchainAcquireResult ISwapchain::AcquireSwapchainImage()
    {
        if (image.viewHandle)
            image.Shutdown(nullptr);
        surfaceTexture = WGPUSurfaceTexture{
            .nextInChain = nullptr,
            .texture = nullptr,
            .status = WGPUSurfaceGetCurrentTextureStatus_Error,
        };
        wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);
        image.handle = surfaceTexture.texture;
        image.ownsTexture = false;
        image.size = ImageSize3D{configuration.size.width, configuration.size.height, 1};
        image.format = configuration.format;
        image.type = ImageType::Image2D;
        image.arrayCount = 1;
        image.subresources = ImageSubresourceRange{ImageAspect::Color, 0, 1, 0, 1};
        image.viewHandle = wgpuTextureCreateView(image.handle, nullptr);
        return SwapchainAcquireResult{
            .image = &image,
            .fence = nullptr,
        };
    }

    SurfaceCapabilities ISwapchain::GetSurfaceCapabilities(IDevice* device) const
    {
        SurfaceCapabilities result{
            .minImageSize = {1, 1},
            .maxImageSize = {UINT32_MAX, UINT32_MAX},
            .minImageCount = 1,
            .maxImageCount = 3,
            .usages = ImageUsage::None,
            .presentModes = SwapchainPresentMode::None,
            .alphaModes = SwapchainAlphaMode::None,
            .formats = {},
        };
        WGPUSurfaceCapabilities capabilities{
            .nextInChain = nullptr,
            .usages = WGPUTextureUsage_None,
            .formatCount = 0,
            .formats = nullptr,
            .presentModeCount = 0,
            .presentModes = nullptr,
            .alphaModeCount = 0,
            .alphaModes = nullptr,
        };
        wgpuSurfaceGetCapabilities(surface, device->m_adapter, &capabilities);

        if (capabilities.usages & WGPUTextureUsage_TextureBinding) result.usages |= ImageUsage::ShaderResource;
        if (capabilities.usages & WGPUTextureUsage_StorageBinding) result.usages |= ImageUsage::StorageResource;
        if (capabilities.usages & WGPUTextureUsage_RenderAttachment) result.usages |= ImageUsage::Color;
        if (capabilities.usages & WGPUTextureUsage_CopySrc) result.usages |= ImageUsage::CopySrc;
        if (capabilities.usages & WGPUTextureUsage_CopyDst) result.usages |= ImageUsage::CopyDst;
        for (size_t index = 0; index < capabilities.formatCount; ++index)
        {
            Format format = ConvertFormat(capabilities.formats[index]);
            result.formats.push_back(format);
        }
        for (size_t index = 0; index < capabilities.presentModeCount; ++index)
        {
            switch (capabilities.presentModes[index])
            {
            case WGPUPresentMode_Immediate: result.presentModes |= SwapchainPresentMode::Immediate; break;
            case WGPUPresentMode_Fifo: result.presentModes |= SwapchainPresentMode::Fifo; break;
            case WGPUPresentMode_FifoRelaxed: result.presentModes |= SwapchainPresentMode::FifoRelaxed; break;
            case WGPUPresentMode_Mailbox: result.presentModes |= SwapchainPresentMode::Mailbox; break;
            default: break;
            }
        }
        for (size_t index = 0; index < capabilities.alphaModeCount; ++index)
        {
            switch (capabilities.alphaModes[index])
            {
            case WGPUCompositeAlphaMode_Premultiplied: result.alphaModes |= SwapchainAlphaMode::PreMultiplied; break;
            case WGPUCompositeAlphaMode_Unpremultiplied: result.alphaModes |= SwapchainAlphaMode::PostMultiplied; break;
            default: break;
            }
        }
        wgpuSurfaceCapabilitiesFreeMembers(capabilities);
        return result;
    }

    ResultCode ISwapchain::ResizeSwapchain(IDevice* device, const ImageSize2D& size)
    {
        SwapchainConfigureInfo resized = configuration;
        resized.size = size;
        return ConfigureSwapchain(device, resized);
    }

    ResultCode ISwapchain::ConfigureSwapchain(IDevice* device, const SwapchainConfigureInfo& configInfo)
    {
        WGPUTextureFormat format = ConvertFormat(configInfo.format);
        WGPUTextureUsage usage = ConvertImageUsage(configInfo.imageUsage);
        WGPUPresentMode presentMode = WGPUPresentMode_Fifo;
        switch (configInfo.presentMode)
        {
        case SwapchainPresentMode::Immediate: presentMode = WGPUPresentMode_Immediate; break;
        case SwapchainPresentMode::FifoRelaxed: presentMode = WGPUPresentMode_FifoRelaxed; break;
        case SwapchainPresentMode::Mailbox: presentMode = WGPUPresentMode_Mailbox; break;
        default: break;
        }
        WGPUCompositeAlphaMode alphaMode = WGPUCompositeAlphaMode_Auto;
        if (configInfo.alphaMode == SwapchainAlphaMode::PreMultiplied) alphaMode = WGPUCompositeAlphaMode_Premultiplied;
        if (configInfo.alphaMode == SwapchainAlphaMode::PostMultiplied) alphaMode = WGPUCompositeAlphaMode_Unpremultiplied;

        WGPUSurfaceConfiguration surfaceConfig{
            .nextInChain = nullptr,
            .device = device->m_device,
            .format = format,
            .usage = usage,
            .width = configInfo.size.width,
            .height = configInfo.size.height,
            .viewFormatCount = 0,
            .viewFormats = nullptr,
            .alphaMode = alphaMode,
            .presentMode = presentMode,
        };
        wgpuSurfaceConfigure(surface, &surfaceConfig);
        configuration = configInfo;
        return ResultCode::Success;
    }

} // namespace RHI::WebGPU
