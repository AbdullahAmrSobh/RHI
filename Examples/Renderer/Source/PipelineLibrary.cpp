#include "Renderer/PipelineLibrary.hpp"
#include "Shaders/GpuCommonStructs.h"

#include <nlohmann/json.hpp>

#include <TL/Defer.hpp>

#include <glm/glm.hpp>

namespace Engine
{
    inline static TL::String readFile(TL::StringView path)
    {
        auto file       = TL::File(path, TL::IOMode::Read);
        auto shaderBlob = TL::Vector<uint8_t>(file.size());

        TL::String json_string;
        json_string.resize(file.size());

        auto [_, err] = file.read(json_string);
        TL_ASSERT(err == TL::IOResultCode::Success);
        return json_string;
    }

    class PipelineStateInitializer
    {
    public:
        inline static RHI::CompareOperator parseCompareOperator(const nlohmann::json& json)
        {
            using RHI::CompareOperator;

            TL_ASSERT(json.is_string(), "CompareOperator must be a string in JSON");

            const std::string& v = json.get<std::string>();
            if (v == "Never") return CompareOperator::Never;
            if (v == "Less") return CompareOperator::Less;
            if (v == "Equal") return CompareOperator::Equal;
            if (v == "LessOrEqual") return CompareOperator::LessOrEqual;
            if (v == "Greater") return CompareOperator::Greater;
            if (v == "NotEqual") return CompareOperator::NotEqual;
            if (v == "GreaterOrEqual") return CompareOperator::GreaterOrEqual;
            if (v == "Always") return CompareOperator::Always;

            TL_UNREACHABLE_MSG("Invalid CompareOperator string value");
            return CompareOperator::Always;
        }

        inline static RHI::BlendEquation parseBlendEquation(const nlohmann::json& json)
        {
            using RHI::BlendEquation;
            TL_ASSERT(json.is_string(), "BlendEquation must be a string in JSON");
            const std::string& v = json.get<std::string>();
            if (v == "Add") return BlendEquation::Add;
            if (v == "Subtract") return BlendEquation::Subtract;
            if (v == "ReverseSubtract") return BlendEquation::ReverseSubtract;
            if (v == "Min") return BlendEquation::Min;
            if (v == "Max") return BlendEquation::Max;
            TL_UNREACHABLE_MSG("Invalid BlendEquation string value");
            return BlendEquation::Add;
        }

        inline static RHI::BlendFactor parseBlendFactor(const nlohmann::json& json)
        {
            using RHI::BlendFactor;
            TL_ASSERT(json.is_string(), "BlendFactor must be a string in JSON");
            const std::string& v = json.get<std::string>();
            if (v == "Zero") return BlendFactor::Zero;
            if (v == "One") return BlendFactor::One;
            if (v == "SrcColor") return BlendFactor::SrcColor;
            if (v == "OneMinusSrcColor") return BlendFactor::OneMinusSrcColor;
            if (v == "DstColor") return BlendFactor::DstColor;
            if (v == "OneMinusDstColor") return BlendFactor::OneMinusDstColor;
            if (v == "SrcAlpha") return BlendFactor::SrcAlpha;
            if (v == "OneMinusSrcAlpha") return BlendFactor::OneMinusSrcAlpha;
            if (v == "DstAlpha") return BlendFactor::DstAlpha;
            if (v == "OneMinusDstAlpha") return BlendFactor::OneMinusDstAlpha;
            if (v == "ConstantColor") return BlendFactor::ConstantColor;
            if (v == "OneMinusConstantColor") return BlendFactor::OneMinusConstantColor;
            if (v == "ConstantAlpha") return BlendFactor::ConstantAlpha;
            if (v == "OneMinusConstantAlpha") return BlendFactor::OneMinusConstantAlpha;
            // if (v == "SrcAlphaSaturate") return BlendFactor::SrcAlphaSaturate;
            TL_UNREACHABLE_MSG("Invalid BlendFactor string value");
            return BlendFactor::One;
        }

