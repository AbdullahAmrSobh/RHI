#include <TL/String.hpp>
#include <TL/Serialization/Binary.hpp>
#include <TL/FileSystem/File.hpp>
#include <TL/Containers.hpp>
#include <TL/Block.hpp>
#include <TL/Allocator/Allocator.hpp>
#include <TL/Assert.hpp>

#include <TL/FileSystem/File.hpp>

#include <RHI/RHI.hpp>
#include <RHI/ShaderUtils.inl>

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

#include <slang/slang-com-helper.h>

#include "Conversions.inl"

#include <string_view>
#include <format>
#include <span>

#include "codegen.h"

struct Args
{
    inline static const char* USAGE = R"(
Usage:
  RHIShaderCompiler [options]

Options:
  --shader,  -s   <file>         Path to input slang shader (required)
  --entry,   -e   <stage>:<name> Entry point with stage prefix (can be repeated, required)
                                 Supported stages: VS, PS, FS, CS
  --include, -i   <dir>          Include or import directory (can be repeated)
  --output,  -o   <file>         Output .spirv file
  --gen,     -g   <file>         Path to generated C++ file
  --help,    -h                  Show this message

Examples:
  toolname -s shader.slang -e VS:mainVS -e PS:mainPS -o out.spv
)";

    TL::String                                    input;   // --shader, -s  Path to input slang shader
    TL::Vector<std::pair<SlangStage, TL::String>> entries; // --entry, -e  Pairs of <stage, entry name>
    TL::Vector<TL::String>                        include; // --include,-i  Include or Import directory
    TL::String                                    output;  // --output, -o  Output .spirv directory
    TL::String                                    gen;     // --gen, -g     Path to generated C++ file

    TL::Vector<TL::String> slangcArgs;

    static bool parse(Args& args, int argc, const char* argv[])
    {
        auto parseStage = [](const TL::String& s, SlangStage& stageOut) -> bool
        {
            if (s == "VS")
            {
                stageOut = SLANG_STAGE_VERTEX;
                return true;
            }
            else if (s == "PS" || s == "FS")
            {
                stageOut = SLANG_STAGE_PIXEL;
                return true;
            }
            else if (s == "CS")
            {
                stageOut = SLANG_STAGE_COMPUTE;
                return true;
            }
            return false;
        };

        for (int i = 1; i < argc; ++i)
        {
            TL::String arg = argv[i];

            if ((arg == "--shader") || (arg == "-s"))
            {
                if (i + 1 < argc) args.input = argv[++i];
                else
                {
                    TL_LOG_ERROR("Missing value for {}\n{}", arg.c_str(), USAGE);
                    return false;
                }
            }
            else if ((arg == "--entry") || (arg == "-e"))
            {
                if (i + 1 < argc)
                {
                    TL::String value = argv[++i];
                    auto       pos   = value.find(':');
                    if (pos == TL::String::npos)
                    {
                        TL_LOG_ERROR("Entry must be in <stage>:<name> form, got '{}'\n{}", value.c_str(), USAGE);
                        return false;
                    }

                    TL::String stageStr = value.substr(0, pos);
                    TL::String name     = value.substr(pos + 1);

                    SlangStage stage;
                    if (!parseStage(stageStr, stage))
                    {
                        TL_LOG_ERROR("Invalid stage '{}'. Supported: VS, PS/FS, CS\n{}", stageStr.c_str(), USAGE);
                        return false;
                    }
                    if (name.empty())
                    {
                        TL_LOG_ERROR("Entry name cannot be empty: '{}'\n{}", value.c_str(), USAGE);
                        return false;
                    }

                    args.entries.push_back({stage, name});
                }
                else
                {
                    TL_LOG_ERROR("Missing value for {}\n{}", arg.c_str(), USAGE);
                    return false;
                }
            }
            else if ((arg == "--include") || (arg == "-i"))
            {
                if (i + 1 < argc) args.include.push_back(argv[++i]);
                else
                {
                    TL_LOG_ERROR("Missing value for {}\n{}", arg.c_str(), USAGE);
                    return false;
                }
            }
            else if ((arg == "--output") || (arg == "-o"))
            {
                if (i + 1 < argc) args.output = argv[++i];
                else
                {
                    TL_LOG_ERROR("Missing value for {}\n{}", arg.c_str(), USAGE);
                    return false;
                }
            }
            else if ((arg == "--gen") || (arg == "-g"))
            {
                if (i + 1 < argc) args.gen = argv[++i];
                else
                {
                    TL_LOG_ERROR("Missing value for {}\n{}", arg.c_str(), USAGE);
                    return false;
                }
            }
            else if ((arg == "--help") || (arg == "-h"))
            {
                TL_LOG_INFO("{}", USAGE);
                return false;
            }
            else if (arg == "--")
            {
                if (i + 1 < argc) args.slangcArgs.push_back(argv[++i]);
            }
            else
            {
                TL_LOG_ERROR("Unknown option: {}\n{}", arg.c_str(), USAGE);
                return false;
            }
        }

        if (args.input.empty() || args.entries.empty())
        {
            TL_LOG_ERROR("Missing required arguments\n{}", USAGE);
            return false;
        }

        return true;
    }
};

