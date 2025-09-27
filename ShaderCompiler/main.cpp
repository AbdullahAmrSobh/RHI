#include <TL/String.hpp>
#include <TL/Serialization/Binary.hpp>
#include <TL/FileSystem/File.hpp>
#include <TL/Containers.hpp>
#include <TL/Block.hpp>
#include <TL/Allocator/Allocator.hpp>
#include <TL/Assert.hpp>
#include <TL/FileSystem/File.hpp>
#include <TL/Result.hpp>

#include <RHI/RHI.hpp>

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
        codegen::Scope     scopeSig        = {"GPU", generator.rootScope(), codegen::ScopeKind::Namespace};
        codegen::Type*     device          = getType("RHI::Device*", true);
        codegen::Type*     bindGroup       = getType("RHI::BindGroup*", true);
        codegen::Type*     bindGroupLayout = getType("RHI::BindGroupLayout*", true);

        void genFunction_createBindGroupLayout(codegen::Type* genType, TL::Span<std::string> shaderBindings)
        {
            auto func = genType->addFunction("createBindGroupLayout", bindGroupLayout, true);
            func->addArgument("device", device);
            func->addLine("RHI::BindGroupLayoutCreateInfo createInfo = {");
            func->addLine("\t.name = nullptr,"); // TODO: add name
            func->addLine("\t.bindings = {");    // TODO: add name
            for (auto binding : shaderBindings)
            {
                func->addLine(binding.data());
            }
            func->addLine("\t},");
            func->addLine("};");
            func->addLine("return device->CreateBindGroupLayout(createInfo);");
        }

        void addBufferUpdateLine(codegen::Function* func, uint32_t bindingIndex, const std::string& memberName)
        {
            func->addLine(std::format("if ({}.m_dirty) \n{{", memberName));
            func->addLine(std::format("\t{}.m_dirty = false;", memberName));
            func->addLine(std::format("\tbufferInfos[bufferCount].dstBinding = {};", bindingIndex));
            func->addLine(std::format("\tbufferInfos[bufferCount].dstArrayElement = 0;"));
            func->addLine(std::format("\tbufferInfos[bufferCount].buffers = {{ &{}.bindingInfo, 1 }};", memberName));
            func->addLine("\tbufferCount++;");
            func->addLine("}");
        }

        void addImageUpdateLine(codegen::Function* func, uint32_t bindingIndex, const std::string& memberName)
        {
            func->addLine(std::format("if ({}.m_dirty) \n{{", memberName));
            func->addLine(std::format("\t{}.m_dirty = false;", memberName));
            func->addLine(std::format("\timageInfos[imageCount].dstBinding = {};", bindingIndex));
            func->addLine(std::format("\timageInfos[imageCount].dstArrayElement = 0;"));
            func->addLine(std::format("\timageInfos[imageCount].images = {{ &{}.m_image, 1 }};", memberName));
            func->addLine("\timageCount++;");
            func->addLine("}");
        }

        void addSamplerUpdateLine(codegen::Function* func, uint32_t bindingIndex, const std::string& memberName)
        {
            func->addLine(std::format("if ({}.m_dirty) \n{{", memberName));
            func->addLine(std::format("\t{}.m_dirty = false;", memberName));
            func->addLine(std::format("\tsamplerInfos[samplerCount].dstBinding = {};", bindingIndex));
            func->addLine(std::format("\tsamplerInfos[samplerCount].dstArrayElement = 0;"));
            func->addLine(std::format("\tsamplerInfos[samplerCount].samplers = {{ &{}.m_sampler, 1 }};", memberName));
            func->addLine("\tsamplerCount++;");
            func->addLine("}");
        }

        void genFunction_updateBindGroup(codegen::Type* genType, std::span<std::pair<uint32_t, std::string>> buffers, std::span<std::pair<uint32_t, std::string>> images, std::span<std::pair<uint32_t, std::string>> samplers)
        {
            auto func = genType->addFunction("updateBindGroup", generator.t_void);
            func->addArgument("device", device);
            func->addArgument("bindGroup", bindGroup);

            // Generate constants for array sizes
            func->addLine(std::format("static constexpr uint32_t MAX_BUFFERS = {};", buffers.size()));
            func->addLine(std::format("static constexpr uint32_t MAX_IMAGES = {};", images.size()));
            func->addLine(std::format("static constexpr uint32_t MAX_SAMPLERS = {};", samplers.size()));

            func->addLine("uint32_t bufferCount = 0;");
            func->addLine("uint32_t imageCount = 0;");
            func->addLine("uint32_t samplerCount = 0;");

            // Generate buffer update code
            if (!buffers.empty())
            {
                func->addLine("RHI::BindGroupBuffersUpdateInfo bufferInfos[MAX_BUFFERS] = {};");

                for (const auto& [bindingIndex, memberName] : buffers)
                {
                    addBufferUpdateLine(func, bindingIndex, memberName);
                }
            }

            // Generate image update code
            if (!images.empty())
            {
                func->addLine("RHI::BindGroupImagesUpdateInfo imageInfos[MAX_IMAGES] = {};");

                for (const auto& [bindingIndex, memberName] : images)
                {
                    addImageUpdateLine(func, bindingIndex, memberName);
                }
            }

            // Generate sampler update code
            if (!samplers.empty())
            {
                func->addLine("RHI::BindGroupSamplersUpdateInfo samplerInfos[MAX_SAMPLERS] = {};");

                for (const auto& [bindingIndex, memberName] : samplers)
                {
                    addSamplerUpdateLine(func, bindingIndex, memberName);
                }
            }

            // Create and populate the update info
            func->addLine("RHI::BindGroupUpdateInfo updateInfo = {};");

            if (!buffers.empty())
            {
                func->addLine("updateInfo.buffers = { bufferInfos, bufferCount };");
            }

            if (!images.empty())
            {
                func->addLine("updateInfo.images = { imageInfos, imageCount };");
            }

            if (!samplers.empty())
            {
                func->addLine("updateInfo.samplers = { samplerInfos, samplerCount };");
            }

            func->addLine("device->UpdateBindGroup(bindGroup, updateInfo);");
            // Call the device update function
            func->addLine("if (bufferCount || imageCount || samplerCount) \n {");
            func->addLine("\tdevice->UpdateBindGroup(bindGroup, updateInfo);");
            func->addLine("}");
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
            return getType("Engine::SamplerBinding", true);
        }

        codegen::Type* texture(RHI::ImageViewType type, bool msaa, bool rw)
        {
            switch (type)
            {
            case RHI::ImageViewType::None:
            case RHI::ImageViewType::View1D:      return getType("Engine::Texture1DBinding", true);
            case RHI::ImageViewType::View1DArray: return getType("Engine::Texture1DArrayBinding", true);
            case RHI::ImageViewType::View2D:      return getType("Engine::Texture2DBinding", true);
            case RHI::ImageViewType::View2DArray: return getType("Engine::Texture2DArrayBinding", true);
            case RHI::ImageViewType::View3D:      return getType("Engine::Texture3DBinding", true);
            case RHI::ImageViewType::CubeMap:     return getType("Engine::TextureCubeBinding", true);
            default:                              TL_UNREACHABLE(); break;
            }
            TL_LOG_ERROR("texture type not supported");
            return nullptr;
        }

        codegen::Type* constantBuffer(codegen::Type* innerType)
        {
            return getType(std::format("Engine::ConstantBufferBinding<{}>", innerType->getName()), true);
        }

        codegen::Type* rawByteAddressBuffer(bool rw)
        {
            TL_UNREACHABLE();
            return getType("RHI::BufferBindingInfo", true);
        }

        codegen::Type* structuredBuffer(codegen::Type* innerType, bool rw)
        {
            return getType(std::format("Engine::StructuredBufferBinding<{}>", innerType->getName()), true);
        }

        codegen::Type* bindless(codegen::Type* innerType)
        {
            return getType(std::format("Engine::BindlessArray<{}>", innerType->getName()), true);
        }

        TL::String getStructName(slang::TypeReflection* type)
        {
            Slang::ComPtr<ISlangBlob> fullnameBlob = nullptr;

            type->getFullName(fullnameBlob.writeRef());

            if (fullnameBlob->getBufferSize() == 0)
            {
                return type->getName();
            }

            TL::String name((const char*)fullnameBlob->getBufferPointer(), fullnameBlob->getBufferSize());

            // replace all dots with ::

            size_t pos = 0;
            while ((pos = name.find(".", pos)) != TL::String::npos)
            {
                name.replace(pos, 1, "::");
                pos += 2;
            }

            return name;
        }

        // generate a C++ struct type (scalars, vectors, matrices, arrays, structs)
        codegen::Type* genStructType(slang::TypeLayoutReflection* type, bool imported = true)
        {
            switch (type->getKind())
            {
            case slang::TypeReflection::Kind::Array:  return genStructType(type->getElementTypeLayout());
            case slang::TypeReflection::Kind::Matrix: return matrix(type->getScalarType(), type->getRowCount(), type->getColumnCount());
            case slang::TypeReflection::Kind::Vector: return vector(type->getScalarType(), type->getRowCount());
            case slang::TypeReflection::Kind::Scalar: return scaler(type->getScalarType());
            case slang::TypeReflection::Kind::Struct:
                {
                    auto structType = getType(getStructName(type->getType()), imported);
                    for (int i = 0; i < type->getFieldCount(); ++i)
                    {
                        auto field = type->getFieldByIndex(i);
                        structType->addField(field->getName(), genStructType(field->getTypeLayout()));
                    }
                    return structType;
                }
            default:
                TL_UNREACHABLE_MSG("UNSUPPORTED STRUCT TYPE");
                return nullptr;
            }
        }

        // generate a C++ API resource type (textures, buffers, samplers, etc.)
        codegen::Type* genAPIResourceType(slang::TypeLayoutReflection* type)
        {
            switch (type->getKind())
            {
            case slang::TypeReflection::Kind::ConstantBuffer: return constantBuffer(genStructType(type->getElementTypeLayout()));
            case slang::TypeReflection::Kind::SamplerState:   return sampler();
            case slang::TypeReflection::Kind::ShaderStorageBuffer:
                {
                    auto access = type->getResourceAccess();
                    bool isRW   = access == SLANG_RESOURCE_ACCESS_WRITE;
                    return structuredBuffer(genStructType(type->getElementTypeLayout()), isRW);
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
                    case SLANG_STRUCTURED_BUFFER:            return structuredBuffer(genStructType(type->getElementTypeLayout()), hasWriteAccess);
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
                    default:                                 TL_UNREACHABLE_MSG("UNSUPPORTED API RESOURCE TYPE"); return nullptr;
                    }
                }
                break;
            case slang::TypeReflection::Kind::Array:
            case slang::TypeReflection::Kind::Matrix:
            case slang::TypeReflection::Kind::Vector:
            case slang::TypeReflection::Kind::Scalar:
            case slang::TypeReflection::Kind::Struct:
                // These should be handled by genStructType, not genAPIResourceType
                TL_UNREACHABLE_MSG("STRUCT TYPE PASSED TO API RESOURCE FUNCTION");
                return nullptr;
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
                TL_UNREACHABLE_MSG("UNSUPPORTED API RESOURCE TYPE");
                break;
            }
            return nullptr;
        }

        // generate a C++ resource type (wrapper that delegates to appropriate function)
        codegen::Type* genResourceType(slang::TypeLayoutReflection* type, bool nested = false)
        {
            switch (type->getKind())
            {
            case slang::TypeReflection::Kind::Array:
            case slang::TypeReflection::Kind::Matrix:
            case slang::TypeReflection::Kind::Vector:
            case slang::TypeReflection::Kind::Scalar:
            case slang::TypeReflection::Kind::Struct:
                return genStructType(type, !nested);
            case slang::TypeReflection::Kind::ConstantBuffer:
            case slang::TypeReflection::Kind::SamplerState:
            case slang::TypeReflection::Kind::ShaderStorageBuffer:
            case slang::TypeReflection::Kind::Resource:
                return genAPIResourceType(type);
            default:
                TL_UNREACHABLE_MSG("UNSUPPORTED TYPE");
                return nullptr;
            }
        }
    };

    static StructReflector reflector{};

    static void addShaderBinding(std::vector<std::string>& out, RHI::BindingType binding, RHI::Access access, int32_t count, size_t bufferStirde)
    {
        bool isBuffer = binding == RHI::BindingType::StorageBuffer || binding == RHI::BindingType::UniformBuffer ||
                        binding == RHI::BindingType::DynamicStorageBuffer || binding == RHI::BindingType::DynamicUniformBuffer;

        out.push_back(std::format(
            "\t{{.type = RHI::{}, .access = RHI::{}, .arrayCount = {}, .stages = RHI::ShaderStage::AllStages, .bufferStride = {} }},",
            RHI::Debug::ToString(binding),
            RHI::Debug::ToString(access),
            count == -1 ? "= RHI::BindlessArraySize" : std::to_string(count),
            isBuffer ? bufferStirde : 0));
    }

    static void addImageUpdate(std::vector<std::pair<uint32_t, std::string>>& out, uint32_t bindingIndex, std::string_view name)
    {
        out.push_back({bindingIndex, std::string(name)});
    }

    static void addBufferUpdate(std::vector<std::pair<uint32_t, std::string>>& out, uint32_t bindingIndex, std::string_view name)
    {
        out.push_back({bindingIndex, std::string(name)});
    }

    static void addSamplerUpdate(std::vector<std::pair<uint32_t, std::string>>& out, uint32_t bindingIndex, std::string_view name)
    {
        out.push_back({bindingIndex, std::string(name)});
    }

    static bool hasDynamicAttribute(slang::UserAttribute* attr)
    {
        for (int i = 0; i < attr->getArgumentCount(); ++i)
        {
            auto arg  = attr->getArgumentType(i);
            auto name = attr->getArgumentValueString(i, nullptr);
            if (strcmp(name, "dynamic") == 0)
            {
                return true;
            }
        }

        return false;
    }

    static void reflectParameterBlock(slang::TypeLayoutReflection* parameterBlock, uint32_t relativeSetIndex)
    {
        slang::TypeLayoutReflection* typeLayout = parameterBlock->getElementTypeLayout();
        slang::TypeReflection*       type       = typeLayout->getType();

        if (typeLayout == nullptr || type == nullptr)
        {
            return;
        }

        auto generetedType = reflector.scopeSig.createType(type->getName(), false);

        std::vector<std::string>                      shaderbindings;
        std::vector<std::pair<uint32_t, std::string>> buffersUpdateInfo, imagesUpdateInfo, samplersUpdateInfo;

        if (auto size = typeLayout->getSize())
        {
            auto constantStructType = generetedType->innerScope()->createType("CB");

            // Add fields to the constantStructType
            bool                                         isDynamic = false;
            TL::Vector<slang::VariableLayoutReflection*> fields;
            for (int i = 0; i < typeLayout->getFieldCount(); ++i)
            {
                auto field = typeLayout->getFieldByIndex(i);
                auto kind  = field->getTypeLayout()->getKind();

                // TODO: Only allow single dynamic buffer per group
                auto attr = field->getType()->getUserAttributeByIndex(0);
                isDynamic = hasDynamicAttribute(attr);

                if (kind == slang::TypeReflection::Kind::Array)
                {
                    kind = field->getTypeLayout()->getElementTypeLayout()->getKind();
                }

                bool isCB = kind == slang::TypeReflection::Kind::Struct ||
                            kind == slang::TypeReflection::Kind::Matrix ||
                            kind == slang::TypeReflection::Kind::Vector ||
                            kind == slang::TypeReflection::Kind::Scalar;

                if (isCB)
                {
                    fields.push_back(field);
                    constantStructType->addField(field->getName(), reflector.genResourceType(field->getTypeLayout()));
                }
            }

            auto bindingType = isDynamic ? RHI::BindingType::DynamicUniformBuffer : RHI::BindingType::UniformBuffer;
            generetedType->addField("cb", reflector.constantBuffer(constantStructType));
            addShaderBinding(shaderbindings, bindingType, RHI::Access::Read, 1, typeLayout->getStride());
            addBufferUpdate(buffersUpdateInfo, 0, "cb");
        }

        int      rangeCount          = typeLayout->getDescriptorSetDescriptorRangeCount(relativeSetIndex);
        uint32_t currentBindingIndex = typeLayout->getSize() ? 1 : 0; // Start from 1 if we have a constant buffer (binding 0)

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
            {
                TL_ASSERT(descriptorCount == -1);
            }


            // TODO: Fix this
            bool hasDynamic = false;
            for (auto attrIdx = 0; attrIdx < field->getUserAttributeCount(); attrIdx++)
            {
                auto       attr = field->getUserAttributeByIndex(attrIdx);
                TL::String name = attr->getName();
                hasDynamic      = name == "dynamic";
                TL_LOG_INFO("Found attr {}", name);
            }

            if (hasDynamic)
            {
                switch (bindingType)
                {
                case RHI::BindingType::StorageImage:  bindingType = RHI::BindingType::DynamicStorageBuffer; break;
                case RHI::BindingType::UniformBuffer: bindingType = RHI::BindingType::DynamicUniformBuffer; break;
                default:                              break;
                };
            }

            switch (bindingType)
            {
            case RHI::BindingType::Sampler:              addSamplerUpdate(samplersUpdateInfo, currentBindingIndex, field->getName()); break;
            case RHI::BindingType::SampledImage:         addImageUpdate(imagesUpdateInfo, currentBindingIndex, field->getName()); break;
            case RHI::BindingType::StorageImage:         addImageUpdate(imagesUpdateInfo, currentBindingIndex, field->getName()); break;
            case RHI::BindingType::UniformBuffer:        addBufferUpdate(buffersUpdateInfo, currentBindingIndex, field->getName()); break;
            case RHI::BindingType::StorageBuffer:        addBufferUpdate(buffersUpdateInfo, currentBindingIndex, field->getName()); break;
            case RHI::BindingType::DynamicUniformBuffer: addBufferUpdate(buffersUpdateInfo, currentBindingIndex, field->getName()); break;
            case RHI::BindingType::DynamicStorageBuffer: addBufferUpdate(buffersUpdateInfo, currentBindingIndex, field->getName()); break;
            case RHI::BindingType::BufferView:           addBufferUpdate(buffersUpdateInfo, currentBindingIndex, field->getName()); break;
            case RHI::BindingType::StorageBufferView:    addBufferUpdate(buffersUpdateInfo, currentBindingIndex, field->getName()); break;
            default:                                     TL_UNREACHABLE(); break;
            }

            addShaderBinding(shaderbindings, bindingType, access, descriptorCount, fieldType->getElementTypeLayout()->getStride());
            currentBindingIndex++;
        }

        reflector.genFunction_createBindGroupLayout(generetedType, shaderbindings);
        reflector.genFunction_updateBindGroup(generetedType, buffersUpdateInfo, imagesUpdateInfo, samplersUpdateInfo);
    }

    void reflectProgram(slang::ProgramLayout* programLayout, std::ostream& stream)
    {
        auto paramsCount = programLayout->getParameterCount();
        for (int paramIndex = 0; paramIndex < paramsCount; ++paramIndex)
        {
            if (auto parameterBlock = programLayout->getParameterByIndex(paramIndex)->getTypeLayout())
            {
                reflectParameterBlock(parameterBlock, 0);
            }
        }

        reflector.scopeSig.render(stream, 0);
    }
}; // namespace BGC