        inline static RHI::ColorWriteMask parseColorWriteMask(const nlohmann::json& json)
        {
            using RHI::ColorWriteMask;
            TL_ASSERT(json.is_string(), "ColorWriteMask must be a string in JSON");
            const std::string& v = json.get<std::string>();
            // if (v == "None") return ColorWriteMask::None;
            if (v == "Red") return ColorWriteMask::Red;
            if (v == "Green") return ColorWriteMask::Green;
            if (v == "Blue") return ColorWriteMask::Blue;
            if (v == "Alpha") return ColorWriteMask::Alpha;
            if (v == "All") return ColorWriteMask::All;
            TL_UNREACHABLE_MSG("Invalid ColorWriteMask string value");
            return ColorWriteMask::All;
        }

        inline static RHI::PipelineTopologyMode parsePipelineTopologyMode(const nlohmann::json& json)
        {
            TL_ASSERT(json.is_string(), "PipelineTopologyMode must be a string in JSON");
            const std::string& v = json.get<std::string>();
            if (v == "Points") return RHI::PipelineTopologyMode::Points;
            if (v == "Lines") return RHI::PipelineTopologyMode::Lines;
            if (v == "Triangles") return RHI::PipelineTopologyMode::Triangles;
            TL_UNREACHABLE_MSG("Invalid PipelineTopologyMode string value");
            return RHI::PipelineTopologyMode::Triangles;
        }

        inline static RHI::PipelineRasterizerStateCullMode parsePipelineRasterizerStateCullMode(const nlohmann::json& json)
        {
            using RHI::PipelineRasterizerStateCullMode;
            TL_ASSERT(json.is_string(), "CullMode must be a string in JSON");
            const std::string& v = json.get<std::string>();
            if (v == "None") return RHI::PipelineRasterizerStateCullMode::None;
            if (v == "FrontFace") return RHI::PipelineRasterizerStateCullMode::FrontFace;
            if (v == "BackFace") return RHI::PipelineRasterizerStateCullMode::BackFace;
            TL_UNREACHABLE_MSG("Invalid CullMode string value");
            return RHI::PipelineRasterizerStateCullMode::None;
        }

        inline static RHI::PipelineRasterizerStateFillMode parsePipelineRasterizerStateFillMode(const nlohmann::json& json)
        {
            using RHI::PipelineRasterizerStateFillMode;
            TL_ASSERT(json.is_string(), "FillMode must be a string in JSON");
            const std::string& v = json.get<std::string>();
            if (v == "Point") return RHI::PipelineRasterizerStateFillMode::Point;
            if (v == "Triangle") return RHI::PipelineRasterizerStateFillMode::Triangle;
            if (v == "Line") return RHI::PipelineRasterizerStateFillMode::Line;
            TL_UNREACHABLE_MSG("Invalid FillMode string value");
            return RHI::PipelineRasterizerStateFillMode::Triangle;
        }

        inline static RHI::PipelineRasterizerStateFrontFace parsePipelineRasterizerStateFrontFace(const nlohmann::json& json)
        {
            using RHI::PipelineRasterizerStateFrontFace;
            TL_ASSERT(json.is_string(), "FrontFace must be a string in JSON");
            const std::string& v = json.get<std::string>();
            if (v == "Clockwise") return RHI::PipelineRasterizerStateFrontFace::Clockwise;
            if (v == "CounterClockwise") return RHI::PipelineRasterizerStateFrontFace::CounterClockwise;
            TL_UNREACHABLE_MSG("Invalid FrontFace string value");
            return RHI::PipelineRasterizerStateFrontFace::Clockwise;
        }

        inline static RHI::ColorAttachmentBlendStateDesc parseColorAttachmentBlendStateDesc(const nlohmann::json& json)
        {
            TL_ASSERT(json.is_object(), "ColorAttachmentBlendStateDesc must be a JSON object");
            RHI::ColorAttachmentBlendStateDesc desc{};
            if (json.contains("blendEnable")) desc.blendEnable = json.at("blendEnable").get<bool>();
            if (json.contains("colorBlendOp")) desc.colorBlendOp = parseBlendEquation(json.at("colorBlendOp"));
            if (json.contains("srcColor")) desc.srcColor = parseBlendFactor(json.at("srcColor"));
            if (json.contains("dstColor")) desc.dstColor = parseBlendFactor(json.at("dstColor"));
            if (json.contains("alphaBlendOp")) desc.alphaBlendOp = parseBlendEquation(json.at("alphaBlendOp"));
            if (json.contains("srcAlpha")) desc.srcAlpha = parseBlendFactor(json.at("srcAlpha"));
            if (json.contains("dstAlpha")) desc.dstAlpha = parseBlendFactor(json.at("dstAlpha"));
            if (json.contains("writeMask")) desc.writeMask = parseColorWriteMask(json.at("writeMask"));
            return desc;
        }

