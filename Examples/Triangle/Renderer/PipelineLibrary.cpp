#include "PipelineLibrary.hpp"

#include <TL/FileSystem/FileSystem.hpp>

#include <glm/glm.hpp>

// #include <fileapi.h>
// #include <winbase.h>

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
        auto vertexModule       = LoadShaderModule(device, vertexShaderPath.c_str());
        auto pixelModule        = LoadShaderModule(device, fragmentShaderPath.c_str());

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
                // {
                //     .stride     = sizeof(glm::uvec4),
                //     .stepRate   = RHI::PipelineVertexInputRate::PerInstance,
                //     .attributes = {{.format = RHI::Format::RGBA32_UINT}},
                // },
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
        return device->CreateGraphicsPipeline(pipelineCI);
    }

    RHI::Handle<RHI::ComputePipeline> CreateComputePipeline(RHI::Device* device, RHI::Handle<RHI::PipelineLayout> layout, const char* name)
    {
        auto computeShaderPath = std::format("{}.compute.spv", name);
        auto computeModule     = LoadShaderModule(device, computeShaderPath.c_str());

        RHI::ComputePipelineCreateInfo pipelineCI{
            .name         = name,
            .shaderName   = computeShaderPath.c_str(),
            .shaderModule = computeModule,
            .layout       = layout,
        };
        return device->CreateComputePipeline(pipelineCI);
    }

    ResultCode PipelineLibrary::Init(RHI::Device* device)
    {
        m_device = device;

        RHI::BindGroupLayoutCreateInfo bindGroupLayoutCI = {
            .name     = "BGL-Bindless",
            .bindings = {
                // Global scene uniform data
                {.type = RHI::BindingType::UniformBuffer, .stages = RHI::ShaderStage::Vertex | RHI::ShaderStage::Pixel},
                // Per Draw Uniform data e.g. (Transforms, Material ID, ...etc)
                {.type = RHI::BindingType::StorageBuffer, .stages = RHI::ShaderStage::Pixel},
                // Material Properties
                {.type = RHI::BindingType::StorageBuffer, .stages = RHI::ShaderStage::Pixel},
                // Global sampler
                {.type = RHI::BindingType::Sampler, .stages = RHI::ShaderStage::Pixel},
                // Bindless Textures
                // TODO: bindless currently broken, change back to RHI::BindlessArraySize when fixed
                {.type = RHI::BindingType::SampledImage, .arrayCount = 1048576, .stages = RHI::ShaderStage::Pixel},
            },
        };
        m_gBufferBGL             = m_device->CreateBindGroupLayout(bindGroupLayoutCI);
        m_graphicsPipelineLayout = m_device->CreatePipelineLayout({.name = "GraphicsPipelineLayout", .layouts = m_gBufferBGL});

#define LOAD_SHADER(map, name, layout) \
    map[name] = CreateGraphicsPipeline(device, m_graphicsPipelineLayout, name);

        LOAD_SHADER(m_graphicsPipelines, kGBufferFill, m_graphicsPipelineLayout);

#undef LOAD_SHADER

        return ResultCode::Success;
    }

    void PipelineLibrary::Shutdown()
    {
        m_device->DestroyBindGroupLayout(m_gBufferBGL);

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

    void PipelineLibrary::ReloadInvalidatedShaders()
    {
        // // Create a handle to watch for changes in the shader directory
        // static HANDLE hChangeHandle = FindFirstChangeNotificationA(
        //     "path/to/shader/directory",   // directory to watch
        //     FALSE,                        // do not watch subdirectories
        //     FILE_NOTIFY_CHANGE_LAST_WRITE // watch for changes to the last write time
        // );

        // if (hChangeHandle == INVALID_HANDLE_VALUE)
        // {
        //     // Handle error
        //     return;
        // }

        // // Wait for a change to occur
        // DWORD dwWaitStatus = WaitForSingleObject(hChangeHandle, INFINITE);

        // switch (dwWaitStatus)
        // {
        // case WAIT_OBJECT_0:
        //     // A change has occurred, reload shaders
        //     for (auto& [name, pipeline] : m_graphicsPipelines)
        //     {
        //         m_device->DestroyGraphicsPipeline(pipeline);
        //         pipeline = CreateGraphicsPipeline(m_device, m_graphicsPipelineLayout, name.c_str());
        //     }

        //     for (auto& [name, pipeline] : m_computePipelines)
        //     {
        //         m_device->DestroyComputePipeline(pipeline);
        //         pipeline = CreateComputePipeline(m_device, m_computePipelineLayout, name.c_str());
        //     }

        //     // Reset the change notification
        //     if (FindNextChangeNotification(hChangeHandle) == FALSE)
        //     {
        //         // Handle error
        //         return;
        //     }
        //     break;

        // default:
        //     // Handle error
        //     break;
        // }

        // // Close the change notification handle
        // FindCloseChangeNotification(hChangeHandle);
    }

} // namespace Engine