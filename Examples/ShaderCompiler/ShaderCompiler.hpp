#pragma once

#include <RHI/RHI.hpp>
#include <RHI/ShaderUtils.inl>

#include <TL/FileSystem/File.hpp>
#include <TL/Serialization/Binary.hpp>

class ShaderModule
{
public:
    // Returns the SPIRV code for selected entry point.
    RHI::Shader::Spirv GetSpriv(TL::StringView entryName, RHI::ShaderStage shaderStage) const;

    // Returns the bind group layout at index for the give collection of entry points.
    RHI::Shader::BindGroupLayoutReflection GetPipelineLayoutsReflectionBlob(TL::Span<const TL::StringView> entryName, uint32_t groupIndex) const;

    // Returns the entire pipeline reflectioin for entry points.
    RHI::Shader::PipelineLayoutsReflectionBlob GetPipelineLayoutsReflectionBlob(TL::Span<const TL::StringView> entryName) const;
private:
    struct Impl;
    Impl* m_impl;
};

// Interface for compiling slang shaders to SPIR-V and generate seriaalized pipeline layout objects.
class ShaderCompiler
{
public:
    virtual ~ShaderCompiler() = default;

    TL::Result<ShaderModule> Compile(TL::StringView shaderCode) const;
    TL::Result<ShaderModule> CompileFromFile(TL::StringView shaderPath) const;

};