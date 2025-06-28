#pragma once

#include <RHI/RHI.hpp>

#include <TL/FileSystem/File.hpp>

#include <slang/slang.h>
#include <slang/slang-com-helper.h>
#include <slang/slang-com-ptr.h>
#include <slang/slang-gfx.h>

// Represent a shader file that could be serialized to disk
/// and contains the compiled SPIR-V code, bindings, and entry points.
struct ShaderFile
{
    TL::String                            path;
    // TL::Map<uint32_t, TL::Vector<RHI::ShaderBinding>> bindings;
    TL::Vector<uint32_t>                  spirv;
    TL::Map<TL::String, RHI::ShaderStage> entryPoints;
};

class ShaderCompiler
{
public:
    ShaderCompiler();
    ~ShaderCompiler();

    TL::Result<ShaderFile> CompileShader(const char* path);

private:
    Slang::ComPtr<slang::IGlobalSession> m_globalSession;
    Slang::ComPtr<slang::ISession>       m_session;
};