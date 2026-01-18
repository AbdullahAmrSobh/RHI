#pragma once

// #include <slang/slang.h>

#include <TL/Containers/Vector.hpp>
#include <TL/Containers/String.hpp>
#include <TL/Stream.hpp>

#include <RHI/RHI.hpp>

#include <nlohmann/json.hpp>

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

#include <string>
#include <vector>
#include <format>

namespace BGC
{
    class BindGroupReflection
    {
    public:
        struct RHIBindingEntry
        {
            TL::String                  name;
            uint32_t                    index;
            RHI::BindingType            type;
            RHI::Access                 access;
            uint32_t                    arrayCount;
            TL::Flags<RHI::ShaderStage> stages;
            size_t                      bufferStride;

            bool hasDynamicAttribute = false;

            slang::TypeLayoutReflection*                 slangVarLayout = nullptr;
            TL::Vector<slang::VariableLayoutReflection*> cbFields;
        };

        TL::String                    m_name;
        TL::Vector<RHIBindingEntry>   m_shaderBindings;
        TL::Map<TL::String, uint32_t> m_nameToBindings;

        bool m_hasInlineConstantBuffer;

        BindGroupReflection(slang::TypeLayoutReflection* parameterBlock, uint32_t relativeSetIndex);

        void writeCPP(std::ostream& out) const;

    };

    class ShaderReflection
    {
    public:
        TL::Vector<BindGroupReflection> bindGroups;

        void reflectProgram(slang::ProgramLayout* programLayout);
        void genJson(TL::StringView outPath) const;
        void genCpp(TL::StringView outPath);
    };
} // namespace BGC