bool createSlangSession(Slang::ComPtr<slang::IGlobalSession>& globalSession, TL::String& errorMessage)
{
    SlangResult result = createGlobalSession(globalSession.writeRef());
    if (SLANG_FAILED(result))
    {
        errorMessage = "Failed to create Slang global session";
        return false;
    }
    return true;
}

bool createCompileSession(Slang::ComPtr<slang::IGlobalSession> globalSession, Slang::ComPtr<slang::ISession>& session, TL::String& errorMessage)
{
    slang::SessionDesc sessionDesc{};
    SlangResult        result = globalSession->createSession(sessionDesc, session.writeRef());
    if (SLANG_FAILED(result))
    {
        errorMessage = "Failed to create Slang session";
        return false;
    }
    return true;
}

bool setupCompileRequest(Slang::ComPtr<slang::ISession> session, const Args& args, Slang::ComPtr<slang::ICompileRequest>& compileRequest, TL::String& errorMessage)
{
    SlangResult result = session->createCompileRequest(compileRequest.writeRef());
    if (SLANG_FAILED(result))
    {
        errorMessage = "Failed to create compile request";
        return false;
    }

    // Read source file
    TL::File   sourceCodeFile(args.input, TL::IOMode::Read);
    TL::String sourceCode;
    auto       ioresult = sourceCodeFile.read(sourceCode);
    if (ioresult.result != TL::IOResultCode::Success)
    {
        errorMessage = std::format("Failed to read shader file '{}'", args.input);
        return false;
    }

    for (auto includeDir : args.include)
    {
        compileRequest->addSearchPath(includeDir.c_str());
    }

    compileRequest->setCodeGenTarget(SlangCompileTarget::SLANG_SPIRV);

    auto tu = compileRequest->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, args.input.c_str());
    compileRequest->addTranslationUnitSourceFile(tu, args.input.c_str());

    for (const auto& e : args.entries)
    {
        compileRequest->addEntryPoint(tu, e.second.c_str(), e.first);
    }

    return true;
}

