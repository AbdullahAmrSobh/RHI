#pragma once

#include <RHI/RHI.hpp>
#include <RHI/Reflect.hpp>

#include <TL/FileSystem/File.hpp>
#include <TL/Serialization/Binary.hpp>

#include <format>

namespace RHI::Shader
{
    using SpirvCode = TL::Vector<uint32_t>;

    struct ShaderBindingReflection
    {
        TL::String                  name;
        RHI::BindingType            type;
        RHI::Access                 access;
        uint32_t                    arrayCount;
        TL::Flags<RHI::ShaderStage> stages;
        size_t                      bufferStride;

        template<typename Archive>
        void Serialize(Archive& archive) const
        {
            TL::Encode(archive, name);
            TL::Encode(archive, type);
            TL::Encode(archive, access);
            TL::Encode(archive, arrayCount);
            TL::Encode(archive, stages);
            TL::Encode(archive, bufferStride);
        }

        template<typename Archive>
        void Deserialize(Archive& archive)
        {
            TL::Decode(archive, name);
            TL::Decode(archive, type);
            TL::Decode(archive, access);
            TL::Decode(archive, arrayCount);
            TL::Decode(archive, stages);
            TL::Decode(archive, bufferStride);
        }

        operator ShaderBinding() const
        {
            return {type, access, arrayCount, stages, bufferStride};
        }
    };

    class BindGroupLayoutReflection
    {
    public:
        BindGroupLayoutReflection() = default;

        BindGroupLayoutReflection(TL::StringView name)
            : m_name(name.data())
        {
        }

        BindGroupLayoutReflection(TL::StringView name, TL::Span<const ShaderBindingReflection> shaderBindings)
            : m_name(name.data())
            , m_bindings(shaderBindings.begin(), shaderBindings.end())
        {
        }

        TL::StringView                          GetName() const { return m_name; }

        void                                    AddBinding(const ShaderBindingReflection& binding) { m_bindings.push_back(binding); }

        TL::Span<const ShaderBindingReflection> GetBindings() const { return m_bindings; }

        template<typename Archive>
        void Serialize(Archive& archive) const
        {
            TL::Encode(archive, m_name);
            TL::Encode(archive, m_bindings);
        }

        template<typename Archive>
        void Deserialize(Archive& archive)
        {
            TL::Decode(archive, m_name);
            TL::Decode(archive, m_bindings);
        }

    private:
        TL::String                          m_name;
        TL::Vector<ShaderBindingReflection> m_bindings;
    };

    class PipelineLayoutsReflectionBlob
    {
    public:
        PipelineLayoutsReflectionBlob() = default;
        PipelineLayoutsReflectionBlob(TL::StringView name, TL::Span<const BindGroupLayoutReflection> shaderBindings)
            : m_name(name.data())
        // // , m_bindGroups(shaderBindings.data)
        // , m_bindGroups(shaderBindings.data())
        {
        }

        template<typename Archive>
        void Serialize(Archive& archive) const
        {
            TL::Encode(archive, m_name);
            TL::Encode(archive, m_bindGroups);
        }

        template<typename Archive>
        void Deserialize(Archive& archive)
        {
            TL::Decode(archive, m_name);
            TL::Decode(archive, m_bindGroups);
        }

        TL::StringView                            GetName() const;
        TL::Span<const BindGroupLayoutReflection> GetGroups() const;
        void                                      AddGroup(const BindGroupLayoutReflection& group);

    private:
        TL::String                            m_name;
        TL::Vector<BindGroupLayoutReflection> m_bindGroups;
    };

    class Spirv
    {
    public:
        Spirv() = default;

        Spirv(const TL::String& entry, RHI::ShaderStage stage, const SpirvCode& codeData)
            : m_entryName(entry)
            , m_stage(stage)
            , m_code(codeData)
        {
        }

        const TL::String& GetEntryName() const { return m_entryName; }

        RHI::ShaderStage  GetStage() const { return m_stage; }

        const SpirvCode&  GetCode() const { return m_code; }

        template<typename Archive>
        void Serialize(Archive& archive) const
        {
            TL::Encode(archive, m_entryName);
            TL::Encode(archive, m_stage);
            TL::Encode(archive, m_code);
        }

        template<typename Archive>
        void Deserialize(Archive& archive)
        {
            TL::Decode(archive, m_entryName);
            TL::Decode(archive, m_stage);
            TL::Decode(archive, m_code);
        }

    private:
        TL::String       m_entryName;
        RHI::ShaderStage m_stage;
        SpirvCode        m_code;
    };

} // namespace RHI::Shader