namespace BGC
{
    class StructReflector
    {
    public:
        TL::Map<TL::String, codegen::Type*> types = {};

        codegen::Generator generator       = {};
        codegen::Scope     scopeSig        = {"sig", generator.rootScope(), codegen::ScopeKind::Namespace};
        codegen::Type*     device          = getType("RHI::Device", true);
        codegen::Type*     bindGroup       = getType("RHI::BindGroup", true);
        codegen::Type*     bindGroupLayout = getType("RHI::BindGroupLayout", true);

        void genFunction_createBindGroupLayout(codegen::Type* genType, TL::Span<std::string> shaderBindings)
        {
            auto func = genType->addFunction("createBindGroupLayout", generator.t_void);
            func->addArgument("device", device);
            func->addArgument("bindGroup", bindGroupLayout);
            func->addLine("BindGroupUpdateInfo updateInfo = {");
            for (auto updateInfo : shaderBindings)
            {
                func->addLine(updateInfo.data());
            }
            func->addLine("};");
            func->addLine("device->UpdateBindGroup(bindGroup, updateInfo);");
        }

        void genFunction_updateBindGroup(codegen::Type* genType, std::span<std::string> buffers, std::span<std::string> images, std::span<std::string> samplers)
        {
            auto func = genType->addFunction("updateBindGroup", generator.t_void);
            func->addArgument("device", device);
            func->addArgument("bindGroup", bindGroup);

            func->addLine(std::format("static constexpr uint32_t MAX_BUFFERS   = {};", buffers.size()));
            func->addLine(std::format("static constexpr uint32_t MAX_IMAGES    = {};", images.size()));
            func->addLine(std::format("static constexpr uint32_t MAX_SAMPLERS  = {};", samplers.size()));

            // Buffers
            func->addLine("uint32_t bufferCount = 0;");
            func->addLine("RHI::BindGroupBuffersUpdateInfo bufferInfos[MAX_BUFFERS] = {};");
            for (size_t i = 0; i < buffers.size(); i++)
            {
                func->addLine(std::format(
                    "if (m_dirtyMask & (1u << {})) bufferInfos[bufferCount++] = {};",
                    i,
                    buffers[i]));
            }

            // Images
            func->addLine("uint32_t imageCount = 0;");
            func->addLine("RHI::BindGroupImagesUpdateInfo imageInfos[MAX_IMAGES] = {};");
            for (size_t i = 0; i < images.size(); i++)
            {
                func->addLine(std::format(
                    "if (m_dirtyMask & (1u << ({} + MAX_BUFFERS))) imageInfos[imageCount++] = {};",
                    i,
                    images[i]));
            }

            // Samplers
            func->addLine("uint32_t samplerCount = 0;");
            func->addLine("RHI::BindGroupSamplersUpdateInfo samplerInfos[MAX_SAMPLERS] = {};");
            for (size_t i = 0; i < samplers.size(); i++)
            {
                func->addLine(std::format(
                    "if (m_dirtyMask & (1u << ({} + MAX_BUFFERS + MAX_IMAGES))) samplerInfos[samplerCount++] = {};",
                    i,
                    samplers[i]));
            }

            func->addLine("RHI::BindGroupUpdateInfo updateInfo = {};");
            func->addLine("updateInfo.buffers  = { bufferInfos, bufferCount };");
            func->addLine("updateInfo.images   = { imageInfos, imageCount };");
            func->addLine("updateInfo.samplers = { samplerInfos, samplerCount };");
            func->addLine("device->UpdateBindGroup(bindGroup, updateInfo);");
        }

