#include "RHI/Resources.hpp"

#include "RHI/Context.hpp"
#include "RHI/Swapchain.hpp"

#include <TL/Assert.hpp>

#include <spirv_reflect.h>

namespace RHI
{
    inline static bool SpvCheck(SpvReflectResult result)
    {
        auto isSuccess = result == SPV_REFLECT_RESULT_SUCCESS;
        TL_ASSERT(isSuccess);
        return isSuccess;
    }

    inline static Format ConvertVertexFormatType(SpvReflectFormat format)
    {
        switch (format)
        {
        case SPV_REFLECT_FORMAT_UNDEFINED:     return Format::Unknown;
        case SPV_REFLECT_FORMAT_R16_UINT:      return Format::R16_UINT;
        case SPV_REFLECT_FORMAT_R16_SINT:      return Format::R16_SINT;
        case SPV_REFLECT_FORMAT_R16_SFLOAT:    return Format::R16_FLOAT;
        case SPV_REFLECT_FORMAT_R16G16_UINT:   return Format::RG16_UINT;
        case SPV_REFLECT_FORMAT_R16G16_SINT:   return Format::RG16_SINT;
        case SPV_REFLECT_FORMAT_R16G16_SFLOAT: return Format::RG16_FLOAT;
        // case SPV_REFLECT_FORMAT_R16G16B16_UINT: return Format::RGB16_UINT;
        // case SPV_REFLECT_FORMAT_R16G16B16_SINT: return Format::RGB16_SINT;
        // case SPV_REFLECT_FORMAT_R16G16B16_SFLOAT:return Format::RGB16_SFLOAT;
        // case SPV_REFLECT_FORMAT_R16G16B16A16_UINT:
        // case SPV_REFLECT_FORMAT_R16G16B16A16_SINT:
        // case SPV_REFLECT_FORMAT_R16G16B16A16_SFLOAT:
        case SPV_REFLECT_FORMAT_R32_UINT:          return Format::R32_UINT;
        case SPV_REFLECT_FORMAT_R32_SINT:          return Format::R32_SINT;
        case SPV_REFLECT_FORMAT_R32_SFLOAT:        return Format::R32_FLOAT;
        case SPV_REFLECT_FORMAT_R32G32_UINT:       return Format::RG32_UINT;
        case SPV_REFLECT_FORMAT_R32G32_SINT:       return Format::RG32_SINT;
        case SPV_REFLECT_FORMAT_R32G32_SFLOAT:     return Format::RG32_FLOAT;
        case SPV_REFLECT_FORMAT_R32G32B32_UINT:    return Format::RGB32_UINT;
        case SPV_REFLECT_FORMAT_R32G32B32_SINT:    return Format::RGB32_SINT;
        case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:  return Format::RGB32_FLOAT;
        case SPV_REFLECT_FORMAT_R32G32B32A32_UINT: return Format::RGBA32_UINT;
        case SPV_REFLECT_FORMAT_R32G32B32A32_SINT: return Format::RGBA32_SINT;
        case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
            return Format::RGBA32_FLOAT;
            // case SPV_REFLECT_FORMAT_R64_UINT:return Format::R64_UINT;
            // case SPV_REFLECT_FORMAT_R64_SINT: return Format::R64_UINT;
            // case SPV_REFLECT_FORMAT_R64_SFLOAT: return Format::R64_UINT;
            // case SPV_REFLECT_FORMAT_R64G64_UINT:
            // case SPV_REFLECT_FORMAT_R64G64_SINT:
            // case SPV_REFLECT_FORMAT_R64G64_SFLOAT:
            // case SPV_REFLECT_FORMAT_R64G64B64_UINT:
            // case SPV_REFLECT_FORMAT_R64G64B64_SINT:
            // case SPV_REFLECT_FORMAT_R64G64B64_SFLOAT:
            // case SPV_REFLECT_FORMAT_R64G64B64A64_UINT:
            // case SPV_REFLECT_FORMAT_R64G64B64A64_SINT:
            // case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT: break;

        default: TL_UNREACHABLE(); return {};
        }
    }

    inline static BindingType ConvertBindingType(SpvReflectDescriptorType type)
    {
        switch (type)
        {
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:       return BindingType::Sampler;
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return BindingType::SampledImage;
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE: return BindingType::StorageImage;
        // case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return ;
        // case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return {};
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:         return BindingType::UniformBuffer;
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:         return BindingType::StorageBuffer;
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return BindingType::UniformBuffer;
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return BindingType::DynamicStorageBuffer;
        // case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return ShaderBindingType;
        // case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: break; return ;
        default: TL_UNREACHABLE(); return {};
        }
    }

