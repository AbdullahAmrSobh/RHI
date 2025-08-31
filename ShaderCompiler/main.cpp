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

#include "ShaderCompiler.hpp"

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

#include <slang/slang-com-helper.h>

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