#include <TL/String.hpp>
#include <TL/Serialization/Binary.hpp>
#include <TL/FileSystem/File.hpp>
#include <TL/Containers.hpp>
#include <TL/Block.hpp>
#include <TL/Allocator/Allocator.hpp>

#include <array>

#include <RHI/RHI.hpp>
#include <RHI/ShaderUtils.inl>

#include <slang/slang.h>
#include <slang/slang-com-helper.h>
#include <slang/slang-com-ptr.h>

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

inline static const char* ConvertShaderBindingTypeToStrng(slang::BindingType bindingType)
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

namespace std
{
    template<>
    struct formatter<slang::BindingType>
    {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx)
        {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(const slang::BindingType& b, FormatContext& ctx) const
        {
            format_to_n(ctx.out(), strlen(ConvertShaderBindingTypeToStrng(b)), "{}", ConvertShaderBindingTypeToStrng(b));
            return ctx.out();
        }
    };
} // namespace std

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

struct ShaderEntry
{
    RHI::ShaderStage stage;
    TL::StringView   name;
};

struct ShaderCompileRequest
{
    TL::StringView              path;
    TL::Span<const ShaderEntry> entries;
};

// slang session variables
static Slang::ComPtr<slang::IGlobalSession> s_globalSession;
static Slang::ComPtr<slang::ISession>       s_session;

struct TargetAndProfileName
{
    SlangCompileTarget format;
    const char*        profile;
};

static constexpr TargetAndProfileName kTargets[] = {
    {SLANG_SPIRV, "sm_6_0"},
};

static constexpr uint32_t kTargetCount             = SLANG_COUNT_OF(kTargets);
static constexpr uint32_t SLANG_SPIRV_TARGET_INDEX = 0;

TL::Result<Slang::ComPtr<slang::IComponentType>> compileShader(TL::StringView path, TL::Span<const ShaderEntry> entries)
{
    std::filesystem::path inPath = path.data();
    // TL_LOG_INFO("Compiling `{}` output shader: `{}`", inPath.string(), outPath.string());

    TL::File file;
    if (file.open(path, TL::IOMode::Read) != TL::IOResultCode::Success)
    {
        return TL::Error("Failed to open shader file");
    }

    size_t     size = file.size();
    TL::String source(size, '\0');
    if (auto [_, result] = file.read(source, 0); result != TL::IOResultCode::Success)
    {
        return TL::Error("Failed to read shader file");
    }

    // Prepare slang compile request
    Slang::ComPtr<slang::ICompileRequest> compileRequest = nullptr;
    if (auto result = s_session->createCompileRequest(compileRequest.writeRef()); SLANG_FAILED(result))
    {
        return TL::Error("Failed to create Slang compile request");
    }

    uint32_t translationUnitIndex = 0;
    // Add primary translation unit
    translationUnitIndex          = compileRequest->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
    compileRequest->addTranslationUnitSourceFile(translationUnitIndex, path.data());

    struct EntryPointIndexAndStage
    {
        int        index;
        SlangStage stage;
    };

    TL::Map<TL::String, EntryPointIndexAndStage> compiledEntryPoints;
    for (auto [stage, name] : entries)
    {
        TL_LOG_INFO("Compiling {}", name.data());
        compiledEntryPoints[name.data()] = {compileRequest->addEntryPoint(translationUnitIndex, name.data(), ConvertShaderStage(stage)), ConvertShaderStage(stage)};
    }
    if (auto result = compileRequest->compile(); SLANG_FAILED(result))
    {
        auto output = compileRequest->getDiagnosticOutput();
        return TL::Error("Failed to compile slang shader. Error: {}");
    }

    for (auto [entryName, indexAndstage] : compiledEntryPoints)
    {
        auto [index, stage] = indexAndstage;
        Slang::ComPtr<slang::IBlob> codeBlob;
        if (auto result = compileRequest->getEntryPointCodeBlob(index, SLANG_SPIRV_TARGET_INDEX, codeBlob.writeRef()); SLANG_SUCCEEDED(result))
        {
            auto code     = (uint32_t*)codeBlob->getBufferPointer();
            auto codeSize = (uint32_t)codeBlob->getBufferSize() / sizeof(uint32_t);
            TL_ASSERT(((uint64_t)code) % 4, "pointer is not aligned by 4");
            RHI::Shader::Spirv spirv{
                entryName, ConvertShaderStage(stage), {code, code + codeSize}
            };

            std::filesystem::path outPath = inPath;
            outPath.replace_extension(std::format("{}.rhibin", entryName));

            std::fstream fileStream{outPath, std::ios::binary | std::ios::out};
            auto         encoder = TL::BinaryArchive(fileStream);
            encoder.Encode(spirv);
        }
        else
        {
            return TL::Error("Failed to get compiled shader bytecode");
        }
    }

    Slang::ComPtr<slang::IComponentType> program;
    if (SLANG_SUCCEEDED(compileRequest->getProgram(program.writeRef())))
    {
        // generateShaderInputLayouts(program->getLayout(SLANG_SPIRV_TARGET_INDEX));

        return program;
    }

    return TL::Error("Unexpecetd");
}

