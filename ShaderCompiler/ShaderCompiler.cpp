#include <TL/String.hpp>
#include <TL/Serialization/Binary.hpp>
#include <TL/FileSystem/File.hpp>
#include <TL/Containers.hpp>
#include <TL/Block.hpp>
#include <TL/Allocator/Allocator.hpp>

#include <RHI/RHI.hpp>
#include <RHI/ShaderUtils.inl>

#include <slang/slang.h>
#include <slang/slang-com-helper.h>
#include <slang/slang-com-ptr.h>

#include "Conversions.inl"

#include <format>

namespace BGC
{
    // generate RHI Bind group struct data layout

    static void reflectParameterBlock(slang::TypeReflection* type)
    {
        for (int i = 0; i < type->getFieldCount(); ++i)
        {
            auto field     = type->getFieldByIndex(i);
            auto fieldType = field->getType();
            TL_LOG_INFO("- binding: {}, name: {}, type: {}",
                i,
                field->getName(),
                fieldType->getName());
        }
    }

    void reflectProgram(slang::ProgramLayout* programLayout)
    {
        auto paramsCount = programLayout->getParameterCount();
        for (int i = 0; i < paramsCount; ++i)
        {
            auto parameterBlock     = programLayout->getParameterByIndex(i);
            auto parameterBlockName = parameterBlock->getName();
            TL_LOG_INFO("ParameterBlock: {}, binding: {}", parameterBlockName, parameterBlock->getBindingIndex());
            reflectParameterBlock(parameterBlock->getType()->getElementType());
        }
    }
}; // namespace BGC