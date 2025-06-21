#include <RHI/Reflect.hpp>

#include "ShaderCompiler.hpp"

#include <TL/Defer.hpp>
#include <TL/Serialization/Binary.hpp>

#include <glm/glm.hpp>

#include <array>

namespace Engine
{
    struct PipelineLayoutBuilder
    {
        std::vector<RHI::Handle<RHI::BindGroupLayout>> descriptorSetLayouts;
    };

    struct BindGroupLayoutBuilder
    {
        int32_t                        setIndex         = -1;
        TL::Vector<RHI::ShaderBinding> descriptorRanges = {};
        TL::Flags<RHI::ShaderStage>    shaderFlags      = RHI::ShaderStage::None;
    };

    inline static void LogDiagnostics(Slang::ComPtr<slang::IBlob> diagnosticsBlob)
    {
        if (diagnosticsBlob && diagnosticsBlob->getBufferSize() > 0)
        {
            TL_LOG_ERROR("Slang diagnostics:\n{}", (const char*)diagnosticsBlob->getBufferPointer());
        }
    }

    inline static TL::Flags<RHI::ShaderStage> ConvertShaderStage(SlangStage stage)
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
        case slang::BindingType::MutableRawBuffer:                return RHI::BindingType::StorageImage;
        case slang::BindingType::InputRenderTarget:               return RHI::BindingType::InputAttachment;
        case slang::BindingType::RayTracingAccelerationStructure: return RHI::BindingType::RayTracingAccelerationStructure;
        default:                                                  TL_UNREACHABLE(); return RHI::BindingType::None;
        }
    }

    inline static void PrintStructLayout(slang::TypeLayoutReflection* typeLayout, const std::string& indent = "")
    {
        using namespace slang;

        if (!typeLayout)
            return;

        auto type = typeLayout->getType();

        // Header: Struct name, size, stride, alignment
        if (type->getKind() == TypeReflection::Kind::Struct)
        {
            size_t size      = typeLayout->getSize();
            size_t stride    = typeLayout->getStride();
            size_t alignment = typeLayout->getStride(); // Often equals stride for struct elements

            TL_LOG_INFO("{}Struct: {} | Size: {} | Stride: {} | Alignment: {}", indent, type->getName(), size, stride, alignment);
        }

        // Iterate fields
        for (int i = 0; i < typeLayout->getFieldCount(); ++i)
        {
            auto fieldLayout     = typeLayout->getFieldByIndex(i);
            auto field           = fieldLayout->getVariable();
            auto fieldTypeLayout = fieldLayout->getTypeLayout();
            auto fieldType       = fieldTypeLayout->getType();

            size_t offset = fieldLayout->getOffset();
            size_t size   = fieldTypeLayout->getSize();

            std::string fieldName = field->getName();
            std::string typeName  = fieldType->getName();

            if (fieldType->getKind() == TypeReflection::Kind::Struct)
            {
                // Print header for nested struct
                TL_LOG_INFO("{}  {} : {} @ offset {}", indent, fieldName, typeName, offset);
                // Recurse into the nested struct
                PrintStructLayout(fieldTypeLayout, indent + "    ");
            }
            else if (fieldType->getKind() == TypeReflection::Kind::Array)
            {
                auto elementType   = fieldType->getElementType();
                auto elementLayout = fieldTypeLayout->getElementTypeLayout();
                int  elementCount  = fieldType->getElementCount();

                TL_LOG_INFO("{}  {} : {} @ offset {} (array of {})", indent, fieldName, typeName, offset, elementCount);

                if (elementType->getKind() == TypeReflection::Kind::Struct)
                {
                    // Recurse into each element layout
                    PrintStructLayout(elementLayout, indent + "\t");
                }
            }
            else
            {
                // Primitive
                TL_LOG_INFO("{}  {} : {} @ offset {} | size {}", indent, fieldName, typeName, offset, size);
            }
        }
    }

    inline static void ReflectAllParameters(slang::ShaderReflection* shaderReflection)
    {
        for (int i = 0; i < shaderReflection->getParameterCount(); ++i)
        {
            auto var        = shaderReflection->getParameterByIndex(i);
            auto typeLayout = var->getTypeLayout();

            std::string name = var->getName();
            auto        type = var->getType();

            // Print category
            std::cout << "Parameter: " << name << "\n";
            std::cout << "  Type: " << type->getName() << "\n";

            // Check for constant/storage buffer
            if (type->getKind() == slang::TypeReflection::Kind::ConstantBuffer ||
                type->getKind() == slang::TypeReflection::Kind::ParameterBlock)
            {
                // For cbuffers or parameter blocks, the *element* type is the struct
                auto elementTypeLayout = typeLayout->getElementTypeLayout();
                PrintStructLayout(elementTypeLayout, "    ");
            }
            else
            {
                // Print normally (e.g., regular struct/array)
                PrintStructLayout(typeLayout, "    ");
            }
        }
    }

    // Automatically-Introduced Uniform Buffer
    // ---------------------------------------

    inline static void AddBindingRange(BindGroupLayoutBuilder& bindGroupLayoutBuilder, slang::TypeLayoutReflection* typeLayout, int relativeGroupIndex, int rangeIndex)
    {
        slang::BindingType bindingType  = typeLayout->getDescriptorSetDescriptorRangeType(relativeGroupIndex, rangeIndex);
        SlangInt           bindingCount = typeLayout->getDescriptorSetDescriptorRangeDescriptorCount(relativeGroupIndex, rangeIndex);
        uint32_t           bindingIndex = bindGroupLayoutBuilder.descriptorRanges.size();
        uint32_t           bufferStride = 0;

        // Some Ranges Need to Be Skipped
        switch (bindingType)
        {
        case slang::BindingType::ConstantBuffer:
            bufferStride = typeLayout->getStride(slang::ParameterCategory::ConstantBuffer);
        case slang::BindingType::TypedBuffer:
            // TODO:
            break;
        default:
            // PrintStructLayout(typeLayout);
            break;
        }

        RHI::ShaderBinding outShaderBinding = {
            .type       = ConvertShaderBindingType(bindingType),
            .arrayCount = (uint32_t)bindingCount,
            .stages     = bindGroupLayoutBuilder.shaderFlags,
        };
        bindGroupLayoutBuilder.descriptorRanges.push_back(outShaderBinding);

        auto size      = typeLayout->getSize();
        auto stride    = typeLayout->getStride();
        auto alignment = typeLayout->getAlignment();

        TL_LOG_INFO("binding: {} size, stride, align: {}, {}, {})  type: {}", bindingIndex, size, stride, alignment, RHI::Debug::ToString(outShaderBinding));
    }

    inline static void AddBindingRanges(BindGroupLayoutBuilder& bindGroupLayoutBuilder, slang::TypeLayoutReflection* typeLayout)
    {
        int relativeSetIndex = 0;
        int rangeCount       = typeLayout->getDescriptorSetDescriptorRangeCount(relativeSetIndex);

        for (int rangeIndex = 0; rangeIndex < rangeCount; ++rangeIndex)
        {
            AddBindingRange(bindGroupLayoutBuilder, typeLayout, relativeSetIndex, rangeIndex);

            auto kind = typeLayout->getKind();
            if (kind == slang::TypeReflection::Kind::ConstantBuffer)
            {
                TL_LOG_INFO("CB: {} size: {}", "...", typeLayout->getSize(SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER));
                for (auto i = 0; i < typeLayout->getFieldCount(); ++i)
                {
                    auto field = typeLayout->getFieldByIndex(i);
                    TL_LOG_INFO("- {} size: {}", field->getName(), field->getTypeLayout()->getSize());
                }
            }
        }
    }

    inline static void addRangesForParameterBlockElement(PipelineLayoutBuilder& pipelineLayoutBuilder, BindGroupLayoutBuilder& bindGroupLayoutBuilder, slang::TypeLayoutReflection* elementTypeLayout, slang::VariableLayoutReflection* variableLayout)
    {
        if (elementTypeLayout->getSize(slang::ParameterCategory::Uniform) > 0)
        {
            auto size      = elementTypeLayout->getSize();
            auto stride    = elementTypeLayout->getStride();
            auto alignment = elementTypeLayout->getAlignment();

            TL_MAYBE_UNUSED auto vulkanBindingIndex = bindGroupLayoutBuilder.descriptorRanges.size();

            RHI::ShaderBinding binding = {
                .type         = RHI::BindingType::UniformBuffer,
                .access       = RHI::Access::Read,
                .arrayCount   = 1,
                .stages       = RHI::ShaderStage::AllStages,
                .bufferStride = stride,
            };
            bindGroupLayoutBuilder.descriptorRanges.push_back(binding);
        }

        // Once we have accounted for the possibility of an implicitly-introduced
        // constant buffer, we can move on and add bindings based on whatever
        // non-ordinary data (textures, buffers, etc.) is in the element type:
        AddBindingRanges(bindGroupLayoutBuilder, elementTypeLayout);
    }

    // Global Scope
    // ------------

    inline static void AddGlobalScopeParameters(PipelineLayoutBuilder& pipelineLayoutBuilder, BindGroupLayoutBuilder& bindGroupLayoutBuilder, slang::ProgramLayout* programLayout)
    {
        bindGroupLayoutBuilder.shaderFlags = RHI::ShaderStage::AllStages;
        auto gl                            = programLayout->getGlobalParamsVarLayout();
        addRangesForParameterBlockElement(pipelineLayoutBuilder, bindGroupLayoutBuilder, programLayout->getGlobalParamsTypeLayout(), programLayout->getGlobalParamsVarLayout());
    }

    // Entry Points
    // ------------

    inline static void AddEntryPointParameters(PipelineLayoutBuilder& pipelineLayoutBuilder, BindGroupLayoutBuilder& bindGroupLayoutBuilder, slang::EntryPointLayout* entryPointLayout)
    {
        bindGroupLayoutBuilder.shaderFlags = ConvertShaderStage(entryPointLayout->getStage());
        addRangesForParameterBlockElement(pipelineLayoutBuilder, bindGroupLayoutBuilder, entryPointLayout->getTypeLayout(), entryPointLayout->getVarLayout());
    }

    inline static void AddEntryPointParameters(PipelineLayoutBuilder& pipelineLayoutBuilder, BindGroupLayoutBuilder& bindGroupLayoutBuilder, slang::ProgramLayout* programLayout)
    {
        int entryPointCount = programLayout->getEntryPointCount();
        for (int i = 0; i < entryPointCount; ++i)
        {
            auto entryPointLayout = programLayout->getEntryPointByIndex(i);
            AddEntryPointParameters(pipelineLayoutBuilder, bindGroupLayoutBuilder, entryPointLayout);
        }
    }

    inline static void ParseShaderParameterBlock(slang::ProgramLayout* programLayout)
    {
        ReflectAllParameters(programLayout);

        PipelineLayoutBuilder  pipelineLayoutBuilder;
        BindGroupLayoutBuilder bindGroupLayoutBuilder;

        // start building pipeline layouts
        {
            bindGroupLayoutBuilder.setIndex = pipelineLayoutBuilder.descriptorSetLayouts.size();
            pipelineLayoutBuilder.descriptorSetLayouts.push_back(RHI::NullHandle);
        }

        AddGlobalScopeParameters(pipelineLayoutBuilder, bindGroupLayoutBuilder, programLayout);
        AddEntryPointParameters(pipelineLayoutBuilder, bindGroupLayoutBuilder, programLayout);

        {
            if (bindGroupLayoutBuilder.descriptorRanges.empty())
                return;

            TL::Vector<RHI::ShaderBinding> bindings;
            RHI::BindGroupLayoutCreateInfo descriptorSetLayoutInfo = {.bindings = bindGroupLayoutBuilder.descriptorRanges};

            // auto descriptorSetLayout = VK_NULL_HANDLE;
            // vkAPI.vkCreateDescriptorSetLayout(
            //     vkAPI.device,
            //     &descriptorSetLayoutInfo,
            //     nullptr,
            //     &descriptorSetLayout);

            // pipelineLayoutBuilder.descriptorSetLayouts[bindGroupLayoutBuilder.setIndex] = descriptorSetLayout;
        }

        TL_LOG_INFO("...");
    }

    void ShaderCompiler::Init()
    {
        // Start slang session
        auto result = createGlobalSession(m_globalSession.writeRef());
        TL_ASSERT(SLANG_SUCCEEDED(result));

        struct TargetAndProfileName
        {
            SlangCompileTarget format;
            const char*        profile;
        };

        TargetAndProfileName kTargets[] = {
            {SLANG_DXIL,  "sm_6_0"},
            {SLANG_SPIRV, "sm_6_0"},
            {SLANG_HLSL,  "sm_6_0"},
        };
        auto kTargetCount = SLANG_COUNT_OF(kTargets);

        std::array<slang::CompilerOptionEntry, 1> options = {
            // {slang::CompilerOptionName::EmitSpirvDirectly, {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}},
        };

        TL::Vector<slang::TargetDesc> targetDescs;
        for (auto target : kTargets)
        {
            auto profile = m_globalSession->findProfile(target.profile);

            slang::TargetDesc targetDesc{
                .format  = target.format,
                .profile = profile,
            };
            targetDescs.push_back(targetDesc);
        }

        slang::SessionDesc sessionDesc{
            .targets                  = targetDescs.data(),
            .targetCount              = (uint32_t)targetDescs.size(),
            .compilerOptionEntries    = options.data(),
            .compilerOptionEntryCount = options.size(),
        };
        result = m_globalSession->createSession(sessionDesc, m_session.writeRef());
        TL_ASSERT(SLANG_SUCCEEDED(result));
    }

    void ShaderCompiler::Shutdown()
    {
        m_session->Release();
        m_globalSession->Release();
    }
} // namespace Engine