        codegen::Type* getType(TL::StringView name, bool imported = false)
        {
            // TODO: cleanup
            auto it = types.find(name.data());
            if (it != types.end())
                return it->second;
            auto t = scopeSig.createType(name, imported);
            return types.emplace(name.data(), t).first->second;
        }

        codegen::Type* scaler(slang::TypeReflection::ScalarType scalar)
        {
            switch (scalar)
            {
            case slang::TypeReflection::Bool:    return getType("bool", true);
            case slang::TypeReflection::Int8:    return getType("int8_t", true);
            case slang::TypeReflection::UInt8:   return getType("uint8_t", true);
            case slang::TypeReflection::Int16:   return getType("int16_t", true);
            case slang::TypeReflection::UInt16:  return getType("uint16_t", true);
            case slang::TypeReflection::Int32:   return getType("int32_t", true);
            case slang::TypeReflection::UInt32:  return getType("uint32_t", true);
            case slang::TypeReflection::Int64:   return getType("int64_t", true);
            case slang::TypeReflection::UInt64:  return getType("uint64_t", true);
            case slang::TypeReflection::Float16: return getType("float16_t", true);
            case slang::TypeReflection::Float32: return getType("float", true);
            case slang::TypeReflection::Float64: return getType("double", true);
            case slang::TypeReflection::Void:
            default:                             TL_UNREACHABLE(); return nullptr;
            }
        }

        codegen::Type* vector(slang::TypeReflection::ScalarType scalar, uint32_t components)
        {
            if (components == 1)
                return scaler(scalar);

            switch (scalar)
            {
            case slang::TypeReflection::Bool:    return getType(std::format("glm::bvec{}", components), true);
            case slang::TypeReflection::Int32:   return getType(std::format("glm::ivec{}", components), true);
            case slang::TypeReflection::UInt32:  return getType(std::format("glm::uvec{}", components), true);
            case slang::TypeReflection::Float32: return getType(std::format("glm::vec{}", components), true);
            case slang::TypeReflection::Float64: return getType(std::format("glm::dvec{}", components), true);
            default:
                TL_UNREACHABLE();
                return nullptr;
            }
        }

        codegen::Type* matrix(slang::TypeReflection::ScalarType scalar, uint32_t rows, uint32_t columns)
        {
            switch (scalar)
            {
            case slang::TypeReflection::Float32: return getType(std::format("glm::mat{}x{}", columns, rows), true);
            case slang::TypeReflection::Float64: return getType(std::format("glm::dmat{}x{}", columns, rows), true);
            default:                             TL_UNREACHABLE(); return nullptr;
            }
        }

        codegen::Type* sampler()
        {
            return getType("RHI::BindGroupSamplersUpdateInfo", true);
        }

        codegen::Type* texture(RHI::ImageViewType type, bool msaa, bool rw)
        {
            switch (type)
            {
            case RHI::ImageViewType::None:
            case RHI::ImageViewType::View1D:
            case RHI::ImageViewType::View1DArray:
            case RHI::ImageViewType::View2D:
            case RHI::ImageViewType::View2DArray:
            case RHI::ImageViewType::View3D:
            case RHI::ImageViewType::CubeMap:
            default:
                break;
            }

            return getType("RHI::BindGroupImagesUpdateInfo", true);
        }