inline static void AddBindingRange(RHI::Shader::BindGroupLayoutReflection& bindGroupLayoutBuilder, slang::TypeLayoutReflection* typeLayout, int relativeGroupIndex, int rangeIndex)
{
    slang::BindingType bindingType  = typeLayout->getDescriptorSetDescriptorRangeType(relativeGroupIndex, rangeIndex);
    SlangInt           bindingCount = typeLayout->getDescriptorSetDescriptorRangeDescriptorCount(relativeGroupIndex, rangeIndex);
    uint32_t           bindingIndex = bindGroupLayoutBuilder.GetBindings().size();
    uint32_t           bufferStride = 0;

    // Some Ranges Need to Be Skipped
    switch (bindingType)
    {
    case slang::BindingType::ConstantBuffer: bufferStride = typeLayout->getStride(slang::ParameterCategory::ConstantBuffer); break;
    case slang::BindingType::TypedBuffer:    break;
    default:
        // PrintStructLayout(typeLayout);
        break;
    }

    auto size      = typeLayout->getSize();
    auto stride    = typeLayout->getStride();
    auto alignment = typeLayout->getAlignment();

    bindGroupLayoutBuilder.AddBinding({
        .name         = typeLayout->getName(),
        .type         = ConvertShaderBindingType(bindingType),
        .access       = ConvertAccess(typeLayout->getResourceAccess()),
        .arrayCount   = (uint32_t)bindingCount,
        .stages       = {},
        .bufferStride = stride,
    });

    // TL_LOG_INFO("binding: {} size, stride, align: {}, {}, {})  type: {}", bindingIndex, size, stride, alignment, RHI::Debug::ToString(outShaderBinding));
}