    inline static PipelineInputAssemblerStateDesc GetInputAssemblerStateDesc(SpvReflectShaderModule module)
    {
        uint32_t count = 0;
        SpvCheck(spvReflectEnumerateInputVariables(&module, &count, nullptr));
        TL::Vector<SpvReflectInterfaceVariable*> inputVariables;
        inputVariables.resize(count);
        SpvCheck(spvReflectEnumerateInputVariables(&module, &count, inputVariables.data()));

        PipelineInputAssemblerStateDesc inputAssmblerStateDesc{};
        for (uint32_t varIndex = 0; varIndex < count; varIndex++)
        {
            const auto& inputVar = inputVariables[varIndex];

            auto& attributeDesc = inputAssmblerStateDesc.attributes[inputVar->location];
            attributeDesc.location = inputVar->location;
            attributeDesc.binding = inputVar->location;
            attributeDesc.format = ConvertVertexFormatType(inputVar->format);
            attributeDesc.offset = 0;

            auto& bindingDesc = inputAssmblerStateDesc.bindings[inputVar->location];
            bindingDesc.stepRate = PipelineVertexInputRate::PerVertex;
            bindingDesc.stride = GetFormatByteSize(attributeDesc.format);
            bindingDesc.binding = inputVar->location;
        }

        return inputAssmblerStateDesc;
    }

    inline static TL::Vector<Handle<BindGroupLayout>> GetBindGroupLayouts(RHI::Context& context, SpvReflectShaderModule module, TL::Flags<ShaderStage> stages)
    {
        TL::Vector<Handle<BindGroupLayout>> layouts;

        uint32_t count = 0;
        SpvCheck(spvReflectEnumerateDescriptorSets(&module, &count, nullptr));
        TL::Vector<SpvReflectDescriptorSet*> descriptorSets;
        descriptorSets.resize(count);
        SpvCheck(spvReflectEnumerateDescriptorSets(&module, &count, descriptorSets.data()));

        for (uint32_t setIndex = 0; setIndex < count; setIndex++)
        {
            const auto& descriptorSet = descriptorSets[setIndex];
            BindGroupLayoutCreateInfo bindGroupLayoutCreateInfo{};
            for (uint32_t bindingIndex = 0; bindingIndex < descriptorSet->binding_count; bindingIndex++)
            {
                const auto& binding = descriptorSet->bindings[bindingIndex];
                bindGroupLayoutCreateInfo.bindings[bindingIndex] = {
                    .type = ConvertBindingType(binding->descriptor_type),
                    .access = Access::Read,
                    .arrayCount = binding->count,
                    .stages = stages
                };
            }
            layouts.push_back(context.CreateBindGroupLayout(bindGroupLayoutCreateInfo));
        }

        return layouts;
    }

    inline static ShaderModuleReflectionData BuildReflectionData(RHI::Context* context, TL::Span<const uint32_t> spirv, TL::Flags<ShaderStage> stages)
    {
        SpvReflectShaderModule module;
        SpvCheck(spvReflectCreateShaderModule(spirv.size_bytes(), spirv.data(), &module));

        ShaderModuleReflectionData reflectionData{};
        reflectionData.bindGroupLayout = GetBindGroupLayouts(*context, module, stages);

        if (stages & ShaderStage::Vertex)
        {
            reflectionData.inputAssemblerStateDesc = GetInputAssemblerStateDesc(module);
        }

        uint32_t i = 0;
        PipelineLayoutCreateInfo pipelineLayoutCI{};
        for (const auto& layout : reflectionData.bindGroupLayout)
            pipelineLayoutCI.layouts[i++] = layout;
        reflectionData.pipelineLayout = context->CreatePipelineLayout(pipelineLayoutCI);

        spvReflectDestroyShaderModule(&module);

        return reflectionData;
    }

    ShaderModuleReflectionData ShaderModule::GetReflectionData(const ShaderModuleEntryPointNames& request) const
    {
        TL::Flags stages = request.csName ? ShaderStage::Compute : ShaderStage::Vertex | ShaderStage::Pixel;
        return BuildReflectionData(m_context, TL::Span<const uint32_t>{ m_spirv.data(), m_spirv.size() }, stages);
    }

    Swapchain::Swapchain(Context* context)
        : m_context(context)
        , m_imageIndex(0)
        , m_imageCount(0)
        , m_images()
    {
    }

    uint32_t Swapchain::GetCurrentImageIndex() const
    {
        return m_imageIndex;
    }

    uint32_t Swapchain::GetImagesCount() const
    {
        return m_imageCount;
    }

    Handle<Image> Swapchain::GetImage() const
    {
        return m_images[m_imageIndex];
    }
} // namespace RHI