        codegen::Type* constantBuffer(codegen::Type* innerType)
        {
            return getType(std::format("neon::ConstantBuffer<{}>", innerType->getName()), true);
        }

        codegen::Type* rawByteAddressBuffer(bool rw)
        {
            TL_UNREACHABLE();
            return getType(std::format("{}ByteAddressBuffer", rw ? "RW" : "", true));
        }

        codegen::Type* structuredBuffer(codegen::Type* innerType, bool rw)
        {
            return getType(std::format("{}StructuredBuffer<{}>", rw ? "RW" : "", innerType->getName()), true);
        }

        codegen::Type* bindless(codegen::Type* innerType)
        {
            return getType(std::format("bindless<{}>", innerType->getName()), true);
        }

        // codegen::Type* genStructType(uint32_t fieldsCount, slang::TypeLayoutReflection*** )

        // generate a C++ resource type
        codegen::Type* genResourceType(slang::TypeLayoutReflection* type)
        {
            switch (type->getKind())
            {
            case slang::TypeReflection::Kind::Array:          return genResourceType(type->getElementTypeLayout());
            case slang::TypeReflection::Kind::Matrix:         return matrix(type->getScalarType(), type->getRowCount(), type->getColumnCount());
            case slang::TypeReflection::Kind::Vector:         return vector(type->getScalarType(), type->getRowCount());
            case slang::TypeReflection::Kind::Scalar:         return scaler(type->getScalarType());
            case slang::TypeReflection::Kind::ConstantBuffer: return constantBuffer(genResourceType(type->getElementTypeLayout()));
            case slang::TypeReflection::Kind::SamplerState:   return sampler();
            case slang::TypeReflection::Kind::Struct:
                {
                    auto structType = getType(type->getName());
                    for (int i = 0; i < type->getFieldCount(); ++i)
                    {
                        auto field = type->getFieldByIndex(i);
                        structType->addField(field->getName(), genResourceType(field->getTypeLayout()));
                    }
                    return structType;
                }
            case slang::TypeReflection::Kind::ShaderStorageBuffer:
                {
                    auto access = type->getResourceAccess();
                    bool isRW   = access == SLANG_RESOURCE_ACCESS_WRITE;
                    return structuredBuffer(genResourceType(type->getElementTypeLayout()), isRW);
                }
            case slang::TypeReflection::Kind::Resource:
                {
                    auto resource       = type->getResourceShape();
                    auto access         = type->getResourceAccess();
                    bool hasWriteAccess = access == SLANG_RESOURCE_ACCESS_READ_WRITE || access == SLANG_RESOURCE_ACCESS_WRITE;

                    switch (resource)
                    {
                    case SLANG_TEXTURE_1D:                   return texture(RHI::ImageViewType::View1D, false, hasWriteAccess);
                    case SLANG_TEXTURE_2D:                   return texture(RHI::ImageViewType::View2D, false, hasWriteAccess);
                    case SLANG_TEXTURE_3D:                   return texture(RHI::ImageViewType::View3D, false, hasWriteAccess);
                    case SLANG_TEXTURE_CUBE:                 return texture(RHI::ImageViewType::CubeMap, false, hasWriteAccess);
                    case SLANG_STRUCTURED_BUFFER:            return structuredBuffer(genResourceType(type->getElementTypeLayout()), hasWriteAccess);
                    case SLANG_BYTE_ADDRESS_BUFFER:          return rawByteAddressBuffer(hasWriteAccess);
                    case SLANG_TEXTURE_1D_ARRAY:             return texture(RHI::ImageViewType::View1DArray, false, hasWriteAccess);
                    case SLANG_TEXTURE_2D_ARRAY:             return texture(RHI::ImageViewType::View2DArray, false, hasWriteAccess);
                    case SLANG_TEXTURE_2D_MULTISAMPLE:       return texture(RHI::ImageViewType::View2D, true, hasWriteAccess);
                    case SLANG_TEXTURE_2D_MULTISAMPLE_ARRAY: return texture(RHI::ImageViewType::View2DArray, true, hasWriteAccess);
                    case SLANG_RESOURCE_BASE_SHAPE_MASK:
                    case SLANG_RESOURCE_NONE:
                    case SLANG_TEXTURE_CUBE_ARRAY:
                    case SLANG_TEXTURE_BUFFER:
                    case SLANG_RESOURCE_UNKNOWN:
                    case SLANG_ACCELERATION_STRUCTURE:
                    case SLANG_TEXTURE_SUBPASS:
                    case SLANG_RESOURCE_EXT_SHAPE_MASK:
                    case SLANG_TEXTURE_FEEDBACK_FLAG:
                    case SLANG_TEXTURE_SHADOW_FLAG:
                    case SLANG_TEXTURE_ARRAY_FLAG:
                    case SLANG_TEXTURE_MULTISAMPLE_FLAG:
                    case SLANG_TEXTURE_SUBPASS_MULTISAMPLE:
                    default:                                 TL_UNREACHABLE(); return nullptr;
                    }
                }
                break;
            case slang::TypeReflection::Kind::None:
            case slang::TypeReflection::Kind::ParameterBlock:
            case slang::TypeReflection::Kind::TextureBuffer:
            case slang::TypeReflection::Kind::GenericTypeParameter:
            case slang::TypeReflection::Kind::Interface:
            case slang::TypeReflection::Kind::OutputStream:
            case slang::TypeReflection::Kind::Specialized:
            case slang::TypeReflection::Kind::Feedback:
            case slang::TypeReflection::Kind::Pointer:
            case slang::TypeReflection::Kind::DynamicResource:
                TL_UNREACHABLE();
                break;
            }
            return nullptr;
        }
    };