inline static void AddBindingRanges(RHI::Shader::BindGroupLayoutReflection& builder, slang::TypeLayoutReflection* typeLayout)
{
    int relativeSetIndex = 0;
    int rangeCount       = typeLayout->getDescriptorSetDescriptorRangeCount(relativeSetIndex);

    // for (int feildIndex = 0; feildIndex < typeLayout->getType()->getFieldCount(); ++feildIndex)
    // {
    //     auto feild = typeLayout->getType()->getFieldByIndex(feildIndex);
    //     TL_LOG_INFO("{}", feild->getName());
    // }


    for (int subObjectRangeIndex = 0; subObjectRangeIndex < typeLayout->getSubObjectRangeCount(); ++subObjectRangeIndex)
    {
        auto bindingIndex = typeLayout->getSubObjectRangeBindingRangeIndex(subObjectRangeIndex);
        auto typeName     = typeLayout->getName();
        auto varName      = typeLayout->getElementVarLayout()->getName();
        auto bindingKind  = typeLayout->getBindingRangeType(bindingIndex);

        auto arrayCount = (uint32_t)typeLayout->getTotalArrayElementCount();

        RHI::ShaderStage stages = RHI::ShaderStage::AllStages;
        switch (bindingKind)
        {
        case slang::BindingType::Unknown:                         TL_LOG_INFO("binding({}) - Unknown", bindingIndex); break;
        case slang::BindingType::Sampler:                         TL_LOG_INFO("binding({}) - Sampler", bindingIndex); break;
        case slang::BindingType::Texture:                         TL_LOG_INFO("binding({}) - Texture", bindingIndex); break;
        case slang::BindingType::ConstantBuffer:                  TL_LOG_INFO("binding({}) - ConstantBuffer", bindingIndex); break;
        case slang::BindingType::ParameterBlock:                  TL_LOG_INFO("binding({}) - ParameterBlock", bindingIndex); break;
        case slang::BindingType::TypedBuffer:                     TL_LOG_INFO("binding({}) - TypedBuffer", bindingIndex); break;
        case slang::BindingType::RawBuffer:                       TL_LOG_INFO("binding({}) - RawBuffer", bindingIndex); break;
        case slang::BindingType::CombinedTextureSampler:          TL_LOG_INFO("binding({}) - CombinedTextureSampler", bindingIndex); break;
        case slang::BindingType::InputRenderTarget:               TL_LOG_INFO("binding({}) - InputRenderTarget", bindingIndex); break;
        case slang::BindingType::InlineUniformData:               TL_LOG_INFO("binding({}) - InlineUniformData", bindingIndex); break;
        case slang::BindingType::RayTracingAccelerationStructure: TL_LOG_INFO("binding({}) - RayTracingAccelerationStructure", bindingIndex); break;
        case slang::BindingType::VaryingInput:                    TL_LOG_INFO("binding({}) - VaryingInput", bindingIndex); break;
        case slang::BindingType::VaryingOutput:                   TL_LOG_INFO("binding({}) - VaryingOutput", bindingIndex); break;
        case slang::BindingType::ExistentialValue:                TL_LOG_INFO("binding({}) - ExistentialValue", bindingIndex); break;
        case slang::BindingType::PushConstant:                    TL_LOG_INFO("binding({}) - PushConstant", bindingIndex); break;
        case slang::BindingType::MutableFlag:                     TL_LOG_INFO("binding({}) - MutableFlag", bindingIndex); break;
        case slang::BindingType::MutableTexture:                  TL_LOG_INFO("binding({}) - MutableTexture", bindingIndex); break;
        case slang::BindingType::MutableTypedBuffer:              TL_LOG_INFO("binding({}) - MutableTypedBuffer", bindingIndex); break;
        case slang::BindingType::MutableRawBuffer:                TL_LOG_INFO("binding({}) - MutableRawBuffer", bindingIndex); break;
        case slang::BindingType::BaseMask:                        TL_LOG_INFO("binding({}) - BaseMask", bindingIndex); break;
        case slang::BindingType::ExtMask:                         TL_LOG_INFO("binding({}) - ExtMask", bindingIndex); break;
        }
        // switch (bindingKind)
        // {
        // // clang-format off
        // case slang::BindingType::Unknown:                         TL_LOG_INFO(" - KIND: {}, Name: {};", ConvertShaderBindingTypeToStrng(bindingKind), varName); break;
        // case slang::BindingType::Sampler:                         builder.AddBinding({.name = typeName, .type = RHI::BindingType::Sampler,                                                           .arrayCount = arrayCount, .stages = stages}); break;
        // case slang::BindingType::Texture:                         builder.AddBinding({.name = typeName, .type = RHI::BindingType::SampledImage,                                                      .arrayCount = arrayCount, .stages = stages}); break;
        // case slang::BindingType::ConstantBuffer:                  builder.AddBinding({.name = typeName, .type = RHI::BindingType::UniformBuffer,                                                     .arrayCount = arrayCount, .stages = stages, .bufferStride = typeLayout->getStride()}); break;
        // case slang::BindingType::TypedBuffer:                     builder.AddBinding({.name = typeName, .type = RHI::BindingType::StorageBuffer,                   .access = RHI::Access::Read,      .arrayCount = {},         .stages = stages, .bufferStride = typeLayout->getStride()}); break;
        // case slang::BindingType::RawBuffer:                       builder.AddBinding({.name = typeName, .type = RHI::BindingType::StorageBuffer,                   .access = RHI::Access::Read,      .arrayCount = {},         .stages = stages, .bufferStride = typeLayout->getStride()}); break;
        // case slang::BindingType::InputRenderTarget:               builder.AddBinding({.name = typeName, .type = RHI::BindingType::InputAttachment,                 .access = RHI::Access::Read,      .arrayCount = {},         .stages = stages}); break;
        // case slang::BindingType::RayTracingAccelerationStructure: builder.AddBinding({.name = typeName, .type = RHI::BindingType::RayTracingAccelerationStructure, .access = RHI::Access::Read,      .arrayCount = {},         .stages = stages}); break;
        // case slang::BindingType::MutableTexture:                  builder.AddBinding({.name = typeName, .type = RHI::BindingType::StorageImage,                    .access = RHI::Access::ReadWrite, .arrayCount = arrayCount, .stages = stages}); break;
        // case slang::BindingType::MutableTypedBuffer:              builder.AddBinding({.name = typeName, .type = RHI::BindingType::StorageBuffer,                   .access = RHI::Access::ReadWrite, .arrayCount = arrayCount, .stages = stages, .bufferStride = typeLayout->getStride()}); break;
        // case slang::BindingType::MutableRawBuffer:                builder.AddBinding({.name = typeName, .type = RHI::BindingType::StorageBuffer,                   .access = RHI::Access::ReadWrite, .arrayCount = arrayCount, .stages = stages, .bufferStride = typeLayout->getStride()}); break;
        // case slang::BindingType::ParameterBlock:                  break;
        // // clang-format on
        // default:
        // };

        // addSubObjectRange(pipelineLayoutBuilder, typeLayout, subObjectRangeIndex);
    }

    // for (int rangeIndex = 0; rangeIndex < rangeCount; ++rangeIndex)
    // {
    //     AddBindingRange(bindGroupLayoutBuilder, typeLayout, relativeSetIndex, rangeIndex);

    //     auto kind = typeLayout->getKind();
    //     if (kind == slang::TypeReflection::Kind::ConstantBuffer)
    //     {
    //         TL_LOG_INFO("CB: {} size: {}", "...", typeLayout->getSize(SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER));
    //         for (auto i = 0; i < typeLayout->getFieldCount(); ++i)
    //         {
    //             auto field = typeLayout->getFieldByIndex(i);
    //             TL_LOG_INFO("- {} size: {}", field->getName(), field->getTypeLayout()->getSize());
    //         }
    //     }
    // }
}

