#include "Resources.hpp"

#include <ranges>
#include <wrl.h>

#include "Common.hpp"
#include "Device.hpp"

// Complete Resources
// - [] IBindGroupLayout
// - [] IBindGroup
// - [] IShaderModule
// - [] IPipelineLayout
// - [] IGraphicsPipeline
// - [] IComputePipeline
// - [] IBuffer
// - [] IImage
// - [] ISampler

namespace RHI::D3D12
{
    using Microsoft::WRL::ComPtr;

    inline static D3D12_RESOURCE_FLAGS ConvertToResourceFlags(TL::Flags<BufferUsage> flags)
    {
        D3D12_RESOURCE_FLAGS result = D3D12_RESOURCE_FLAG_NONE;
        if (flags & BufferUsage::Storage) result |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        if (flags & BufferUsage::Uniform) result |= D3D12_RESOURCE_FLAG_NONE;
        if (flags & BufferUsage::Vertex) result |= D3D12_RESOURCE_FLAG_NONE;
        if (flags & BufferUsage::Index) result |= D3D12_RESOURCE_FLAG_NONE;
        if (flags & BufferUsage::CopySrc) result |= D3D12_RESOURCE_FLAG_NONE;
        if (flags & BufferUsage::CopyDst) result |= D3D12_RESOURCE_FLAG_NONE;
        if (flags & BufferUsage::Indirect) result |= D3D12_RESOURCE_FLAG_NONE;
        return result;
    }

    inline static D3D12_RESOURCE_FLAGS ConvertToResourceFlags(TL::Flags<ImageUsage> flags)
    {
        D3D12_RESOURCE_FLAGS result = D3D12_RESOURCE_FLAG_NONE;
        if (!(flags & ImageUsage::ShaderResource)) result |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        if (flags & ImageUsage::StorageResource) result |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        if (flags & ImageUsage::Color) result |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        if (flags & ImageUsage::Depth) result |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        if (flags & ImageUsage::Stencil) result |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        if (flags & ImageUsage::DepthStencil) result |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        if (flags & ImageUsage::CopySrc) result |= D3D12_RESOURCE_FLAG_NONE;
        if (flags & ImageUsage::CopyDst) result |= D3D12_RESOURCE_FLAG_NONE;
        return result;
    }

    inline static D3D12_RESOURCE_DIMENSION ConvertToResourceDimension(ImageType imageType)
    {
        switch (imageType)
        {
        case ImageType::Image1D: return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        case ImageType::Image2D: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        case ImageType::Image3D: return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        default:                 return D3D12_RESOURCE_DIMENSION_UNKNOWN;
        }
    }

    inline static UINT8 ConvertToWriteMask(TL::Flags<ColorWriteMask> mask)
    {
        UINT8 result = 0;
        if (mask & ColorWriteMask::Red) result |= D3D12_COLOR_WRITE_ENABLE_RED;
        if (mask & ColorWriteMask::Green) result |= D3D12_COLOR_WRITE_ENABLE_GREEN;
        if (mask & ColorWriteMask::Blue) result |= D3D12_COLOR_WRITE_ENABLE_BLUE;
        if (mask & ColorWriteMask::Alpha) result |= D3D12_COLOR_WRITE_ENABLE_ALPHA;
        return result;
    }

    inline static D3D12_FILTER ConvertToFilter(SamplerFilter minFilter, SamplerFilter magFilter, SamplerFilter mipFilter)
    {
        bool min = minFilter == SamplerFilter::Linear;
        bool mag = magFilter == SamplerFilter::Linear;
        bool mip = mipFilter == SamplerFilter::Linear;
        if (min && mag && mip) return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        if (min && mag && !mip) return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        if (min && !mag && mip) return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        if (min && !mag && !mip) return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        if (!min && mag && mip) return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        if (!min && mag && !mip) return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        if (!min && !mag && mip) return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        return D3D12_FILTER_MIN_MAG_MIP_POINT;
    }