    static StructReflector reflector{};

    static void addShaderBinding(std::vector<std::string>& out, RHI::BindingType binding, RHI::Access access, uint32_t count, size_t bufferStirde)
    {
        bool isBuffer = binding == RHI::BindingType::StorageBuffer || binding == RHI::BindingType::UniformBuffer ||
                        binding == RHI::BindingType::DynamicStorageBuffer || binding == RHI::BindingType::DynamicUniformBuffer;

        out.push_back(std::format(
            "\t{{.type = RHI::{}, .access = RHI::{}, .arrayCount = {}, .stages = RHI::ShaderStage::All, .bufferStride = {} }},",
            RHI::Debug::ToString(binding),
            RHI::Debug::ToString(access),
            count,
            isBuffer ? bufferStirde : 0));
    }

    static void addImageUpdate(std::vector<std::string>& out, std::string_view name)
    {
        out.push_back(std::format("{}", name));
    }

    static void addBufferUpdate(std::vector<std::string>& out, std::string_view name)
    {
        out.push_back(std::format("{}", name));
    }

    static void addSamplerUpdate(std::vector<std::string>& out, std::string_view name)
    {
        out.push_back(std::format("{}", name));
    }

    static void reflectParameterBlock(slang::TypeLayoutReflection* parameterBlock, uint32_t relativeSetIndex)
    {
        slang::TypeLayoutReflection* typeLayout = parameterBlock->getElementTypeLayout();
        slang::TypeReflection*       type       = typeLayout->getType();

        auto generetedType = reflector.scopeSig.createType(type->getName(), false);

        std::vector<std::string> shaderbindings, buffersUpdateInfo, imagesUpdateInfo, samplersUpdateInfo;

        if (auto size = typeLayout->getSize())
        {
            auto constantStructType = generetedType->innerScope()->createType("CB");
            generetedType->addField("cb", constantStructType);
            addShaderBinding(shaderbindings, RHI::BindingType::UniformBuffer, RHI::Access::Read, 1, typeLayout->getStride());
            addBufferUpdate(buffersUpdateInfo, "cb");
        }

        int rangeCount = typeLayout->getDescriptorSetDescriptorRangeCount(relativeSetIndex);
        for (int rangeIndex = 0; rangeIndex < rangeCount; ++rangeIndex)
        {
            slang::BindingType slangBindingType = typeLayout->getDescriptorSetDescriptorRangeType(relativeSetIndex, rangeIndex);
            SlangInt           descriptorCount  = typeLayout->getDescriptorSetDescriptorRangeDescriptorCount(relativeSetIndex, rangeIndex);

            auto field       = typeLayout->getBindingRangeLeafVariable(rangeIndex);
            auto fieldType   = typeLayout->getBindingRangeLeafTypeLayout(rangeIndex);
            auto access      = ConvertAccess(field->getType()->getResourceAccess());
            auto bindingType = ConvertShaderBindingType(slangBindingType);

            generetedType->addField(field->getName(), reflector.genResourceType(fieldType));

            bool isBindless = field->getType()->getKind() == slang::TypeReflection::Kind::Array && field->getType()->getTotalArrayElementCount() == 0;
            if (isBindless)
                TL_ASSERT(descriptorCount == -1);

            switch (bindingType)
            {
            case RHI::BindingType::Sampler:              addSamplerUpdate(samplersUpdateInfo, field->getName()); break;
            case RHI::BindingType::SampledImage:         addImageUpdate(imagesUpdateInfo, field->getName()); break;
            case RHI::BindingType::StorageImage:         addImageUpdate(imagesUpdateInfo, field->getName()); break;
            case RHI::BindingType::UniformBuffer:        addBufferUpdate(buffersUpdateInfo, field->getName()); break;
            case RHI::BindingType::StorageBuffer:        addBufferUpdate(buffersUpdateInfo, field->getName()); break;
            case RHI::BindingType::DynamicUniformBuffer: addBufferUpdate(buffersUpdateInfo, field->getName()); break;
            case RHI::BindingType::DynamicStorageBuffer: addBufferUpdate(buffersUpdateInfo, field->getName()); break;
            case RHI::BindingType::BufferView:           addBufferUpdate(buffersUpdateInfo, field->getName()); break;
            case RHI::BindingType::StorageBufferView:    addBufferUpdate(buffersUpdateInfo, field->getName()); break;
            default:                                     TL_UNREACHABLE(); break;
            }

            addShaderBinding(shaderbindings, bindingType, access, descriptorCount, fieldType->getElementTypeLayout()->getStride());
        }

        reflector.genFunction_createBindGroupLayout(generetedType, shaderbindings);
        reflector.genFunction_updateBindGroup(generetedType, buffersUpdateInfo, imagesUpdateInfo, samplersUpdateInfo);
    }

