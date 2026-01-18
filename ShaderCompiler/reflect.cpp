#include "reflect.h"
#include "Conversions.inl"

#include <string>
#include <format>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <fstream>

struct Stream
{
    std::unordered_map<std::string, std::string> m_structLookups;

    uint32_t          indent = 0;
    std::stringstream ss;
};

void writeLine(Stream& stream, std::string_view line)
{
    for (uint32_t i = 0; i < stream.indent; ++i)
        stream.ss << "    ";
    stream.ss << line << "\n";
}

uint32_t indentPush(Stream& stream)
{
    return ++stream.indent;
}

uint32_t indentPop(Stream& stream)
{
    return --stream.indent;
}

void writeStructBegin(Stream& stream, const char* name)
{
    writeLine(stream, std::format("struct {}", name));
    writeLine(stream, "{");
    indentPush(stream);
}

void writeStructEnd(Stream& stream, const char* name)
{
    indentPop(stream);
    writeLine(stream, "};");
}

void writeStructMember(Stream& stream, const char* name)
{
    writeLine(stream, name);
}

void writeStruct(
    Stream&                                           stream,
    const char*                                       name,
    std::span<slang::VariableLayoutReflection* const> members)
{
    writeStructBegin(stream, "CB");

    for (auto member : members)
    {
        auto bytesSize      = member->getTypeLayout()->getSize();
        auto bytesAlignment = member->getTypeLayout()->getAlignment();

        auto name = member->getName();

        auto typeStr = BGC::convertVector(member->getType()->getScalarType(), member->getType()->getColumnCount());

        writeStructMember(stream, std::format("alignas({}) {} {};", bytesAlignment, typeStr, name).c_str());
    }

    writeStructEnd(stream, "CB");
}

