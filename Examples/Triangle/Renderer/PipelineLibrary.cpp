#include "PipelineLibrary.hpp"

#include <TL/FileSystem/FileSystem.hpp>

#include <glm/glm.hpp>

#include "Shaders/Public/GPU.h"

#include "Passes/GBufferPass.hpp"

#include <TL/Defer.hpp>

namespace Engine
{

    inline static RHI::ShaderModule* LoadShaderModule(RHI::Device* device, const char* path)
    {
        auto code   = TL::ReadBinaryFile(path);
        // NOTE: Code might not be correctly aligned here?
        auto module = device->CreateShaderModule({
            .name = path,
            .code = {(uint32_t*)code.ptr, code.size / 4},
        });
        TL::Allocator::Release(code, 1);
        return module;
    }

    inline static RHI::Handle<RHI::GraphicsPipeline> CreateGraphicsPipeline(RHI::Device* device, RHI::Handle<RHI::PipelineLayout> layout, const char* name)
    {
        auto vertexShaderPath   = std::format("{}.vertex.spv", name);
        auto fragmentShaderPath = std::format("{}.fragment.spv", name);

        auto vertexModule = LoadShaderModule(device, vertexShaderPath.c_str());
        auto pixelModule  = LoadShaderModule(device, fragmentShaderPath.c_str());

        TL_defer
        {
            device->DestroyShaderModule(vertexModule);
            device->DestroyShaderModule(pixelModule);
        };
        RHI::ColorAttachmentBlendStateDesc attachmentBlendDesc =
            {
                true,
                RHI::BlendEquation::Add,
                RHI::BlendFactor::SrcAlpha,
                RHI::BlendFactor::OneMinusSrcAlpha,
                RHI::BlendEquation::Add,
                RHI::BlendFactor::One,
                RHI::BlendFactor::OneMinusSrcAlpha,
                RHI::ColorWriteMask::All,
            };
        RHI::GraphicsPipelineCreateInfo pipelineCI{
            .name                 = name,
            .vertexShaderName     = "VSMain",
            .vertexShaderModule   = vertexModule,
            .pixelShaderName      = "PSMain",
            .pixelShaderModule    = pixelModule,
            .layout               = layout,
            .vertexBufferBindings = {
                                     // Position
                {
                    .stride     = sizeof(glm::vec3),
                    .attributes = {{.format = RHI::Format::RGB32_FLOAT}},
                },
                                     // Normal
                {
                    .stride     = sizeof(glm::vec3),
                    .attributes = {{.format = RHI::Format::RGB32_FLOAT}},
                },
                                     // Texcoord
                {
                    .stride     = sizeof(glm::vec2),
                    .attributes = {{.format = RHI::Format::RG32_FLOAT}},
                },
                                     // Draw Instance Info
                {
                    .stride     = sizeof(glm::mat4),
                    .stepRate   = RHI::PipelineVertexInputRate::PerInstance,
                    .attributes = {{.format = RHI::Format::RGBA32_FLOAT}, {.format = RHI::Format::RGBA32_FLOAT}, {.format = RHI::Format::RGBA32_FLOAT}, {.format = RHI::Format::RGBA32_FLOAT}},
                },
                                     },
            .renderTargetLayout = {
                                     .colorAttachmentsFormats = {GBufferPass::FormatPosition, GBufferPass::FormatNormal, GBufferPass::FormatMaterial},
                                     .depthAttachmentFormat   = GBufferPass::FormatDepth,
                                     },
            .colorBlendState = {
                                     .blendStates    = {attachmentBlendDesc, attachmentBlendDesc, attachmentBlendDesc},
                                     .blendConstants = {},
                                     },
            .rasterizationState{
                                     .cullMode  = RHI::PipelineRasterizerStateCullMode::BackFace,
                                     .frontFace = RHI::PipelineRasterizerStateFrontFace::Clockwise,
                                     },
            .depthStencilState = {
                                     .depthTestEnable  = true,
                                     .depthWriteEnable = true,
                                     },
        };
        return device->CreateGraphicsPipeline(pipelineCI);
    }

    RHI::Handle<RHI::ComputePipeline> CreateComputePipeline(RHI::Device* device, RHI::Handle<RHI::PipelineLayout> layout, const char* name)
    {
        auto computeShaderPath = std::format("{}.compute.spv", name);
        auto computeModule     = LoadShaderModule(device, computeShaderPath.c_str());

        RHI::ComputePipelineCreateInfo pipelineCI{
            .name         = name,
            .shaderName   = "CSMain",
            .shaderModule = computeModule,
            .layout       = layout,
        };
        return device->CreateComputePipeline(pipelineCI);
    }

