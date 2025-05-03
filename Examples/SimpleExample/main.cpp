
#include <RHI/RHI.hpp>

#include <TL/Allocator/MemPlumber.hpp>
#include <TL/Defer.hpp>
#include <TL/FileSystem/FileSystem.hpp>
#include <TL/Log.hpp>
#include <TL/Utils.hpp>

// #include <RHI-D3D12/Loader.hpp>
#include <RHI-Vulkan/Loader.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <iostream>
#include <tracy/Tracy.hpp>

// #include "CommandList.hpp"
// #include "Device.hpp"

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

int main(int argc, const char* argv[])
{
    auto device = RHI::CreateVulkanDevice({});

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    RHI::ImageSize2D windowSize = {800, 480};
    GLFWwindow*      window;
    if (window = glfwCreateWindow(windowSize.width, windowSize.height, "Hello World", NULL, NULL); window == nullptr)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    RHI::SwapchainCreateInfo swapchainCI{
        .name        = "Swapchain",
        .win32Window = {.hwnd = glfwGetWin32Window(window)},
    };
    auto swapchain = device->CreateSwapchain(swapchainCI);
    auto result    = swapchain->Configure({
           .size        = windowSize,
           .imageCount  = 2,
           .imageUsage  = RHI::ImageUsage::Color,
           .format      = RHI::Format::RGBA8_UNORM,
           .presentMode = RHI::SwapchainPresentMode::Fifo,
           .alphaMode   = RHI::SwapchainAlphaMode::PostMultiplied,
    });
    TL_ASSERT(RHI::IsSuccess(result));

    auto renderGraph = device->CreateRenderGraph({});

#if 0 // Pipeline logic
    {
        RHI::BindGroupLayoutCreateInfo bindGroupLayoutCI{
            .name     = "ImGui-BindGroupLayout",
            .bindings = {
                         RHI::ShaderBinding{.type = RHI::BindingType::UniformBuffer, .access = RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Vertex},
                         RHI::ShaderBinding{.type = RHI::BindingType::Sampler, .access = RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel},
                         RHI::ShaderBinding{.type = RHI::BindingType::SampledImage, .access = RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel}}
        };
        auto bindGroupLayout = device->CreateBindGroupLayout(bindGroupLayoutCI);

        RHI::PipelineLayoutCreateInfo pipelineLayoutCI{.layouts = {bindGroupLayout}};
        auto                          pipelineLayout = device->CreatePipelineLayout(pipelineLayoutCI);

        auto vertexShaderModule = LoadShaderModule(device, "Shaders/ImGui.vertex.spv");
        auto fragmentShader     = LoadShaderModule(device, "Shaders/ImGui.fragment.spv");

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
        RHI::GraphicsPipelineCreateInfo pipelineCI =
            {
                .name               = "ImGui Pipeline",
                .vertexShaderName   = "VSMain",
                .vertexShaderModule = vertexShaderModule,
                .pixelShaderName    = "PSMain",
                .pixelShaderModule  = fragmentShader,
                .layout             = pipelineLayout,
                .vertexBufferBindings =
                    {
                                           {
                            .stride   = 20,
                            .stepRate = RHI::PipelineVertexInputRate::PerVertex,
                            .attributes =
                                {
                                    {.offset = 0, .format = RHI::Format::RG32_FLOAT},
                                    {.offset = 8, .format = RHI::Format::RG32_FLOAT},
                                    {.offset = 16, .format = RHI::Format::RGBA8_UNORM},
                                },
                        },
                                           },
                .renderTargetLayout =
                    {
                                           .colorAttachmentsFormats = {RHI::Format::RGBA8_UNORM},
                                           .depthAttachmentFormat   = RHI::Format::D32,
                                           },
                .colorBlendState =
                    {
                                           .blendStates    = {attachmentBlendDesc},
                                           .blendConstants = {},
                                           },
                .topologyMode = RHI::PipelineTopologyMode::Triangles,
                .rasterizationState =
                    {
                                           .cullMode  = RHI::PipelineRasterizerStateCullMode::None,
                                           .fillMode  = RHI::PipelineRasterizerStateFillMode::Triangle,
                                           .frontFace = RHI::PipelineRasterizerStateFrontFace::CounterClockwise,
                                           .lineWidth = 1.0f,
                                           },
                .multisampleState =
                    {
                                           .sampleCount   = RHI::SampleCount::Samples1,
                                           .sampleShading = false,
                                           },
                .depthStencilState =
                    {
                                           .depthTestEnable   = false,
                                           .depthWriteEnable  = true,
                                           .compareOperator   = RHI::CompareOperator::Always,
                                           .stencilTestEnable = false,
                                           },
        };
        auto pipeline = device->CreateGraphicsPipeline(pipelineCI);
    }
#endif

    while (!glfwWindowShouldClose(window))
    {
        ZoneScopedN("DrawLoop");

        auto rg = renderGraph;

        bool EnableGPUDrivenPipeline      = true;
        bool EnableSSAO                   = true;
        bool EnableScreenSpaceReflections = true;
        bool EnableFXAA                   = true;
        bool EnableTAA                    = true;

        // The following should invalidate the graph, and require new setup
        // 1. adding new pass that was not added before
        // 2. changing the graph size
        // The following will only update the resource transitions (but graph may be un-optimal)
        // 1. disabling pass

        rg->BeginFrame(windowSize);
        {
            RHI::Handle<RHI::RGBuffer> indirectDrawList = rg->CreateBuffer("indirectDrawList", 1);
            RHI::Handle<RHI::RGBuffer> indirectDrawArgs;
            if (EnableGPUDrivenPipeline)
            {
                rg->AddPass({
                    .name          = "Cull",
                    .type          = RHI::PassType::Compute,
                    .size          = windowSize,
                    .setupCallback = [&](RHI::RenderGraphBuilder& builder)
                    {
                        builder.ReadBuffer(indirectDrawList, RHI::BufferUsage::Storage, RHI::PipelineStage::ComputeShader);
                        indirectDrawArgs = builder.WriteBuffer(rg->CreateBuffer("indirectDrawArgs", 1), RHI::BufferUsage::Storage, RHI::PipelineStage::ComputeShader);
                    },
                    .executeCallback = [=](RHI::CommandList& cmd)
                    {
                        // Cull compute shader dispatch
                    },
                });
            }

            RHI::Handle<RHI::RGImage> wsPositionRT, normalRT, materialRT, depthRT;
            rg->AddPass({
                .name          = "GBuffer",
                .type          = RHI::PassType::Graphics,
                .size          = windowSize,
                .setupCallback = [&](RHI::RenderGraphBuilder& builder)
                {
                    if (EnableGPUDrivenPipeline)
                    {
                        builder.ReadBuffer(indirectDrawList, RHI::BufferUsage::Indirect, RHI::PipelineStage::VertexAttributeInput);
                        builder.ReadBuffer(indirectDrawArgs, RHI::BufferUsage::Indirect, RHI::PipelineStage::DrawIndirect);
                    }

                    wsPositionRT = builder.AddColorAttachment({.color = rg->CreateRenderTarget("wsPosition", windowSize, RHI::Format::RGBA32_FLOAT)});
                    normalRT     = builder.AddColorAttachment({.color = rg->CreateRenderTarget("normal", windowSize, RHI::Format::RGBA32_FLOAT)});
                    materialRT   = builder.AddColorAttachment({.color = rg->CreateRenderTarget("material", windowSize, RHI::Format::RGBA32_FLOAT)});
                    depthRT      = builder.SetDepthStencil({.depthStencil = rg->CreateRenderTarget("depth", windowSize, RHI::Format::D32)});
                },
                .executeCallback = [=](RHI::CommandList& cmd)
                {
                    // GBuffer rendering
                },
            });

            // Shadow Pass (up to 6 cascades)
            TL::Vector<RHI::Handle<RHI::RGImage>> shadowMaps;
            for (int i = 0; i < 6; ++i)
            {
                auto name      = std::format("shadowMap[{}]", i);
                auto shadowMap = rg->CreateRenderTarget(name.c_str(), windowSize, RHI::Format::D32);
                shadowMaps.push_back(shadowMap);
                rg->AddPass({
                    .name          = "Shadow",
                    .type          = RHI::PassType::Graphics,
                    .size          = windowSize,
                    .setupCallback = [&, shadowMap](RHI::RenderGraphBuilder& builder)
                    {
                        shadowMaps[i] = builder.SetDepthStencil({.depthStencil = shadowMap});
                        // Read scene geometry buffers if needed
                    },
                    .executeCallback = [=](RHI::CommandList& cmd)
                    {
                        // Shadow map rendering
                    },
                });
            }

            // Lighting Pass
            RHI::Handle<RHI::RGImage> lightingRT;
            rg->AddPass({
                .name          = "Lighting",
                .type          = RHI::PassType::Graphics,
                .size          = windowSize,
                .setupCallback = [&](RHI::RenderGraphBuilder& builder)
                {
                    builder.ReadImage(wsPositionRT, RHI::ImageUsage::ShaderResource, RHI::PipelineStage::PixelShader);
                    builder.ReadImage(normalRT, RHI::ImageUsage::ShaderResource, RHI::PipelineStage::PixelShader);
                    builder.ReadImage(materialRT, RHI::ImageUsage::ShaderResource, RHI::PipelineStage::PixelShader);
                    builder.ReadImage(depthRT, RHI::ImageUsage::ShaderResource, RHI::PipelineStage::PixelShader);
                    for (auto& shadowMap : shadowMaps)
                        builder.ReadImage(shadowMap, RHI::ImageUsage::ShaderResource, RHI::PipelineStage::PixelShader);
                    lightingRT = builder.AddColorAttachment({.color = rg->CreateRenderTarget("lighting", windowSize, RHI::Format::RGBA16_FLOAT)});
                },
                .executeCallback = [=](RHI::CommandList& cmd)
                {
                    // Lighting calculations
                },
            });

            // SSAO Pass (optional)
            RHI::Handle<RHI::RGImage> ssaoRT;
            if (EnableSSAO)
            {
                rg->AddPass({
                    .name          = "SSAO",
                    .type          = RHI::PassType::Compute,
                    .size          = windowSize,
                    .setupCallback = [&](RHI::RenderGraphBuilder& builder)
                    {
                        builder.ReadImage(normalRT, RHI::ImageUsage::ShaderResource, RHI::PipelineStage::ComputeShader);
                        builder.ReadImage(depthRT, RHI::ImageUsage::ShaderResource, RHI::PipelineStage::ComputeShader);

                        ssaoRT = rg->CreateRenderTarget("ssao", windowSize, RHI::Format::R8_UNORM);
                        ssaoRT = builder.WriteImage(ssaoRT, RHI::ImageUsage::StorageResource, RHI::PipelineStage::ComputeShader);
                    },
                    .executeCallback = [=](RHI::CommandList& cmd)
                    {
                        // SSAO compute
                    },
                });
            }

            // Screen Space Reflections Pass (optional)
            RHI::Handle<RHI::RGImage> ssrRT;
            if (EnableScreenSpaceReflections)
            {
                ssrRT = rg->CreateRenderTarget("ssr", windowSize, RHI::Format::RGBA16_FLOAT);
                rg->AddPass({
                    .name          = "SSR",
                    .type          = RHI::PassType::Compute,
                    .size          = windowSize,
                    .setupCallback = [&](RHI::RenderGraphBuilder& builder)
                    {
                        builder.ReadImage(wsPositionRT, RHI::ImageUsage::ShaderResource, RHI::PipelineStage::ComputeShader);
                        builder.ReadImage(normalRT, RHI::ImageUsage::ShaderResource, RHI::PipelineStage::ComputeShader);
                        builder.ReadImage(depthRT, RHI::ImageUsage::ShaderResource, RHI::PipelineStage::ComputeShader);
                        builder.WriteImage(ssrRT, RHI::ImageUsage::StorageResource, RHI::PipelineStage::ComputeShader);
                    },
                    .executeCallback = [=](RHI::CommandList& cmd)
                    {
                        // SSR compute
                    },
                });
            }

            // FXAA/TAA/Tonemap Passes (chained, writing to the same handle)
            RHI::Handle<RHI::RGImage> postProcessInput = lightingRT;
            RHI::Handle<RHI::RGImage> postProcessOutput;

            // FXAA Pass (optional)
            if (EnableFXAA)
            {
                postProcessOutput = rg->CreateRenderTarget("fxaa", windowSize, RHI::Format::RGBA8_UNORM);
                rg->AddPass({
                    .name          = "FXAA",
                    .type          = RHI::PassType::Compute,
                    .size          = windowSize,
                    .setupCallback = [&](RHI::RenderGraphBuilder& builder)
                    {
                        builder.ReadImage(postProcessInput, RHI::ImageUsage::ShaderResource, RHI::PipelineStage::ComputeShader);
                        postProcessOutput = builder.WriteImage(postProcessOutput, RHI::ImageUsage::StorageResource, RHI::PipelineStage::ComputeShader);
                    },
                    .executeCallback = [=](RHI::CommandList& cmd)
                    {
                        // FXAA compute
                    },
                });
                postProcessInput = postProcessOutput;
            }

            // TAA Pass (optional)
            if (EnableTAA)
            {
                postProcessOutput = rg->CreateRenderTarget("taa", windowSize, RHI::Format::RGBA8_UNORM);
                rg->AddPass({
                    .name          = "TAA",
                    .type          = RHI::PassType::Compute,
                    .size          = windowSize,
                    .setupCallback = [&](RHI::RenderGraphBuilder& builder)
                    {
                        builder.ReadImage(postProcessInput, RHI::ImageUsage::ShaderResource, RHI::PipelineStage::ComputeShader);
                        postProcessOutput = builder.WriteImage(postProcessOutput, RHI::ImageUsage::StorageResource, RHI::PipelineStage::ComputeShader);
                    },
                    .executeCallback = [=](RHI::CommandList& cmd)
                    {
                        // TAA compute
                    },
                });
                postProcessInput = postProcessOutput;
            }

            // Tonemap Pass
            RHI::Handle<RHI::RGImage> swapchainRT = rg->ImportSwapchain("final", *swapchain, RHI::Format::RGBA8_UNORM);
            rg->AddPass({
                .name          = "Tonemap",
                .type          = RHI::PassType::Graphics,
                .size          = windowSize,
                .setupCallback = [&](RHI::RenderGraphBuilder& builder)
                {
                    builder.ReadImage(postProcessInput, RHI::ImageUsage::ShaderResource, RHI::PipelineStage::PixelShader);
                    swapchainRT = builder.AddColorAttachment({.color = swapchainRT});
                },
                .executeCallback = [=](RHI::CommandList& cmd)
                {
                    // Tonemapping shader
                },
            });
        }
        rg->EndFrame();

        glfwPollEvents();
        FrameMark;
    }

    device->DestroySwapchain(swapchain);
    RHI::DestroyVulkanDevice(device);
    glfwTerminate();
    return 0;
}