void generateBindGroup(
    Stream&                         stream,
    const std::string&              name,
    const BGC::BindGroupReflection& bindings)
{
    indentPush(stream);
    writeStructBegin(stream, name.c_str());

    indentPop(stream);
    writeLine(stream, "public:");
    indentPush(stream);

    // Constructor

    // Add inline CB struct
    bool hasInlineCB = bindings.m_hasInlineConstantBuffer;
    if (hasInlineCB)
    {
        auto inlineConstantBuffer = bindings.m_shaderBindings.front(); // assuming first binding is inline CB

        writeStruct(stream, "CB", inlineConstantBuffer.cbFields);
    }

    writeLine(stream, "");

    // Add optional metadata

    // Per binding setters
    if (hasInlineCB)
    {
        writeLine(stream, "void setCB(const CB& value)");
        writeLine(stream, "{");
        writeLine(stream, "\tm_cbData = value;");
        writeLine(stream, "}\n");
    }

    for (const auto& b : bindings.m_shaderBindings)
    {
        // Skip inline CB setter as it's already added
        if (hasInlineCB && b.index == 0)
            continue;

        std::string memberCapitalized = b.name;
        memberCapitalized[0]          = static_cast<char>(std::toupper(memberCapitalized[0]));

        auto        slangType      = b.slangVarLayout;
        std::string bindingTypeStr = BGC::getBindingType(slangType->getKind(), slangType->getResourceShape(), slangType->getElementTypeLayout()->getName());

        writeLine(stream, std::format("void set{}({} resource)", memberCapitalized, bindingTypeStr).c_str());
        writeLine(stream, "{");
        writeLine(stream, std::format("\tm_{} = {{{}, {}, resource}};", b.name, b.index, 0).c_str());
        writeLine(stream, "}\n");
    }

    // create bind group layout function
    {
        writeLine(stream, "static RHI::BindGroupLayout* createBindGroupLayout(RHI::Device* device)");
        writeLine(stream, "{");
        indentPush(stream);

        writeLine(stream, "RHI::BindGroupLayoutCreateInfo createInfo = {};");
        writeLine(stream, std::format("createInfo.name = \"{}\";", name).c_str());
        writeLine(stream, "createInfo.bindings = {");
        indentPush(stream);

        for (const auto& b : bindings.m_shaderBindings)
        {
            writeLine(stream, std::format("{{ RHI::{}, RHI::{}, {}, RHI::ShaderStage::AllStages, {} }},", RHI::Debug::ToString(b.type), RHI::Debug::ToString(b.access), b.arrayCount, b.bufferStride).c_str());
        }

        indentPop(stream);
        writeLine(stream, "};");
        writeLine(stream, "return device->CreateBindGroupLayout(createInfo);");

        indentPop(stream);
        writeLine(stream, "}");
        writeLine(stream, "");
    }

    // update function
    {
        writeLine(stream, "void update(RHI::Device* device, RHI::RenderGraph* rg)");
        writeLine(stream, "{");
        indentPush(stream);

        if (hasInlineCB)
        {
            writeLine(stream, "auto dynamicOffset = rg->writeBuffer(m_cbData, m_cbData);");
        }

        std::vector<std::string> bufferMembers;
        std::vector<std::string> imageMembers;
        std::vector<std::string> samplerMembers;
        for (const auto& b : bindings.m_shaderBindings)
        {
            if (hasInlineCB && b.index == 0)
                continue;

            switch (b.type)
            {
            case RHI::BindingType::UniformBuffer:
            case RHI::BindingType::StorageBuffer:
            case RHI::BindingType::BufferView:
            case RHI::BindingType::StorageBufferView:
            case RHI::BindingType::DynamicStorageBuffer:
            case RHI::BindingType::DynamicUniformBuffer:
                bufferMembers.push_back(std::format("m_{},", b.name, b.name));
                break;
            case RHI::BindingType::SampledImage:
            case RHI::BindingType::StorageImage:
            case RHI::BindingType::InputAttachment:
                imageMembers.push_back(std::format("m_{},", b.name, b.name));
                break;
            case RHI::BindingType::Sampler:
                samplerMembers.push_back(std::format("m_{},", b.name, b.name));
                break;
            default:
                break;
            }
        }

        writeLine(stream, "RHI::BindGroupUpdateInfo updateInfo = {};");

        if (!bufferMembers.empty())
        {
            writeLine(stream, "RHI::BindGroupBuffersUpdateInfo buffers[] = {");
            indentPush(stream);
            for (const auto& line : bufferMembers)
            {
                writeLine(stream, line);
            }
            indentPop(stream);
            writeLine(stream, "};\n");

            writeLine(stream, "updateInfo.buffers = buffers;");
        }

        if (!imageMembers.empty())
        {
            writeLine(stream, "RHI::BindGroupImagesUpdateInfo images[] = {");
            indentPush(stream);
            for (const auto& line : imageMembers)
            {
                writeLine(stream, line);
            }
            indentPop(stream);
            writeLine(stream, "};");

            writeLine(stream, "updateInfo.images = images;\n");
        }

        if (!samplerMembers.empty())
        {
            writeLine(stream, "RHI::BindGroupSamplersUpdateInfo samplers[] = {");
            indentPush(stream);
            for (const auto& line : samplerMembers)
            {
                writeLine(stream, line);
            }
            indentPop(stream);
            writeLine(stream, "};\n");

            writeLine(stream, "updateInfo.samplers = samplers;");
        }

        writeLine(stream, "device->UpdateBindGroup(m_bindGroup, updateInfo);");

        indentPop(stream);
        writeLine(stream, "}");
    }

    indentPop(stream);
    writeLine(stream, "private:");
    indentPush(stream);

    writeStructMember(stream, "RHI::BindGroup* m_bindGroup = nullptr;");
    if (hasInlineCB)
    {
        writeStructMember(stream, "CB m_cbData;");
        writeStructMember(stream, "RHI::BindGroupBuffersUpdateInfo m_cb;");
    }

    for (const auto& b : bindings.m_shaderBindings)
    {
        // Skip inline CB member as it's already added
        if (hasInlineCB && b.index == 0)
            continue;

        std::string memberTypeStr;
        switch (b.type)
        {
        case RHI::BindingType::UniformBuffer:
        case RHI::BindingType::StorageBuffer:
        case RHI::BindingType::BufferView:
        case RHI::BindingType::StorageBufferView:
        case RHI::BindingType::DynamicStorageBuffer:
        case RHI::BindingType::DynamicUniformBuffer:
            memberTypeStr = "RHI::BindGroupBuffersUpdateInfo";
            break;
        case RHI::BindingType::SampledImage:
        case RHI::BindingType::StorageImage:
        case RHI::BindingType::InputAttachment:
            memberTypeStr = "RHI::BindGroupImagesUpdateInfo";
            break;
        case RHI::BindingType::Sampler:
            memberTypeStr = "RHI::BindGroupSamplersUpdateInfo";
            break;
        default:
            TL_UNREACHABLE()
            break;
        }

        writeStructMember(stream, std::format("{} m_{};", memberTypeStr, b.name).c_str());
    }

    writeStructEnd(stream, name.c_str());
    indentPop(stream);
}