    ResultCode PipelineLibrary::Init(RHI::Device* device)
    {
        PipelineLibrary::ptr = this;

        m_device = device;

        // struct ShaderBindingTable
        // {
        //     ConstantBuffer<GPU::SceneView>              sceneView;                // 0
        //     StructuredBuffer<GPU::DrawRequest>          drawRequests;             // 1
        //     StructuredBuffer<GPU::StaticMeshIndexed>    indexedMeshes;            // 2
        //     StructuredBuffer<float4x4>                  transforms;               // 3
        //     StructuredBuffer<GPU::MeshMaterialBindless> materials;                // 4
        //     RWStructuredBuffer<U32>                     drawParametersCount;      // 5
        //     RWStructuredBuffer<GPU::DrawIndexedParameters> outDrawParameters;     // 6
        //     Texture2D                                   bindlessTextures[];       // 7
        // };

        RHI::BindGroupLayoutCreateInfo bindGroupLayoutCI = {
            .name     = "BGL-Bindless",
            .bindings = {
                         {RHI::BindingType::UniformBuffer, RHI::Access::Read, 1, RHI::ShaderStage::AllGraphics, sizeof(GPU::SceneView)},
                         {RHI::BindingType::StorageBuffer, RHI::Access::Read, 1, RHI::ShaderStage::AllStages},
                         {RHI::BindingType::StorageBuffer, RHI::Access::Read, 1, RHI::ShaderStage::AllStages},
                         {RHI::BindingType::StorageBuffer, RHI::Access::Read, 1, RHI::ShaderStage::AllStages},
                         {RHI::BindingType::StorageBuffer, RHI::Access::Read, 1, RHI::ShaderStage::AllStages},
                         {RHI::BindingType::StorageBuffer, RHI::Access::Read, 1, RHI::ShaderStage::AllStages},
                         {RHI::BindingType::StorageBuffer, RHI::Access::Write, 1, RHI::ShaderStage::AllStages},
                         {RHI::BindingType::StorageBuffer, RHI::Access::Write, 1, RHI::ShaderStage::AllStages},
                         {RHI::BindingType::SampledImage, RHI::Access::Read, RHI::BindlessArraySize, RHI::ShaderStage::AllStages},
                         },
        };
        m_bindGroupLayout = m_device->CreateBindGroupLayout(bindGroupLayoutCI);

        m_graphicsPipelineLayout = m_device->CreatePipelineLayout({"GraphicsPipelineLayout", m_bindGroupLayout});
        m_computePipelineLayout  = m_device->CreatePipelineLayout({"ComputePipelineLayout", m_bindGroupLayout});

        m_graphicsPipelines[ShaderNames::GBufferFill] = CreateGraphicsPipeline(m_device, m_graphicsPipelineLayout, ShaderNames::GBufferFill);
        m_computePipelines[ShaderNames::Cull]         = CreateComputePipeline(m_device, m_computePipelineLayout, ShaderNames::Cull);

        return ResultCode::Success;
    }

    void PipelineLibrary::Shutdown()
    {
        PipelineLibrary::ptr = nullptr;

        m_device->DestroyBindGroupLayout(m_bindGroupLayout);

        for (auto [_, pipeline] : m_graphicsPipelines)
            m_device->DestroyGraphicsPipeline(pipeline);

        for (auto [_, pipeline] : m_computePipelines)
            m_device->DestroyComputePipeline(pipeline);

        if (m_graphicsPipelineLayout) m_device->DestroyPipelineLayout(m_graphicsPipelineLayout);
        if (m_computePipelineLayout) m_device->DestroyPipelineLayout(m_computePipelineLayout);
    }

    RHI::Handle<RHI::GraphicsPipeline> PipelineLibrary::GetGraphicsPipeline(const char* name)
    {
        auto it = m_graphicsPipelines.find(name);
        return (it != m_graphicsPipelines.end()) ? it->second : RHI::NullHandle;
    }

    RHI::Handle<RHI::ComputePipeline> PipelineLibrary::GetComputePipeline(const char* name)
    {
        auto it = m_computePipelines.find(name);
        return (it != m_computePipelines.end()) ? it->second : RHI::NullHandle;
    }

} // namespace Engine