        inline static RHI::PipelineRasterizerStateDesc parsePipelineRasterizerStateDesc(const nlohmann::json& json)
        {
            TL_ASSERT(json.is_object(), "PipelineRasterizerStateDesc must be a JSON object");
            RHI::PipelineRasterizerStateDesc desc{};
            if (json.contains("cullMode")) desc.cullMode = parsePipelineRasterizerStateCullMode(json.at("cullMode"));
            if (json.contains("fillMode")) desc.fillMode = parsePipelineRasterizerStateFillMode(json.at("fillMode"));
            if (json.contains("frontFace")) desc.frontFace = parsePipelineRasterizerStateFrontFace(json.at("frontFace"));
            if (json.contains("lineWidth")) desc.lineWidth = json.at("lineWidth").get<float>();
            return desc;
        }

        inline static RHI::PipelineDepthStencilStateDesc parsePipelineDepthStencilState(const nlohmann::json& json)
        {
            TL_ASSERT(json.is_object(), "PipelineDepthStencilStateDesc must be a JSON object");
            RHI::PipelineDepthStencilStateDesc desc{};
            if (json.contains("depthTestEnable")) desc.depthTestEnable = json.at("depthTestEnable").get<bool>();
            if (json.contains("depthWriteEnable")) desc.depthWriteEnable = json.at("depthWriteEnable").get<bool>();
            if (json.contains("compareOperator")) desc.compareOperator = parseCompareOperator(json.at("compareOperator"));
            if (json.contains("stencilTestEnable")) desc.stencilTestEnable = json.at("stencilTestEnable").get<bool>();
            return desc;
        }

        struct ShaderStage
        {
            TL::String         entry;
            RHI::ShaderStage   stage;
            RHI::ShaderModule* module;
        };

        inline static ShaderStage parseShaderStage(const nlohmann::json& json, RHI::ShaderStage stage)
        {
            TL_ASSERT(json.is_object(), "ShaderStage must be a JSON object");
            TL_ASSERT(json.contains("entry"), "ShaderStage JSON must contain 'entry'");
            TL_ASSERT(json.contains("path"), "ShaderStage JSON must contain 'path'");
            return ShaderStage{
                .entry  = json.at("entry").get<std::string>(),
                .stage  = stage,
                .module = PipelineLibrary::ptr->LoadShaderModule(json.at("path").get<std::string>()),
            };
        }

        RHI::PipelineColorBlendStateDesc parsePipelineColorBlendState(const nlohmann::json& json)
        {
            TL_ASSERT(json.is_object(), "colorBlendState must be a JSON object");

            RHI::PipelineColorBlendStateDesc desc{};

            if (json.is_object() && json.contains("blendStates"))
            {
                const auto& blendStatesJson = json.at("blendStates");
                TL_ASSERT(blendStatesJson.is_array(), "blendStates must be an array");
                m_blendState.clear();
                for (const auto& attch : blendStatesJson)
                {
                    m_blendState.push_back(parseColorAttachmentBlendStateDesc(attch));
                }
            }

            // Assign span to the member storage
            desc.blendStates = m_blendState;

            // Parse blendConstants if present
            if (json.is_object() && json.contains("blendConstants"))
            {
                const auto& blendConstantsJson = json.at("blendConstants");
                TL_ASSERT(blendConstantsJson.is_array(), "blendConstants must be an array");
                for (size_t i = 0; i < std::min<size_t>(4, blendConstantsJson.size()); ++i)
                {
                    desc.blendConstants[i] = blendConstantsJson.at(i).get<float>();
                }
            }

            return desc;
        }