    void reflectProgram(slang::ProgramLayout* programLayout)
    {
        auto paramsCount = programLayout->getParameterCount();
        for (int paramIndex = 0; paramIndex < paramsCount; ++paramIndex)
        {
            reflectParameterBlock(programLayout->getParameterByIndex(paramIndex)->getTypeLayout(), 0);
        }

        std::stringstream ss;
        reflector.scopeSig.render(ss, 0);
        TL_LOG_INFO("\n{}", ss.str());
    }
}; // namespace BGC

int main(int argc, const char* argv[])
{
    Args args{};
    if (!Args::parse(args, argc, argv))
    {
        return 0;
    }

    TL_LOG_INFO("Compiling `{}`", args.input);

    SlangResult result;

    // create global slang session
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    result = createGlobalSession(globalSession.writeRef());
    SLANG_ASSERT_ON_FAIL(result);

    struct TargetAndProfileName
    {
        SlangCompileTarget format;
        const char*        profile;
    };

    // TL::Vector<slang::TargetDesc> targetDescs{
    //     {.format = SLANG_SPIRV_ASM, .profile = globalSession->findProfile("sm_6_0")}
    // };

    // std::array<slang::CompilerOptionEntry, 1> options{
    //     // {slang::CompilerOptionName::EmitSpirvDirectly, {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}},
    // };

    TL::Vector<const char*> searchPaths{};

    slang::SessionDesc sessionDesc{
        // .targets     = targetDescs.data(),
        // .targetCount = (uint32_t)targetDescs.size(),
        // .searchPaths     = searchPaths.data(),
        // .searchPathCount = (SlangInt)searchPaths.size(),
        // .compilerOptionEntries    = options.data(),
        // .compilerOptionEntryCount = options.size(),
    };
    Slang::ComPtr<slang::ISession> session;
    result = globalSession->createSession(sessionDesc, session.writeRef());
    SLANG_ASSERT_ON_FAIL(result);

    Slang::ComPtr<slang::ICompileRequest> compileRequest;
    {
        result = session->createCompileRequest(compileRequest.writeRef());
        SLANG_ASSERT_ON_FAIL(result);

        TL::String sourceCode;
        TL::File   sourceCodeFile(args.input, TL::IOMode::Read);
        auto       ioresult = sourceCodeFile.read(sourceCode);

        compileRequest->setCodeGenTarget(SlangCompileTarget::SLANG_SPIRV);

        TL_ASSERT(!args.input.empty())
        auto tu = compileRequest->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, args.input.c_str());
        compileRequest->addTranslationUnitSourceFile(tu, args.input.c_str());

        for (const auto& e : args.entries)
            compileRequest->addEntryPoint(tu, e.second.c_str(), e.first);

        if (compileRequest->getDiagnosticOutput())
        {
            TL_LOG_INFO("{}", compileRequest->getDiagnosticOutput());
        }
    }

    result = compileRequest->compile();
    SLANG_ASSERT_ON_FAIL(result);
    if (compileRequest->getDiagnosticOutput() != TL::String("\0"))
    {
        TL_LOG_INFO("{}", compileRequest->getDiagnosticOutput());
    }

    Slang::ComPtr<slang::IComponentType> outProgram;
    result = compileRequest->getProgramWithEntryPoints(outProgram.writeRef());
    SLANG_ASSERT_ON_FAIL(result);

    for (int i = 0; i < args.entries.size(); ++i)
    {
        Slang::ComPtr<slang::IBlob> outCode;
        Slang::ComPtr<slang::IBlob> outDiag;
        result = outProgram->getTargetCode(0, outCode.writeRef(), outDiag.writeRef());
        // result = outProgram->getEntryPointCode(i, 0, outCode.writeRef(), outDiag.writeRef());
        SLANG_ASSERT_ON_FAIL(result);

        if (outDiag && outDiag->getBufferSize())
        {
            TL_LOG_ERROR("{}", (const char*)outDiag->getBufferPointer());
            return 1;
        }
        TL::File file(std::format("./shader.spirv", args.output, args.entries[i].second), TL::IOMode::Write);

        auto outCodeAsBlock = TL::Block{(void*)outCode->getBufferPointer(), outCode->getBufferSize()};
        auto ioresult       = file.write(outCodeAsBlock);
        TL_ASSERT(ioresult.result == TL::IOResultCode::Success);
    }

    // Slang::ComPtr<slang::IBlob> outCode;
    // result = outProgram->getTargetCode(0, outCode.writeRef());
    // SLANG_ASSERT_ON_FAIL(result);

    // auto outCodeAsBlock = TL::Block{(void*)outCode->getBufferPointer(), outCode->getBufferSize()};
    // auto ioresult       = TL::File(std::format("{}.spirv", args.output), TL::IOMode::Write).write(outCodeAsBlock);
    // TL_ASSERT(ioresult.result == TL::IOResultCode::Success);

    if (!args.gen.empty())
    {
        BGC::reflectProgram(outProgram->getLayout());
    }

    return 0;
}