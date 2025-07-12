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
    // {SLANG_DXIL,  "sm_6_0"},
    // {SLANG_HLSL,  "sm_6_0"}
};

static constexpr uint32_t kTargetCount             = SLANG_COUNT_OF(kTargets);
static constexpr uint32_t SLANG_SPIRV_TARGET_INDEX = 0;
// static constexpr uint32_t SLANG_DXIL_TARGET_INDEX  = 1;
// static constexpr uint32_t SLANG_HLSL_TARGET_INDEX  = 2;

static TL::Result<Slang::ComPtr<slang::IComponentType>> compileShader(TL::StringView path, TL::Span<const ShaderEntry> entries);
static TL::Error                                        generateReflecctions(slang::ShaderReflection* slangReflection);
static TL::Error                                        generateCPPStructAccessors(const ShaderCompileRequest& request);

int main(int argc, const char* argv[]) // TOOD: Parse args later
{
    // clang-format off
    TL::Vector<ShaderCompileRequest> compileRequests =
    {
        { "I:/repos/repos3/RHI/Examples/Triangle/Renderer/Shaders/GBufferPass.slang", { { RHI::ShaderStage::Vertex, "VSMain" }, { RHI::ShaderStage::Pixel, "PSMain" } } },
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

        generateReflecctions(program->getLayout(SLANG_SPIRV_TARGET_INDEX));

        // if (auto error =generateCPPStructAccessors(compileRequest))
        // {
        //     TL_LOG_ERROR("{}", error);
        // }
    }
}

static SlangStage ConvertShaderStage(RHI::ShaderStage stage)
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

static RHI::ShaderStage ConvertShaderStage(SlangStage stage)
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
    compileRequest->addTranslationUnitSourceString(translationUnitIndex, path.data(), source.c_str());

    struct EntryPointIndexAndStage
    {
        int        index;
        SlangStage stage;
    };

    TL::Map<TL::String, EntryPointIndexAndStage> compiledEntryPoints;
    for (auto [stage, name] : entries)
    {
        auto slangStage                  = ConvertShaderStage(stage);
        compiledEntryPoints[name.data()] = {compileRequest->addEntryPoint(translationUnitIndex, name.data(), slangStage), slangStage};
    }

    if (auto result = compileRequest->compile(); SLANG_FAILED(result))
    {
        auto output = compileRequest->getDiagnosticOutput();
        return TL::Error("Failed to compile slang shader. Error: ");
    }

    for (auto [entryName, indexAndstage] : compiledEntryPoints)
    {
        auto [index, stage] = indexAndstage;
        Slang::ComPtr<slang::IBlob> codeBlob;
        if (auto result = compileRequest->getEntryPointCodeBlob(index, SLANG_SPIRV_TARGET_INDEX, codeBlob.writeRef()); SLANG_SUCCEEDED(result))
        {
            auto spirvUint32      = (uint32_t*)codeBlob->getBufferPointer();
            auto spirvUint32Count = (uint32_t)codeBlob->getBufferSize() / sizeof(uint32_t);
            TL_ASSERT(((uint64_t)spirvUint32) % 4, "pointer is not aligned by 4");
            RHI::Shader::Spirv spirv{
                entryName, ConvertShaderStage(stage), RHI::Shader::SpirvCode{spirvUint32, spirvUint32 + spirvUint32Count}
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
        return program;
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

    // RHI::ShaderBinding outShaderBinding = {
    //     .arrayCount = (uint32_t)bindingCount,
    //     .stages     = bindGroupLayoutBuilder.shaderFlags,
    // };

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

inline static void AddBindingRanges(RHI::Shader::BindGroupLayoutReflection& bindGroupLayoutBuilder, slang::TypeLayoutReflection* typeLayout)
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

inline static void addRangesForParameterBlockElement(
    RHI::Shader::PipelineLayoutsReflectionBlob& pipelineLayoutBuilder,
    RHI::Shader::BindGroupLayoutReflection&     bindGroupLayoutBuilder,
    slang::TypeLayoutReflection*                elementTypeLayout,
    slang::VariableLayoutReflection*            variableLayout)
{
    if (elementTypeLayout->getSize(slang::ParameterCategory::Uniform) > 0)
    {
        auto size      = elementTypeLayout->getSize();
        auto stride    = elementTypeLayout->getStride();
        auto alignment = elementTypeLayout->getAlignment();
        bindGroupLayoutBuilder.AddBinding({
            .name         = elementTypeLayout->getName(),
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
    AddBindingRanges(bindGroupLayoutBuilder, elementTypeLayout);
}

void addEntryPointParameters(
    RHI::Shader::PipelineLayoutsReflectionBlob& pipelineLayoutBuilder,
    RHI::Shader::BindGroupLayoutReflection&     descriptorSetLayoutBuilder,
    slang::EntryPointLayout*                    entryPointLayout)
{
    // stages |= ConvertShaderStageFlags(entryPointLayout->getStage());
    addRangesForParameterBlockElement(
        pipelineLayoutBuilder,
        descriptorSetLayoutBuilder,
        entryPointLayout->getTypeLayout(),
        entryPointLayout->getVarLayout());
}

void addEntryPointParameters(
    RHI::Shader::PipelineLayoutsReflectionBlob& pipelineLayoutBuilder,
    RHI::Shader::BindGroupLayoutReflection&     descriptorSetLayoutBuilder,
    slang::ProgramLayout*                       programLayout)
{
    int entryPointCount = programLayout->getEntryPointCount();
    for (int i = 0; i < entryPointCount; ++i)
    {
        auto entryPointLayout = programLayout->getEntryPointByIndex(i);
        addEntryPointParameters(
            pipelineLayoutBuilder,
            descriptorSetLayoutBuilder,
            entryPointLayout);
    }
}

TL::Error generateReflecctions(slang::ShaderReflection* slangReflection)
{
    RHI::Shader::PipelineLayoutsReflectionBlob pipelineLayoutBuilder;
    RHI::Shader::BindGroupLayoutReflection     bindGroupLayout;

    auto elementTypeLayout = slangReflection->getGlobalParamsTypeLayout();
    auto variableLayout    = slangReflection->getGlobalParamsVarLayout();
    addRangesForParameterBlockElement(pipelineLayoutBuilder, bindGroupLayout, elementTypeLayout, variableLayout);
    addEntryPointParameters(pipelineLayoutBuilder, bindGroupLayout, slangReflection);
    return {};
}

TL::Error generateCPPStructAccessors(const ShaderCompileRequest& request)
{
    return {};
}

class ShaderModule;

class ShaderCompiler
{
public:
    struct ShaderEntry
    {
        TL::StringView   entryName;
        RHI::ShaderStage stage;
    };

    RHI::Shader::Spirv CompileShader(TL::StringView path, TL::StringView entryName, RHI::ShaderStage stage);
    // RHI::Shader::Spirv CompileShader(TL::StringView path, TL::StringView entryName, RHI::ShaderStage stage);
    void GenerateBindGroup(RHI::Shader::ShaderBindingReflection& outBindGroup, uint32_t i);
    void GeneratePipelineLayout(RHI::Shader::PipelineLayoutsReflectionBlob& outBindGroup, uint32_t i);
    // void GeneratePipelineLayout(RHI::Shader::StructsBuilder& outBindGroup, uint32_t i);

};