inline static void addRangesForParameterBlockElement(RHI::Shader::BindGroupLayoutReflection& bindGroupLayoutBuilder, slang::VariableLayoutReflection* parameter)
{
    auto typeLayout = parameter->getTypeLayout();
    if (typeLayout->getSize(slang::ParameterCategory::Uniform) > 0)
    {
        auto size      = typeLayout->getSize();
        auto stride    = typeLayout->getStride();
        auto alignment = typeLayout->getAlignment();
        bindGroupLayoutBuilder.AddBinding({
            .name         = typeLayout->getName(),
            .type         = RHI::BindingType::UniformBuffer,
            .access       = RHI::Access::Read,
            .arrayCount   = 1,
            .stages       = RHI::ShaderStage::AllStages,
            .bufferStride = stride,
        });
    }
    // Once we have accounted for the possibility of an implicitly-introduced
    // constant buffer, we can move on and add bindings based on whatever
    // non-ordinary data (textures, buffers, etc.) is in the element type:
    AddBindingRanges(bindGroupLayoutBuilder, typeLayout);

    for (auto categoryIndex = 0; categoryIndex < parameter->getCategoryCount(); categoryIndex++)
    {
        auto category = parameter->getCategoryByIndex(categoryIndex);

        TL_LOG_INFO("{}-{}", (int)category, parameter->getName());
    }
}

inline static void addEntryPointParameters(RHI::Shader::BindGroupLayoutReflection& descriptorSetLayoutBuilder, slang::EntryPointLayout* entryPointLayout)
{
    addRangesForParameterBlockElement(descriptorSetLayoutBuilder, entryPointLayout->getVarLayout());
}

inline static void addEntryPointParameters(RHI::Shader::BindGroupLayoutReflection& descriptorSetLayoutBuilder, slang::ProgramLayout* programLayout)
{
    for (int i = 0; i < programLayout->getEntryPointCount(); ++i)
        addEntryPointParameters(descriptorSetLayoutBuilder, programLayout->getEntryPointByIndex(i));
}

