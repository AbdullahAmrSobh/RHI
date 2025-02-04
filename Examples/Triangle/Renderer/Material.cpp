#include "Material.hpp"

#include "TL/FileSystem/FileSystem.hpp"

namespace Engine
{
    inline static TL::Ptr<RHI::ShaderModule> LoadShaderModule(RHI::Device* device, const char* path)
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

    RHI::Handle<RHI::GraphicsPipeline> PipelineLibrary::GetGraphicsPipeline(const char* name)
    {
        auto vertexShaderPath   = std::format("{}.vertex.spv", name);
        auto fragmentShaderPath = std::format("{}.fragment.spv", name);
        auto vertexModule       = LoadShaderModule(m_device, vertexShaderPath.c_str());
        auto pixelModule        = LoadShaderModule(m_device, fragmentShaderPath.c_str());

        RHI::GraphicsPipelineCreateInfo pipelineCI{
            .name                 = name,
            .vertexShaderName     = "VSMain",
            .vertexShaderModule   = vertexModule.get(),
            .pixelShaderName      = "PSMain",
            .pixelShaderModule    = pixelModule.get(),
            .layout               = m_graphicsPipelineLayout,
            .vertexBufferBindings = {
                // Position
                {
                    .stride     = sizeof(float) * 3,
                    .attributes = {{.format = RHI::Format::RGB32_FLOAT}},
                },
                // Normal
                {
                    .stride     = sizeof(float) * 3,
                    .attributes = {{.format = RHI::Format::RGB32_FLOAT}},
                },
                // Texcoord
                {
                    .stride     = sizeof(float) * 2,
                    .attributes = {{.format = RHI::Format::RG32_FLOAT}},
                },
            },
            .renderTargetLayout = {
                .colorAttachmentsFormats = {RHI::Format::RGBA8_UNORM, RHI::Format::RGBA32_FLOAT, RHI::Format::RGBA32_FLOAT, RHI::Format::RG8_UNORM},
                .depthAttachmentFormat   = RHI::Format::D32,
            },
            .colorBlendState = {
                .blendStates = {
                    {.blendEnable = true},
                },
            },
            .rasterizationState = {
                .cullMode  = RHI::PipelineRasterizerStateCullMode::BackFace,
                .frontFace = RHI::PipelineRasterizerStateFrontFace::Clockwise,
            },
            .depthStencilState = {
                .depthTestEnable  = true,
                .depthWriteEnable = true,
            },
        };
        auto pipeline = m_device->CreateGraphicsPipeline(pipelineCI);
    }

    RHI::Handle<RHI::ComputePipeline> PipelineLibrary::GetComputePipeline(const char* name)
    {
        auto computeShaderPath = std::format("{}.compute.spv", name);
        auto computeModule     = LoadShaderModule(m_device, computeShaderPath.c_str());

        RHI::ComputePipelineCreateInfo pipelineCI{
            .name         = name,
            .shaderName   = computeShaderPath.c_str(),
            .shaderModule = computeModule.get(),
            .layout       = m_computePipelineLayout,
        };
        auto pipeline = m_device->CreateComputePipeline(pipelineCI);
    }
} // namespace Engine