bool compileShader(Slang::ComPtr<slang::ICompileRequest> compileRequest, TL::String& errorMessage)
{
    SlangResult result = compileRequest->compile();
    if (SLANG_FAILED(result))
    {
        TL::String diagnosticOutput = compileRequest->getDiagnosticOutput();
        if (diagnosticOutput != TL::String("\0"))
        {
            errorMessage = std::format("Shader compilation failed:\n{}", diagnosticOutput.c_str());
        }
        else
        {
            errorMessage = "Shader compilation failed with no diagnostic output";
        }
        return false;
    }

    // Log any warnings or info
    TL::String diagnosticOutput = compileRequest->getDiagnosticOutput();
    if (diagnosticOutput != TL::String("\0"))
    {
        TL_LOG_INFO("Compilation warnings/info:\n{}", diagnosticOutput.c_str());
    }

    return true;
}

bool getCompiledProgram(Slang::ComPtr<slang::ICompileRequest> compileRequest, Slang::ComPtr<slang::IComponentType>& outProgram, TL::String& errorMessage)
{
    SlangResult result = compileRequest->getProgramWithEntryPoints(outProgram.writeRef());
    if (SLANG_FAILED(result))
    {
        errorMessage = "Failed to get compiled program with entry points";
        return false;
    }
    return true;
}