inline static void reflectParameterBlock(RHI::Shader::BindGroupLayoutReflection& builder, slang::VariableLayoutReflection* param)
{
    auto name       = param->getName();
    auto typeLayout = param->getTypeLayout();
    TL_ASSERT(typeLayout->getKind() == slang::TypeReflection::Kind::ParameterBlock);

    if (typeLayout->getSize(slang::ParameterCategory::Uniform) > 0)
    {
        builder.AddBinding({
            .name         = typeLayout->getName(),
            .type         = RHI::BindingType::UniformBuffer,
            .access       = RHI::Access::Read,
            .arrayCount   = 1,
            .stages       = RHI::ShaderStage::AllStages,
            .bufferStride = typeLayout->getStride(),
        });
        TL_LOG_INFO("{} constants size", name, typeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM));
    }

    AddBindingRanges(builder, typeLayout);

    for (auto feildIndex = 0; typeLayout->getFieldCount(); feildIndex++)
    {
        auto feild      = typeLayout->getFieldByIndex(feildIndex);
        auto name       = feild->getName();
        auto typeLayout = feild->getTypeLayout();

        TL_LOG_INFO("{} {}", typeLayout->getName(), name);
    }
}

inline static TL::Error generateShaderInputLayouts(slang::ShaderReflection* reflection)
{
    RHI::Shader::PipelineLayoutsReflectionBlob pipelineLayoutBuilder;
    RHI::Shader::BindGroupLayoutReflection     bindGroupLayout;

    for (int entryIndex = 0; entryIndex < reflection->getEntryPointCount(); ++entryIndex)
    {
        slang::EntryPointReflection* entry = reflection->getEntryPointByIndex(entryIndex);
        SlangStage                   stage = entry->getStage();

        // Function's return
        auto returnVarLayout = entry->getResultVarLayout();
        auto entryFunction   = entry->getFunction();
        for (auto paramIndex = 0; paramIndex < entryFunction->getParameterCount(); ++paramIndex)
        {
            auto entryParam = entry->getParameterByIndex(paramIndex);
            TL_LOG_INFO("{}", entry->getName());
        }
    }

    for (auto paramIndex = 0; paramIndex < reflection->getParameterCount(); paramIndex++)
    {
        reflectParameterBlock(bindGroupLayout, reflection->getParameterByIndex(paramIndex));
    }

    return {};
}

TL::Error generateCPPStructAccessors(const ShaderCompileRequest& request)
{
    return {};
}

// Why does getEntryPointCount return 0?
// Why does assert with message not work?

int main(int argc, const char* argv[]) // TOOD: Parse args later
{
    // clang-format off
    TL::Vector<ShaderCompileRequest> compileRequests =
    {
        { "I:/repos/repos3/RHI/Examples/Triangle/Renderer/Shaders/GBufferPass.slang", {
            { RHI::ShaderStage::Vertex, "VSMain" }, { RHI::ShaderStage::Pixel, "PSMain" } , { RHI::ShaderStage::Compute, "CSMain" } } },
        // { "./Shaders/ImGui.slang",       { { RHI::ShaderStage::Vertex, "VSMain" }, { RHI::ShaderStage::Pixel, "PSMain" } } },
    };
    // clang-format on

    std::array<slang::CompilerOptionEntry, 1> options{
        // {slang::CompilerOptionName::EmitSpirvDirectly, {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}},
    };

    auto result = createGlobalSession(s_globalSession.writeRef());
    TL_ASSERT(SLANG_SUCCEEDED(result));

    TL::Vector<slang::TargetDesc> targetDescs;
    for (auto target : kTargets)
    {
        auto profile = s_globalSession->findProfile(target.profile);

        slang::TargetDesc targetDesc{
            .format  = target.format,
            .profile = profile,
        };
        targetDescs.push_back(targetDesc);
    }

    TL::Vector<const char*> searchPaths{
        "I:/repos/repos3/RHI/Examples/Triangle/Renderer/Shaders/Public",
    };

    slang::SessionDesc sessionDesc{
        .targets                  = targetDescs.data(),
        .targetCount              = (uint32_t)targetDescs.size(),
        .searchPaths              = searchPaths.data(),
        .searchPathCount          = (SlangInt)searchPaths.size(),
        .compilerOptionEntries    = options.data(),
        .compilerOptionEntryCount = options.size(),
    };
    result = s_globalSession->createSession(sessionDesc, s_session.writeRef());
    TL_ASSERT(SLANG_SUCCEEDED(result));

    // compile shaders and generate reflections
    for (auto compileRequest : compileRequests)
    {
        auto [program, error] = compileShader(compileRequest.path, compileRequest.entries);
        if (error.IsError())
        {
            TL_LOG_ERROR("{}", error.GetMessage());
        }
        generateShaderInputLayouts(program->getLayout(SLANG_SPIRV_TARGET_INDEX));
    }
}