namespace BGC
{
    BindGroupReflection::BindGroupReflection(slang::TypeLayoutReflection* parameterBlock, uint32_t relativeSetIndex)
    {
        slang::TypeLayoutReflection* typeLayout = parameterBlock->getElementTypeLayout();
        slang::TypeReflection*       type       = typeLayout->getType();

        m_name = type->getName();

        uint32_t bindingCounter = 0;

        if (auto size = typeLayout->getSize())
        {
            RHIBindingEntry binding{};
            binding.name           = "CB";
            binding.index          = bindingCounter++;
            binding.type           = RHI::BindingType::UniformBuffer;
            binding.access         = RHI::Access::Read;
            binding.stages         = RHI::ShaderStage::AllStages;
            binding.arrayCount     = 1;
            binding.bufferStride   = typeLayout->getStride();
            binding.slangVarLayout = parameterBlock;
            for (int i = 0; i < typeLayout->getFieldCount(); ++i)
            {
                auto fieldLayout = typeLayout->getFieldByIndex(i);

                if (fieldLayout->getTypeLayout()->getSize() == 0)
                    continue;

                binding.cbFields.push_back(fieldLayout);
            }
            m_shaderBindings.push_back(binding);

            m_hasInlineConstantBuffer = true;
        }

        for (int rangeIndex = 0; rangeIndex < typeLayout->getDescriptorSetDescriptorRangeCount(relativeSetIndex); ++rangeIndex)
        {
            slang::BindingType slangBindingType = typeLayout->getDescriptorSetDescriptorRangeType(relativeSetIndex, rangeIndex);
            SlangInt           descriptorCount  = typeLayout->getDescriptorSetDescriptorRangeDescriptorCount(relativeSetIndex, rangeIndex);

            auto field       = typeLayout->getBindingRangeLeafVariable(rangeIndex);
            auto fieldType   = typeLayout->getBindingRangeLeafTypeLayout(rangeIndex);
            auto access      = convertAccess(field->getType()->getResourceAccess());
            auto bindingType = convertShaderBindingType(slangBindingType);

            RHIBindingEntry binding{};
            binding.name           = field->getName();
            binding.index          = bindingCounter++;
            binding.type           = convertShaderBindingType(slangBindingType);
            binding.access         = access;
            binding.stages         = RHI::ShaderStage::AllStages;
            binding.arrayCount     = descriptorCount == -1 ? 1 : static_cast<uint32_t>(descriptorCount);
            // binding.bufferStride   = typeLayout->getStride();
            binding.slangVarLayout = fieldType;
            m_shaderBindings.push_back(binding);
        }
    }

    void BindGroupReflection::writeCPP(std::ostream& out) const
    {
        Stream stream;
        generateBindGroup(stream, m_name, *this);
        out << stream.ss.str();
    }

    void ShaderReflection::reflectProgram(slang::ProgramLayout* programLayout)
    {
        for (int paramIndex = 0; paramIndex < programLayout->getParameterCount(); ++paramIndex)
        {
            if (auto parameterBlock = programLayout->getParameterByIndex(paramIndex)->getTypeLayout())
            {
                slang::TypeLayoutReflection* typeLayout = parameterBlock->getElementTypeLayout();
                slang::TypeReflection*       type       = typeLayout->getType();

                if (typeLayout == nullptr || type == nullptr)
                {
                    continue;
                }

                bindGroups.push_back(BindGroupReflection(parameterBlock, 0));
            }
        }
    }

    void ShaderReflection::genJson(TL::StringView outPath) const
    {
        nlohmann::json root;
        // root["name"] = outPath;
        root["bindGroups"] = nlohmann::json::array();

        for (const auto& bindGroup : bindGroups)
        {
            nlohmann::json bgJson;
            bgJson["name"]     = bindGroup.m_name.c_str();
            bgJson["bindings"] = nlohmann::json::array();

            for (const auto& binding : bindGroup.m_shaderBindings)
            {
                nlohmann::json bindingJson;
                bindingJson["name"]         = binding.name.c_str();
                bindingJson["index"]        = binding.index;
                bindingJson["type"]         = RHI::Debug::ToString(binding.type);
                bindingJson["access"]       = RHI::Debug::ToString(binding.access);
                bindingJson["arrayCount"]   = binding.arrayCount;
                bindingJson["bufferStride"] = binding.bufferStride;

                bgJson["bindings"].push_back(bindingJson);
            }

            root["bindGroups"].push_back(bgJson);
        }

        auto file = std::fstream(outPath.data());
        file << root.dump(2);
    }

    void ShaderReflection::genCpp(TL::StringView outPath)
    {
        auto file = std::fstream(outPath.data());

        const char* PREFIX = R"(#pragma once
#include <RHI/RHI.hpp>

namespace neon::gfx
{
)";

        const char* SUFFIX = R"(
}; // namespace neon::gfx
)";

        file << PREFIX;
        for (const auto& bindGroup : bindGroups)
        {
            bindGroup.writeCPP(file);
        }

        file << "\n} // namespace BGC\n";
    }
} // namespace BGC