    inline static D3D12_TEXTURE_ADDRESS_MODE ConvertToAddressMode(SamplerAddressMode addressMode)
    {
        switch (addressMode)
        {
        case SamplerAddressMode::Repeat: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case SamplerAddressMode::Clamp:  return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        default:                         return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
    }

    inline static D3D12_COMPARISON_FUNC ConvertToCompareFunction(CompareOperator compareOp)
    {
        switch (compareOp)
        {
        case CompareOperator::Never:          return D3D12_COMPARISON_FUNC_NEVER;
        case CompareOperator::Equal:          return D3D12_COMPARISON_FUNC_EQUAL;
        case CompareOperator::NotEqual:       return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case CompareOperator::Greater:        return D3D12_COMPARISON_FUNC_GREATER;
        case CompareOperator::GreaterOrEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case CompareOperator::Less:           return D3D12_COMPARISON_FUNC_LESS;
        case CompareOperator::LessOrEqual:    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case CompareOperator::Always:         return D3D12_COMPARISON_FUNC_ALWAYS;
        default:                              TL_UNREACHABLE();
        }
        return D3D12_COMPARISON_FUNC_NEVER;
    }

    inline static UINT ConvertToSampleCount(SampleCount sampleCount)
    {
        switch (sampleCount)
        {
        case SampleCount::Samples1:  return 1;
        case SampleCount::Samples2:  return 2;
        case SampleCount::Samples4:  return 4;
        case SampleCount::Samples8:  return 8;
        case SampleCount::Samples16: return 16;
        case SampleCount::Samples32: return 32;
        case SampleCount::Samples64: return 64;
        default:                     return 1;
        }
    }

    inline static D3D12_PRIMITIVE_TOPOLOGY_TYPE ConvertPrimitiveTopologyType(PipelineTopologyMode topologyMode)
    {
        switch (topologyMode)
        {
        case PipelineTopologyMode::Points:    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
        case PipelineTopologyMode::Lines:     return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        case PipelineTopologyMode::Triangles: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        default:                              return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
        }
    }

    inline static D3D12_COMPARISON_FUNC ConvertComparisonFunc(CompareOperator compareOperator)
    {
        switch (compareOperator)
        {
        case CompareOperator::Never:          return D3D12_COMPARISON_FUNC_NEVER;
        case CompareOperator::Equal:          return D3D12_COMPARISON_FUNC_EQUAL;
        case CompareOperator::NotEqual:       return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case CompareOperator::Less:           return D3D12_COMPARISON_FUNC_LESS;
        case CompareOperator::LessOrEqual:    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case CompareOperator::Greater:        return D3D12_COMPARISON_FUNC_GREATER;
        case CompareOperator::GreaterOrEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case CompareOperator::Always:         return D3D12_COMPARISON_FUNC_ALWAYS;
        default:                              return D3D12_COMPARISON_FUNC_NEVER;
        }
    }

    // Add these conversion functions at the top with the other conversions:
    inline static D3D12_FILL_MODE ConvertToFillMode(PipelineRasterizerStateFillMode fillMode)
    {
        switch (fillMode)
        {
        case PipelineRasterizerStateFillMode::Point:    return D3D12_FILL_MODE_WIREFRAME; // D3D12 doesn't have point fill mode
        case PipelineRasterizerStateFillMode::Line:     return D3D12_FILL_MODE_WIREFRAME;
        case PipelineRasterizerStateFillMode::Triangle: return D3D12_FILL_MODE_SOLID;
        default:                                        return D3D12_FILL_MODE_SOLID;
        }
    }

    inline static D3D12_CULL_MODE ConvertToCullMode(PipelineRasterizerStateCullMode cullMode)
    {
        switch (cullMode)
        {
        case PipelineRasterizerStateCullMode::None:      return D3D12_CULL_MODE_NONE;
        case PipelineRasterizerStateCullMode::FrontFace: return D3D12_CULL_MODE_FRONT;
        case PipelineRasterizerStateCullMode::BackFace:  return D3D12_CULL_MODE_BACK;
        case PipelineRasterizerStateCullMode::Discard:   return D3D12_CULL_MODE_NONE;
        default:                                         return D3D12_CULL_MODE_NONE;
        }
    }

    inline static D3D12_BLEND_OP ConvertBlendOp(BlendEquation blendOp)
    {
        switch (blendOp)
        {
        case BlendEquation::Add:             return D3D12_BLEND_OP_ADD;
        case BlendEquation::Subtract:        return D3D12_BLEND_OP_SUBTRACT;
        case BlendEquation::ReverseSubtract: return D3D12_BLEND_OP_REV_SUBTRACT;
        case BlendEquation::Min:             return D3D12_BLEND_OP_MIN;
        case BlendEquation::Max:             return D3D12_BLEND_OP_MAX;
        default:                             return D3D12_BLEND_OP_ADD;
        }
    }

    inline static D3D12_BLEND ConvertBlendFactor(BlendFactor blendFactor)
    {
        switch (blendFactor)
        {
        case BlendFactor::Zero:             return D3D12_BLEND_ZERO;
        case BlendFactor::One:              return D3D12_BLEND_ONE;
        case BlendFactor::SrcColor:         return D3D12_BLEND_SRC_COLOR;
        case BlendFactor::OneMinusSrcColor: return D3D12_BLEND_INV_SRC_COLOR;
        case BlendFactor::DstColor:         return D3D12_BLEND_DEST_COLOR;
        case BlendFactor::OneMinusDstColor: return D3D12_BLEND_INV_DEST_COLOR;
        case BlendFactor::SrcAlpha:         return D3D12_BLEND_SRC_ALPHA;
        case BlendFactor::OneMinusSrcAlpha: return D3D12_BLEND_INV_SRC_ALPHA;
        case BlendFactor::DstAlpha:         return D3D12_BLEND_DEST_ALPHA;
        case BlendFactor::OneMinusDstAlpha: return D3D12_BLEND_INV_DEST_ALPHA;
        default:                            return D3D12_BLEND_ZERO;
        }
    }

    inline static D3D12_SRV_DIMENSION ConvertToSrvDimension(ImageViewType type)
    {
        switch (type)
        {
        case ImageViewType::View1D:      return D3D12_SRV_DIMENSION_TEXTURE1D;
        case ImageViewType::View1DArray: return D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
        case ImageViewType::View2D:      return D3D12_SRV_DIMENSION_TEXTURE2D;
        case ImageViewType::View2DArray: return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        case ImageViewType::View3D:      return D3D12_SRV_DIMENSION_TEXTURE3D;
        case ImageViewType::CubeMap:     return D3D12_SRV_DIMENSION_TEXTURECUBE;
        default:                         TL_UNREACHABLE();
        }
        return {};
    }

    inline static D3D12_UAV_DIMENSION ConvertToUavDimension(ImageViewType type)
    {
        switch (type)
        {
        case ImageViewType::View1D:      return D3D12_UAV_DIMENSION_TEXTURE1D;
        case ImageViewType::View1DArray: return D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
        case ImageViewType::View2D:      return D3D12_UAV_DIMENSION_TEXTURE2D;
        case ImageViewType::View2DArray: return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        case ImageViewType::View3D:      return D3D12_UAV_DIMENSION_TEXTURE3D;
        default:                         TL_UNREACHABLE();
        }
        return {};
    }

    //////

    inline static D3D12_SHADER_RESOURCE_VIEW_DESC CreateSrvForImage(Format format, ImageViewType type, ImageSubresourceRange range)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc{
            .Format        = ConvertToFormat(format),
            .ViewDimension = ConvertToSrvDimension(type),
            .Shader4ComponentMapping =
                D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0,
                    D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1,
                    D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2,
                    D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3),
        };
        switch (type)
        {
        case ImageViewType::View1D:
            desc.Texture1D.MostDetailedMip = range.mipBase;
            desc.Texture1D.MipLevels       = range.mipLevelCount;
            break;
        case ImageViewType::View1DArray:
            desc.Texture1DArray.MostDetailedMip = range.mipBase;
            desc.Texture1DArray.MipLevels       = range.mipLevelCount;
            desc.Texture1DArray.FirstArraySlice = range.arrayBase;
            desc.Texture1DArray.ArraySize       = range.arrayCount;
            break;
        case ImageViewType::View2D:
            desc.Texture2D.MostDetailedMip = range.mipBase;
            desc.Texture2D.MipLevels       = range.mipLevelCount;
            // desc.Texture2D.ResourceMinLODClamp;
            break;
        case ImageViewType::View2DArray:
            desc.Texture2DArray.MostDetailedMip = range.mipBase;
            desc.Texture2DArray.MipLevels       = range.mipLevelCount;
            desc.Texture2DArray.FirstArraySlice = range.arrayBase;
            desc.Texture2DArray.ArraySize       = range.arrayCount;
            // desc.Texture2DArray.PlaneSlice      = {};
            break;
        case ImageViewType::View3D:
            desc.Texture3D.MostDetailedMip = range.mipBase;
            desc.Texture3D.MipLevels       = range.mipLevelCount;
            // desc.Texture3D.ResourceMinLODClamp;
            break;
        case ImageViewType::CubeMap:
            desc.TextureCube.MostDetailedMip = range.mipBase;
            desc.TextureCube.MipLevels       = range.mipLevelCount;
            // desc.TextureCube.ResourceMinLODClamp;
            break;
        default: TL_UNREACHABLE();
        }
        return desc;
    }

    inline static D3D12_UNORDERED_ACCESS_VIEW_DESC CreateUavForImage(Format format, ImageViewType type, ImageSubresourceRange range)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc{
            .Format        = ConvertToFormat(format),
            .ViewDimension = ConvertToUavDimension(type),
        };
        switch (type)
        {
        case ImageViewType::View1D:
            desc.Texture1D.MipSlice = range.mipBase;
            break;
        case ImageViewType::View1DArray:
            desc.Texture1DArray.MipSlice        = range.mipBase;
            desc.Texture1DArray.FirstArraySlice = range.arrayBase;
            desc.Texture1DArray.ArraySize       = range.arrayCount;
            break;
        case ImageViewType::View2D:
            desc.Texture2D.MipSlice = range.mipBase;
            // desc.Texture2D.PlaneSlice;
            break;
        case ImageViewType::View2DArray:
            desc.Texture2DArray.MipSlice        = range.mipBase;
            desc.Texture2DArray.FirstArraySlice = range.arrayBase;
            desc.Texture2DArray.ArraySize       = range.arrayCount;
            // desc.Texture1DArray.PlaneSlice;
            break;
        case ImageViewType::View3D:
            desc.Texture3D.MipSlice = range.mipBase;
            // desc.Texture3D.ResourceMinLODClamp;
            break;
        default: TL_UNREACHABLE();
        }
        return desc;
    }

    inline static D3D12_SHADER_RESOURCE_VIEW_DESC CreateSrvForBuffer(Buffer* buffer, BufferSubregion subregion)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc{
            .Format                  = DXGI_FORMAT_UNKNOWN,
            .ViewDimension           = D3D12_SRV_DIMENSION_BUFFER,
            .Shader4ComponentMapping = {},
            .Buffer                  = {
                                        .FirstElement        = 0,
                                        .NumElements         = 0,
                                        .StructureByteStride = {},
                                        .Flags               = {},
                                        },
        };
        return desc;
    }

    inline static D3D12_UNORDERED_ACCESS_VIEW_DESC CreateUavForBuffer(Buffer* buffer, BufferSubregion subregion)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc{
            .Format        = DXGI_FORMAT_UNKNOWN,
            .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
            .Buffer        = {
                              .FirstElement        = 0,
                              .NumElements         = 0,
                              .StructureByteStride = {},
                              .Flags               = {},
                              },
        };
        return desc;
    }

    inline static D3D12_CONSTANT_BUFFER_VIEW_DESC CreateCbvForBuffer(IBuffer* buffer, BufferSubregion subregion)
    {
        return {
            .BufferLocation = buffer->resource->GetGPUVirtualAddress() + subregion.offset,
            .SizeInBytes    = (UINT)subregion.size,
        };
    }

    ////////////////////////////////////////////////////////////////////////
    // DescriptorPool
    ////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////
    // IBindGroupLayout
    ////////////////////////////////////////////////////////////////////////

    ResultCode IBindGroupLayout::Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo)
    {
        return ResultCode::Success;
    }

    void IBindGroupLayout::Shutdown([[maybe_unused]] IDevice* device)
    {
    }

    ShaderViewType IBindGroupLayout::GetBindingInfo(int)
    {
        return ShaderViewType::UnorederedAccessView;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE IBindGroupLayout::GetHandleGPU(uint32_t binding, uint32_t arrayIndex) const
    {
        return {};
    }

    D3D12_CPU_DESCRIPTOR_HANDLE IBindGroupLayout::GetHandleCPU(uint32_t binding, uint32_t arrayIndex) const
    {
        return {};
    }

    ////////////////////////////////////////////////////////////////////////
    // IBindGroup
    ////////////////////////////////////////////////////////////////////////

    ResultCode IBindGroup::Init(IDevice* device, const BindGroupCreateInfo& createInfo)
    {
        return ResultCode::Success;
    }

    void IBindGroup::Shutdown([[maybe_unused]] IDevice* device)
    {
    }

    void IBindGroup::Update(IDevice* device, const BindGroupUpdateInfo& updateInfo)
    {
        // ZoneScoped;

        auto layout    = device->m_bindGroupLayoutsOwner.Get(this->layout);
        auto d3ddevice = device->m_device;

        for (auto imageBinding : updateInfo.images)
        {
            for (uint32_t i = 0; i < imageBinding.images.size(); i++)
            {
                auto image     = device->m_imageOwner.Get(imageBinding.images[i]);
                auto cpuHandle = layout->GetHandleCPU(imageBinding.dstBinding, imageBinding.dstArrayElement + i);
                switch (layout->GetBindingInfo(imageBinding.dstBinding))
                {
                case ShaderViewType::ShaderResourceView:
                    {
                        auto desc = CreateSrvForImage(image->format, {}, image->subresoruceRange);
                        d3ddevice->CreateShaderResourceView(image->resource, &desc, cpuHandle);
                        break;
                    }
                case ShaderViewType::UnorederedAccessView:
                    {
                        auto desc = CreateUavForImage(image->format, {}, image->subresoruceRange);
                        d3ddevice->CreateUnorderedAccessView(image->resource, nullptr, &desc, cpuHandle);
                        break;
                    }
                case ShaderViewType::Constant:
                case ShaderViewType::Sampler:
                case ShaderViewType::Count:
                    TL_UNREACHABLE();
                }
            }
        }
        for (auto bufferBinding : updateInfo.buffers)
        {
            for (uint32_t i = 0; i < bufferBinding.buffers.size(); i++)
            {
                auto buffer    = device->m_bufferOwner.Get(bufferBinding.buffers[i]);
                auto cpuHandle = layout->GetHandleCPU(bufferBinding.dstBinding, bufferBinding.dstArrayElement + i);

                switch (layout->GetBindingInfo(bufferBinding.dstBinding))
                {
                case ShaderViewType::Constant:
                    {
                        auto desc = CreateCbvForBuffer(buffer, {});
                        d3ddevice->CreateConstantBufferView(&desc, cpuHandle);
                        break;
                    }
                case ShaderViewType::ShaderResourceView:
                    {
                        auto desc = CreateSrvForBuffer(buffer, {});
                        d3ddevice->CreateShaderResourceView(buffer->resource, &desc, cpuHandle);
                        break;
                    }
                case ShaderViewType::UnorederedAccessView:
                    {
                        auto desc = CreateUavForBuffer(buffer, {});
                        d3ddevice->CreateUnorderedAccessView(buffer->resource, nullptr /* TODO */, &desc, cpuHandle);
                        break;
                    }
                case ShaderViewType::Sampler:
                case ShaderViewType::Count:
                    TL_UNREACHABLE();
                }
            }
        }
        for (auto samplerBinding : updateInfo.samplers)
        {
            for (uint32_t i = 0; i < samplerBinding.samplers.size(); i++)
            {
                auto sampler   = device->m_samplerOwner.Get(samplerBinding.samplers[i]);
                auto cpuHandle = layout->GetHandleCPU(samplerBinding.dstBinding, samplerBinding.dstArrayElement + i);
                // TODO: Figure out
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////
    // IShaderModule
    ////////////////////////////////////////////////////////////////////////

    D3D12_SHADER_BYTECODE IShaderModule::GetShaderBytecode() const
    {
        return {.pShaderBytecode = code.data(), .BytecodeLength = code.size()};
    }

    ResultCode IShaderModule::Init(IDevice* device, const ShaderModuleCreateInfo& createInfo)
    {
        code.resize(createInfo.code.size_bytes());
        memcpy(code.data(), createInfo.code.data(), createInfo.code.size_bytes());
        return ResultCode::Success;
    }

    void IShaderModule::Shutdown()
    {
    }

    ////////////////////////////////////////////////////////////////////////
    // IPipelineLayout
    ////////////////////////////////////////////////////////////////////////

    ResultCode IPipelineLayout::Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo)
    {
        uint32_t registerSpace  = 0;
        uint32_t uavBinding     = 0;
        uint32_t srvBinding     = 0;
        uint32_t cbvBinding     = 0;
        uint32_t samplerBinding = 0;

        constexpr auto BindGroupCountMax = 4;

        TL::Vector<D3D12_ROOT_PARAMETER1>  parameters;
        TL::Vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges[BindGroupCountMax];
        TL::Vector<D3D12_DESCRIPTOR_RANGE> samplerDescriptorRanges[BindGroupCountMax];

        for (int bindGroupLayoutIndex = 0; bindGroupLayoutIndex < createInfo.layouts.size(); bindGroupLayoutIndex++)
        {
            auto* layout = (IBindGroupLayout*)createInfo.layouts[bindGroupLayoutIndex];

            for (auto bufferBinding : layout->bufferBindings)
            {
                D3D12_DESCRIPTOR_RANGE_TYPE type;
                if (bufferBinding.type == BindingType::UniformBuffer || bufferBinding.type == BindingType::DynamicUniformBuffer)
                {
                    type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                }
                else
                {
                    if (bufferBinding.access == Access::Read)
                        type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                    else
                        type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                }
                D3D12_DESCRIPTOR_RANGE range{
                    .RangeType                         = type,
                    .NumDescriptors                    = bufferBinding.arrayCount,
                    .BaseShaderRegister                = 0, // TODO!
                    .RegisterSpace                     = 0, // TODO!
                    .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
                };
                descriptorRanges[bindGroupLayoutIndex].push_back(range);
            }
            for (auto imageBinding : layout->imageBindings)
            {
                D3D12_DESCRIPTOR_RANGE_TYPE type;

                if (imageBinding.access == Access::Read)
                    type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                else
                    type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;

                D3D12_DESCRIPTOR_RANGE range{
                    .RangeType                         = type,
                    .NumDescriptors                    = imageBinding.arrayCount,
                    .BaseShaderRegister                = 0, // TODO!
                    .RegisterSpace                     = 0, // TODO!
                    .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
                };
                descriptorRanges[bindGroupLayoutIndex].push_back(range);
            }

            D3D12_ROOT_PARAMETER1 parameter{
                .ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .DescriptorTable  = {
                    .NumDescriptorRanges = (UINT)descriptorRanges[bindGroupLayoutIndex].size(),
                    .pDescriptorRanges   = (D3D12_DESCRIPTOR_RANGE1*)descriptorRanges[bindGroupLayoutIndex].data(),
                },
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL,
            };

            parameters.push_back(parameter);

            for (auto samplerBinding : layout->samplerBindings)
            {
                D3D12_DESCRIPTOR_RANGE range{
                    .RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
                    .NumDescriptors                    = samplerBinding.arrayCount,
                    .BaseShaderRegister                = 0, // TODO!
                    .RegisterSpace                     = 0, // TODO!
                    .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
                };
                samplerDescriptorRanges[bindGroupLayoutIndex].push_back(range);
            }
        }

        D3D12_ROOT_SIGNATURE_DESC1 desc{
            .NumParameters     = (UINT)parameters.size(),
            .pParameters       = parameters.data(),
            .NumStaticSamplers = 0,
            .pStaticSamplers   = nullptr,
            .Flags             = D3D12_ROOT_SIGNATURE_FLAG_NONE,
        };

        D3D12_VERSIONED_ROOT_SIGNATURE_DESC verDesc{
            .Version  = D3D_ROOT_SIGNATURE_VERSION_1_1,
            .Desc_1_1 = desc,
        };
        ComPtr<ID3DBlob> signatureBlob, errorBlob;
        if (auto hr = D3D12SerializeVersionedRootSignature(&verDesc, &signatureBlob, &errorBlob); FAILED(hr))
        {
            const char* errc = (const char*)errorBlob->GetBufferPointer();
            TL_LOG_ERROR("Failed to create RootSignature: {}", errc);
        }

        device->m_device->CreateRootSignature(
            0,
            signatureBlob->GetBufferPointer(),
            signatureBlob->GetBufferSize(),
            IID_PPV_ARGS(&this->rootSignature));

        return ResultCode::Success;
    }

    void IPipelineLayout::Shutdown([[maybe_unused]] IDevice* device)
    {
        rootSignature->Release();
    }

    ////////////////////////////////////////////////////////////////////////
    // IGraphicsPipeline
    ////////////////////////////////////////////////////////////////////////

    ResultCode IGraphicsPipeline::Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo)
    {
        auto* pipelineLayout = (IPipelineLayout*)createInfo.layout;
        this->layout         = pipelineLayout;

        IShaderModule* VS = nullptr;
        IShaderModule* PS = nullptr;
        for (auto& stage : createInfo.shaderStages)
        {
            if (stage.stage == ShaderStage::Vertex)  VS = (IShaderModule*)stage.module;
            if (stage.stage == ShaderStage::Pixel)   PS = (IShaderModule*)stage.module;
        }

        UINT        rtvFormatCount = 0;
        DXGI_FORMAT rtvFormats[8]  = {};
        DXGI_FORMAT dsvFormat      = ConvertToFormat(createInfo.renderTargetLayout.depthAttachmentFormat);
        for (auto format : createInfo.renderTargetLayout.colorAttachmentsFormats)
            rtvFormats[rtvFormatCount++] = ConvertToFormat(format);

        TL::Vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
        TL::Vector<UINT>                     bindingStrides;
        uint32_t                             bindingIndex = 0;
        for (const auto& bindingDesc : createInfo.vertexBufferBindings)
        {
            bindingStrides.push_back(bindingDesc.stride);

            for (const auto& attributeDesc : bindingDesc.attributes)
            {
                inputElements.push_back(D3D12_INPUT_ELEMENT_DESC{
                    .SemanticName         = nullptr, // TODO
                    .SemanticIndex        = 0,       // TODO
                    .Format               = ConvertToFormat(attributeDesc.format),
                    .InputSlot            = bindingIndex,
                    .AlignedByteOffset    = attributeDesc.offset,
                    .InputSlotClass       = bindingDesc.stepRate == PipelineVertexInputRate::PerVertex ? D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA : D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA,
                    .InstanceDataStepRate = 0, // TODO
                });
            }
        }

        ///
        TL::Vector<D3D12_SO_DECLARATION_ENTRY> streamOutputDeclarations;
        TL::Vector<UINT>                       streamOutputBufferStrides;
        D3D12_RENDER_TARGET_BLEND_DESC         rtBlendDescs[8] = {};
        for (size_t i = 0; i < createInfo.colorBlendState.blendStates.size(); i++)
        {
            const auto& streamOutputDesc = createInfo.colorBlendState.blendStates[i];

            // rtBlendDescs[i] = D3D12_RENDER_TARGET_BLEND_DESC{
            //     .BlendEnable           = streamOutputDesc.blendEnable,
            //     .RenderTargetWriteMask = ConvertToWriteMask(streamOutputDesc.writeMask),
            //     .BlendOp               = ConvertBlendOp(streamOutputDesc.colorBlendOp),
            //     .BlendOpAlpha          = ConvertBlendOp(streamOutputDesc.alphaBlendOp),
            //     .SrcBlend              = ConvertBlendFactor(streamOutputDesc.srcColor),
            //     .DestBlend             = ConvertBlendFactor(streamOutputDesc.dstColor),
            //     .SrcBlendAlpha         = ConvertBlendFactor(streamOutputDesc.srcAlpha),
            //     .DestBlendAlpha        = ConvertBlendFactor(streamOutputDesc.dstAlpha),
            // };
        }
        ///

        // Then update the graphics pipeline state description
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{
            .pRootSignature = pipelineLayout->rootSignature,
            .VS             = VS ? VS->GetShaderBytecode() : D3D12_SHADER_BYTECODE{},
            .PS             = PS ? PS->GetShaderBytecode() : D3D12_SHADER_BYTECODE{},
            .DS             = {.pShaderBytecode = nullptr, .BytecodeLength = 0},
            .HS             = {.pShaderBytecode = nullptr, .BytecodeLength = 0},
            .GS             = {.pShaderBytecode = nullptr, .BytecodeLength = 0},
            .StreamOutput =
                {
                               .pSODeclaration   = streamOutputDeclarations.data(),
                               .NumEntries       = (UINT)streamOutputDeclarations.size(),
                               .pBufferStrides   = streamOutputBufferStrides.data(),
                               .NumStrides       = (UINT)streamOutputBufferStrides.size(),
                               .RasterizedStream = 0,
                               },
            .BlendState      = {}, // TODO: do it after
            .SampleMask      = 0xFFFFFFFF,
            .RasterizerState = {
                               .FillMode              = ConvertToFillMode(createInfo.rasterizationState.fillMode),
                               .CullMode              = ConvertToCullMode(createInfo.rasterizationState.cullMode),
                               .FrontCounterClockwise = createInfo.rasterizationState.frontFace == PipelineRasterizerStateFrontFace::CounterClockwise,
                               .DepthBias             = 0,
                               .DepthBiasClamp        = 0.0f,
                               .SlopeScaledDepthBias  = 0.0f,
                               .DepthClipEnable       = TRUE,
                               .MultisampleEnable     = createInfo.multisampleState.sampleCount != SampleCount::Samples1,
                               .AntialiasedLineEnable = FALSE,
                               .ForcedSampleCount     = 0,
                               .ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF,
                               },
            .DepthStencilState = {
                               .DepthEnable      = createInfo.depthStencilState.depthTestEnable,
                               .DepthWriteMask   = createInfo.depthStencilState.depthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO,
                               .DepthFunc        = ConvertComparisonFunc(createInfo.depthStencilState.compareOperator),
                               .StencilEnable    = createInfo.depthStencilState.stencilTestEnable,
                               .StencilReadMask  = D3D12_DEFAULT_STENCIL_READ_MASK,
                               .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
                               .FrontFace        = {
                           .StencilFailOp      = D3D12_STENCIL_OP_KEEP,
                           .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
                           .StencilPassOp      = D3D12_STENCIL_OP_KEEP,
                           .StencilFunc        = D3D12_COMPARISON_FUNC_ALWAYS,
                },
            .BackFace = {
                    .StencilFailOp      = D3D12_STENCIL_OP_KEEP,
                    .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
                    .StencilPassOp      = D3D12_STENCIL_OP_KEEP,
                    .StencilFunc        = D3D12_COMPARISON_FUNC_ALWAYS,
                },
                               },
            .InputLayout = {
                               .pInputElementDescs = inputElements.data(),
                               .NumElements        = (UINT)inputElements.size(),
                               },
            .IBStripCutValue       = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
            .PrimitiveTopologyType = ConvertPrimitiveTopologyType(createInfo.topologyMode),
            .NumRenderTargets      = rtvFormatCount,
            .RTVFormats            = {rtvFormats[0], rtvFormats[1], rtvFormats[2], rtvFormats[3], rtvFormats[4], rtvFormats[5], rtvFormats[6], rtvFormats[7]},
            .DSVFormat             = dsvFormat,
            .SampleDesc            = {.Count = (UINT)ConvertToSampleCount(createInfo.multisampleState.sampleCount), .Quality = 0},
            .NodeMask              = 0,
            .CachedPSO             = {.pCachedBlob = nullptr, .CachedBlobSizeInBytes = 0},
            .Flags                 = D3D12_PIPELINE_STATE_FLAG_NONE
        };

        HRESULT hr = device->m_device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState));

        return ConvertFromHRESULT(hr);
    }

    void IGraphicsPipeline::Shutdown([[maybe_unused]] IDevice* device)
    {
        pipelineState->Release();
    }

    ////////////////////////////////////////////////////////////////////////
    // IComputePipeline
    ////////////////////////////////////////////////////////////////////////

    ResultCode IComputePipeline::Init(IDevice* device, const ComputePipelineCreateInfo& createInfo)
    {
        auto* pipelineLayout = (IPipelineLayout*)createInfo.layout;
        this->layout         = pipelineLayout;
        auto* CS             = (IShaderModule*)createInfo.computeShader.module;

        D3D12_COMPUTE_PIPELINE_STATE_DESC desc{
            .pRootSignature = pipelineLayout ? pipelineLayout->rootSignature : nullptr,
            .CS             = CS ? CS->GetShaderBytecode() : D3D12_SHADER_BYTECODE{},
            .NodeMask       = 0,
            .CachedPSO      = {.pCachedBlob = nullptr, .CachedBlobSizeInBytes = 0},
            .Flags          = D3D12_PIPELINE_STATE_FLAG_NONE
        };
        HRESULT hr = device->m_device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pipelineState));
        return ConvertFromHRESULT(hr);
    }

    void IComputePipeline::Shutdown([[maybe_unused]] IDevice* device)
    {
        pipelineState->Release();
        // add ref to layout
    }

    ////////////////////////////////////////////////////////////////////////
    // IBuffer
    ////////////////////////////////////////////////////////////////////////

    ResultCode IBuffer::Init(IDevice* device, const BufferCreateInfo& createInfo)
    {
        HRESULT hr;

        D3D12MA::ALLOCATION_DESC allocDesc{
            .Flags          = D3D12MA::ALLOCATION_FLAG_STRATEGY_MIN_TIME,
            .HeapType       = D3D12_HEAP_TYPE_DEFAULT,
            .ExtraHeapFlags = {},
            .CustomPool     = nullptr,
            .pPrivateData   = nullptr,
        };
        D3D12_RESOURCE_DESC1 desc{
            .Dimension                = D3D12_RESOURCE_DIMENSION_BUFFER,
            .Alignment                = 0,
            .Width                    = createInfo.byteSize,
            .Height                   = 1,
            .DepthOrArraySize         = 1,
            .MipLevels                = 1,
            .Format                   = DXGI_FORMAT_UNKNOWN,
            .SampleDesc               = {0, 0},
            .Layout                   = D3D12_TEXTURE_LAYOUT_UNKNOWN,
            .Flags                    = ConvertToResourceFlags(createInfo.usageFlags),
            .SamplerFeedbackMipRegion = {0, 0, 0},
        };
        D3D12_CLEAR_VALUE clearValue{};
        hr = device->m_allocator->CreateResource2(
            &allocDesc,
            &desc,
            D3D12_RESOURCE_STATE_COMMON,
            &clearValue,
            &allocation,
            IID_PPV_ARGS(&resource));
        return ConvertFromHRESULT(hr);
    }

    void IBuffer::Shutdown([[maybe_unused]] IDevice* device)
    {
        if (allocation) allocation->Release();
        if (resource) resource->Release();
    }

    ////////////////////////////////////////////////////////////////////////
    // IImage
    ////////////////////////////////////////////////////////////////////////

    ResultCode IImage::Init(IDevice* device, const ImageCreateInfo& createInfo)
    {
        HRESULT hr;

        D3D12MA::ALLOCATION_DESC allocDesc{
            .Flags          = D3D12MA::ALLOCATION_FLAG_STRATEGY_MIN_TIME,
            .HeapType       = D3D12_HEAP_TYPE_DEFAULT,
            .ExtraHeapFlags = {},
            .CustomPool     = nullptr,
            .pPrivateData   = nullptr,
        };
        D3D12_RESOURCE_DESC1 desc{
            .Dimension        = ConvertToResourceDimension(createInfo.type),
            .Alignment        = 0,
            .Width            = createInfo.size.width,
            .Height           = createInfo.size.height,
            .DepthOrArraySize = max(createInfo.size.depth, createInfo.arrayCount), // TODO! no min max
            .MipLevels        = (UINT16)createInfo.mipLevels,
            .Format           = ConvertToFormat(createInfo.format),
            .SampleDesc =
                {
                             .Count   = ConvertToSampleCount(createInfo.sampleCount),
                             .Quality = 0, // TOOD
                },
            .Layout                   = D3D12_TEXTURE_LAYOUT_UNKNOWN,
            .Flags                    = ConvertToResourceFlags(createInfo.usageFlags),
            .SamplerFeedbackMipRegion = {0, 0, 0},
        };
        D3D12_CLEAR_VALUE clearValue{};
        hr = device->m_allocator->CreateResource2(
            &allocDesc,
            &desc,
            D3D12_RESOURCE_STATE_COMMON,
            &clearValue,
            &allocation,
            IID_PPV_ARGS(&resource));
        return ConvertFromHRESULT(hr);
    }

    ResultCode IImage::Init(IDevice* device, ID3D12Resource1* backbuffer)
    {
        this->resource = backbuffer;
        this->type     = ImageType::Image2D;
        this->subresoruceRange =
            {
                .imageAspects = ImageAspect::Color,
            };
        return ResultCode::Success;
    }

    void IImage::Shutdown([[maybe_unused]] IDevice* device)
    {
        if (allocation)
        {
            allocation->Release();
            if (resource) resource->Release();
        }
    }

    ////////////////////////////////////////////////////////////////////////
    // ISampler
    ////////////////////////////////////////////////////////////////////////

    ResultCode ISampler::Init(IDevice* device, const SamplerCreateInfo& createInfo)
    {
        D3D12_SAMPLER_DESC desc{
            .Filter         = ConvertToFilter(createInfo.filterMin, createInfo.filterMag, createInfo.filterMip),
            .AddressU       = ConvertToAddressMode(createInfo.addressU),
            .AddressV       = ConvertToAddressMode(createInfo.addressV),
            .AddressW       = ConvertToAddressMode(createInfo.addressW),
            .MipLODBias     = createInfo.mipLodBias,
            .MaxAnisotropy  = 1,
            .ComparisonFunc = ConvertToCompareFunction(createInfo.compare),
            .BorderColor    = {0, 0, 0, 0},
            .MinLOD         = createInfo.minLod,
            .MaxLOD         = createInfo.maxLod,
        };

        return ResultCode::Success;
    }

    void ISampler::Shutdown([[maybe_unused]] IDevice* device)
    {
        // samplerHandle->Release();
    }
} // namespace RHI::D3D12