bool writeShaderOutput(Slang::ComPtr<slang::IComponentType> outProgram, const Args& args, TL::String& errorMessage)
{
    for (int i = 0; i < args.entries.size(); ++i)
    {
        Slang::ComPtr<slang::IBlob> outCode;
        Slang::ComPtr<slang::IBlob> outDiag;
        SlangResult                 result = outProgram->getTargetCode(0, outCode.writeRef(), outDiag.writeRef());
        if (SLANG_FAILED(result))
        {
            errorMessage = std::format("Failed to get target code for entry point '{}'", args.entries[i].second);
            return false;
        }

        if (outDiag && outDiag->getBufferSize())
        {
            errorMessage = std::format("Target code generation failed for entry point '{}': {}",
                args.entries[i].second,
                (const char*)outDiag->getBufferPointer());
            return false;
        }

        TL::String outputPath = std::format("{}.{}", args.output, args.entries[i].second);
        TL::File   file(outputPath, TL::IOMode::Write);
        TL_LOG_INFO("Writing shader output: {}", outputPath);

        auto outCodeAsBlock = TL::Block{(void*)outCode->getBufferPointer(), outCode->getBufferSize()};
        auto ioresult       = file.write(outCodeAsBlock);
        if (ioresult.result != TL::IOResultCode::Success)
        {
            errorMessage = std::format("Failed to write shader file '{}'", outputPath);
            return false;
        }
    }
    return true;
}

