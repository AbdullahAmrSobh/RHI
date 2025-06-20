#pragma once

#include <RHI/RHI.hpp>

#include <TL/FileSystem/File.hpp>

#include <slang/slang.h>
#include <slang/slang-com-helper.h>
#include <slang/slang-com-ptr.h>
#include <slang/slang-gfx.h>

namespace Engine
{
    struct ShaderTechnique
    {
        TL::Vector<RHI::Format> colorFormats;
        RHI::Format             depthStencilFormats;
        RHI::PipelineDepthStencilStateDesc depthStencilState;
    };

    // Represent a shader file that could be serialized to disk
    /// and contains the compiled SPIR-V code, bindings, and entry points.
    struct ShaderFile
    {
        TL::String                                        path;
        TL::Map<uint32_t, TL::Vector<RHI::ShaderBinding>> bindings;
        TL::Vector<uint32_t>                              spirv;
        TL::Map<TL::String, RHI::ShaderStage>             entryPoints;
    };

    /// A class which holds all the pipelines that are used by the renderer
    /// Also handles hot reloading of the pipelines when the shader files change
    class ShaderCompiler
    {
    public:
        void                   Init();
        void                   Shutdown();
        // Tries to compile the shader at the given path.
        // Returns a result containing the ShaderFile if successful, or an error if failed.
        TL::Result<ShaderFile> CompileShader(const char* path);

        void CompileShaderModule(const char* path);


    private:
        Slang::ComPtr<slang::IGlobalSession> m_globalSession;
        Slang::ComPtr<slang::ISession>       m_session;
    };
} // namespace Engine