        RHI::GraphicsPipelineCreateInfo parsePipelineDesc(TL::StringView path, TL::Span<const RHI::PipelineVertexBindingDesc> vertexAttribute, const RHI::PipelineRenderTargetLayout& rtLayout, const RHI::PipelineMultisampleStateDesc& msaa = {})
        {
            auto           jsonString = readFile(path);
            nlohmann::json json;
            try
            {
                json = nlohmann::json::parse(jsonString);
            }
            catch (const nlohmann::json::parse_error& e)
            {
                TL_ASSERT(false, "Failed to parse JSON: %s", e.what());
            }
            m_name         = json.value("name", "");
            m_vertexShader = parseShaderStage(json.at("vs"), RHI::ShaderStage::Vertex);
            m_pixelShader  = parseShaderStage(json.at("ps"), RHI::ShaderStage::Pixel);

            RHI::GraphicsPipelineCreateInfo ci{
                .name                 = m_name.c_str(),
                .vertexShaderName     = m_vertexShader.entry.c_str(),
                .vertexShaderModule   = m_vertexShader.module,
                .pixelShaderName      = m_pixelShader.entry.c_str(),
                .pixelShaderModule    = m_pixelShader.module,
                // .layout               = nullptr,
                .vertexBufferBindings = vertexAttribute,
                .renderTargetLayout   = rtLayout,
                .multisampleState     = msaa,
            };

            if (json.contains("colorBlendState"))
            {
                ci.colorBlendState = parsePipelineColorBlendState(json.at("colorBlendState"));
            }

            if (json.contains("topologyMode"))
            {
                ci.topologyMode = parsePipelineTopologyMode(json.at("topologyMode"));
            }

            if (json.contains("rasterizationState"))
            {
                ci.rasterizationState = parsePipelineRasterizerStateDesc(json.at("rasterizationState"));
            }

            if (json.contains("depthStencilState"))
            {
                ci.depthStencilState = parsePipelineDepthStencilState(json.at("depthStencilState"));
            }

            return ci;
        }

    private:
        TL::String                                     m_name;
        TL::Vector<RHI::ColorAttachmentBlendStateDesc> m_blendState;
        ShaderStage                                    m_vertexShader;
        ShaderStage                                    m_pixelShader;
    };

    TL::Error PipelineLibrary::init(RHI::Device* device)
    {
        m_device = device;

        return TL::NoError;
    }

    void PipelineLibrary::shutdown()
    {
    }

    TL::Ptr<GraphicsShader> PipelineLibrary::acquireGraphicsPipeline(
        TL::StringView                                 view,
        RHI::BindGroupLayout*                          layout,
        TL::Span<const RHI::PipelineVertexBindingDesc> vertexBindings,
        RHI::PipelineRenderTargetLayout                renderPassLayout)
    {
        PipelineStateInitializer psoInitializer;
        auto                     ci = psoInitializer.parsePipelineDesc(view, vertexBindings, renderPassLayout);
        ci.layout                   = m_device->CreatePipelineLayout({.name = view.data(), .layouts = {layout}});

        auto pipeline = m_device->CreateGraphicsPipeline(ci);
        return TL::CreatePtr<GraphicsShader>(layout, ci.layout, pipeline);
    }

    TL::Ptr<ComputeShader> PipelineLibrary::acquireComputePipeline(TL::StringView view, RHI::BindGroupLayout* layout)
    {
        auto json                   = nlohmann::json::parse(readFile(view));
        auto name                   = json.value("name", "");
        auto [entry, stage, shader] = PipelineStateInitializer::parseShaderStage(json.at("cs"), RHI::ShaderStage::Compute);

        RHI::ComputePipelineCreateInfo ci{
            .name         = name.c_str(),
            .shaderName   = entry.c_str(),
            .shaderModule = shader,
            .layout       = m_device->CreatePipelineLayout({.name = name.c_str(), .layouts = {layout}}),
        };
        auto pipeline = m_device->CreateComputePipeline(ci);
        return TL::CreatePtr<ComputeShader>(layout, ci.layout, pipeline);
    }

    RHI::ShaderModule* PipelineLibrary::LoadShaderModule(TL::StringView path)
    {
        auto file       = TL::File(path, TL::IOMode::Read);
        auto shaderBlob = TL::Vector<uint8_t>(file.size());
        auto [_, err]   = file.read(TL::Block::create(shaderBlob));
        TL_ASSERT(err == TL::IOResultCode::Success);
        auto module = PipelineLibrary::ptr->m_device->CreateShaderModule({
            .name = path.data(),
            .code = {(uint32_t*)shaderBlob.data(), shaderBlob.size() / size_t(4)},
        });
        return module;
    }

    void PipelineLibrary::updatePipelinesIfChanged()
    {
    }

} // namespace Engine