bool generateHeaderFile(Slang::ComPtr<slang::IComponentType> outProgram, const Args& args, TL::String& errorMessage)
{
    if (args.gen.empty())
    {
        return true;
    }

    std::fstream f(args.gen, std::fstream::out);
    if (!f.is_open())
    {
        errorMessage = std::format("Failed to open header file '{}' for writing", args.gen);
        return false;
    }

    TL_LOG_INFO("Generating header: {}", args.gen);
    f << "#pragma once\n\n";
    f << "#include \"Renderer/ShaderParameters.hpp\"\n\n";
    f << "#include \"Shaders/GpuCommonStructs.h\"\n\n";
    f << "#include <RHI/RHI.hpp>\n\n";
    f << "#include <glm/glm.hpp>\n\n";

    BGC::reflectProgram(outProgram->getLayout(), f);
    return true;
}

int main(int argc, const char* argv[])
{
    Args args{};
    if (!Args::parse(args, argc, argv))
    {
        return 1;
    }

    TL_LOG_INFO("Compiling `{}`", args.input);

    TL::String errorMessage;

    // Create Slang global session
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    if (!createSlangSession(globalSession, errorMessage))
    {
        TL_LOG_ERROR("{}", errorMessage);
        return 1;
    }

    // Create compile session
    Slang::ComPtr<slang::ISession> session;
    if (!createCompileSession(globalSession, session, errorMessage))
    {
        TL_LOG_ERROR("{}", errorMessage);
        return 1;
    }

    // Setup compile request
    Slang::ComPtr<slang::ICompileRequest> compileRequest;
    if (!setupCompileRequest(session, args, compileRequest, errorMessage))
    {
        TL_LOG_ERROR("{}", errorMessage);
        return 1;
    }

    // Compile shader
    if (!compileShader(compileRequest, errorMessage))
    {
        TL_LOG_ERROR("{}", errorMessage);
        return 1;
    }

    // Get compiled program
    Slang::ComPtr<slang::IComponentType> outProgram;
    if (!getCompiledProgram(compileRequest, outProgram, errorMessage))
    {
        TL_LOG_ERROR("{}", errorMessage);
        return 1;
    }

    // Write shader output files
    if (!writeShaderOutput(outProgram, args, errorMessage))
    {
        TL_LOG_ERROR("{}", errorMessage);
        return 1;
    }

    // Generate header file
    if (!generateHeaderFile(outProgram, args, errorMessage))
    {
        TL_LOG_ERROR("{}", errorMessage);
        return 1;
    }

    TL_LOG_INFO("Shader compilation completed successfully");
    return 0;
}