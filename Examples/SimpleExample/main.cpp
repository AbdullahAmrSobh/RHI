
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

#include <glm/glm.hpp>

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
    TL::Release(code, 1);
    return module;
}

int main(int argc, const char* argv[])
{
    // TL::MemPlumber::start();

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
           .imageCount  = 1,
           .imageUsage  = RHI::ImageUsage::Color,
           .format      = RHI::Format::RGBA8_UNORM,
           .presentMode = RHI::SwapchainPresentMode::Fifo,
           .alphaMode   = RHI::SwapchainAlphaMode::None,
    });
    TL_ASSERT(RHI::IsSuccess(result));

    auto renderGraph = device->CreateRenderGraph({});

    struct CBSceneView
    {
        glm::mat4 modelToView;
        glm::mat4 viewToClip;
        glm::mat4 modelToClip;
        glm::mat4 clipToView;
        glm::mat4 clipToLocal;
    };

    // // Test bind groups
    // struct BindGroup0
    // {
    //     ConstantBuffer<CBSceneView> sceneView;
    //     StructuredBuffer<uint32_t> ib;
    //     StructuredBuffer<float3> positionsVB;
    //     StructuredBuffer<float3> normalsVB;
    //     StructuredBuffer<float3> texcoordsVB;
    //     SamplerState             sampler;
    //     Texture2D                textures[];
    // };

    RHI::BindGroupLayoutCreateInfo bindGroupLayoutCI{
        .name     = "default-BindGroupLayout",
        .bindings = {
                     {RHI::BindingType::UniformBuffer, RHI::Access::Read, 1, RHI::ShaderStage::AllGraphics, sizeof(CBSceneView)},
                     {RHI::BindingType::StorageBuffer, RHI::Access::Read, 1, RHI::ShaderStage::AllGraphics},
                     {RHI::BindingType::StorageBuffer, RHI::Access::Read, 1, RHI::ShaderStage::AllGraphics},
                     {RHI::BindingType::StorageBuffer, RHI::Access::Read, 1, RHI::ShaderStage::AllGraphics},
                     {RHI::BindingType::StorageBuffer, RHI::Access::Read, 1, RHI::ShaderStage::AllGraphics},
                     {RHI::BindingType::Sampler, RHI::Access::Read, 1, RHI::ShaderStage::Pixel},
                     {RHI::BindingType::SampledImage, RHI::Access::Read, RHI::BindlessArraySize, RHI::ShaderStage::Pixel},
                     }
    };
    auto bindGroupLayout = device->CreateBindGroupLayout(bindGroupLayoutCI);
    TL_defer
    {
        device->DestroyBindGroupLayout(bindGroupLayout);
    };

    RHI::PipelineLayoutCreateInfo pipelineLayoutCI{.name = "default-PipelineLayout", .layouts = {bindGroupLayout}};
    auto                          pipelineLayout = device->CreatePipelineLayout(pipelineLayoutCI);

    auto vertexShaderModule = LoadShaderModule(device, "Shaders/Triangle.vertex.spv");
    auto fragmentShader     = LoadShaderModule(device, "Shaders/Triangle.fragment.spv");

    RHI::Handle<RHI::Buffer> uniformBuffer = device->CreateBuffer({
        .name       = "default-UniformBuffer",
        .hostMapped = true,
        .usageFlags = RHI::BufferUsage::Uniform,
        .byteSize   = sizeof(CBSceneView),
    });

    RHI::Handle<RHI::Buffer> ib = device->CreateBuffer({
        .name       = "default-IndexBuffer",
        .hostMapped = true,
        .usageFlags = RHI::BufferUsage::Index | RHI::BufferUsage::Storage,
        .byteSize   = 1024 * 1024 * sizeof(uint32_t),
    });

    RHI::Handle<RHI::Buffer> positionsIB = device->CreateBuffer({
        .name       = "default-VertexBuffer",
        .hostMapped = true,
        .usageFlags = RHI::BufferUsage::Vertex | RHI::BufferUsage::Storage,
        .byteSize   = 1024 * 1024 * sizeof(uint32_t),
    });

    RHI::Handle<RHI::Buffer> normalsIB = device->CreateBuffer({
        .name       = "default-NormalBuffer",
        .hostMapped = true,
        .usageFlags = RHI::BufferUsage::Vertex | RHI::BufferUsage::Storage,
        .byteSize   = 1024 * 1024 * sizeof(uint32_t),
    });

    RHI::Handle<RHI::Buffer> texcoordsIB = device->CreateBuffer({
        .name       = "default-TexcoordBuffer",
        .hostMapped = true,
        .usageFlags = RHI::BufferUsage::Vertex | RHI::BufferUsage::Storage,
        .byteSize   = 1024 * 1024 * sizeof(uint32_t),
    });

    RHI::Handle<RHI::Sampler> sampler = device->CreateSampler({
        .name = "default-Sampler",
    });

    auto bindGroup = device->CreateBindGroup({
        .name               = "default-BindGroup",
        .layout             = bindGroupLayout,
        .bindlessArrayCount = 2048,
    });

    RHI::GraphicsPipelineCreateInfo pipelineCI = {
        .name               = "ImGui Pipeline",
        .vertexShaderName   = "VSMain",
        .vertexShaderModule = vertexShaderModule,
        .pixelShaderName    = "PSMain",
        .pixelShaderModule  = fragmentShader,
        .layout             = pipelineLayout,
        .renderTargetLayout = {
                               .colorAttachmentsFormats = {RHI::Format::RGBA8_UNORM, RHI::Format::RGBA8_UNORM},
                               .depthAttachmentFormat   = RHI::Format::D32,
                               },
        .depthStencilState = {
                               .depthTestEnable   = false,
                               .depthWriteEnable  = true,
                               .compareOperator   = RHI::CompareOperator::Always,
                               .stencilTestEnable = false,
                               },
    };
    auto pipeline = device->CreateGraphicsPipeline(pipelineCI);
    device->DestroyShaderModule(vertexShaderModule);
    device->DestroyShaderModule(fragmentShader);

    // #if 0
    while (!glfwWindowShouldClose(window))
    {
        ZoneScopedN("DrawLoop");

        auto        rg                           = renderGraph;
        static bool EnableGPUDrivenPipeline      = true;
        static bool EnableSSAO                   = true;
        static bool EnableScreenSpaceReflections = true;
        static bool EnableFXAA                   = true;
        static bool EnableTAA                    = true;

        static bool GraphDumpEnabled = false;

        // Keyboard toggles: Q=FXAA, W=TAA, E=SSAO
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            EnableFXAA = !EnableFXAA;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            EnableTAA = !EnableTAA;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            EnableSSAO = !EnableSSAO;
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            GraphDumpEnabled = true;

        // The following should invalidate the graph, and require new setup
        // 1. adding new pass that was not added before
        // 2. changing the graph size
        // The following will only update the resource transitions (but graph may be un-optimal)
        // 1. disabling pass

        rg->BeginFrame(windowSize);
#if 0
        {
            RHI::RGBuffer* indirectDrawList = rg->CreateBuffer("indirectDrawList", 1);
            RHI::RGBuffer* indirectDrawArgs;
            if (EnableGPUDrivenPipeline)
            {
                rg->AddPass({
                    .name          = "Cull",
                    .type          = RHI::PassType::Compute,
                    .size          = windowSize,
                    .setupCallback = [&](RHI::RenderGraphBuilder& builder)
                    {
                        indirectDrawList = builder.WriteBuffer(indirectDrawList, RHI::BufferUsage::Storage, RHI::PipelineStage::ComputeShader);
                        indirectDrawArgs = builder.WriteBuffer(rg->CreateBuffer("indirectDrawArgs", 1), RHI::BufferUsage::Storage, RHI::PipelineStage::ComputeShader);
                    },
                    .compileCallback = [&](RHI::RenderGraphContext& context)
                    {
                        TL_LOG_INFO("Cull pass compiled");
                    },
                    .executeCallback = [=](RHI::CommandList& cmd)
                    {
                        // Cull compute shader dispatch
                    },
                });
            }

            RHI::RGImage* wsPositionRT, normalRT, materialRT, depthRT;
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
                .compileCallback = [&](RHI::RenderGraphContext& context)
                {
                    TL_LOG_INFO("GBuffer pass compiled");
                },
                .executeCallback = [=](RHI::CommandList& cmd)
                {
                    // GBuffer rendering
                },
            });

            // Shadow Pass (up to 6 cascades)
            TL::Vector<RHI::RGImage>* shadowMaps;
            for (int i = 0; i < 6; ++i)
            {
                auto passName  = std::format("Shadow[{}]", i);
                auto name      = std::format("shadowMap[{}]", i);
                auto shadowMap = rg->CreateRenderTarget(name.c_str(), windowSize, RHI::Format::D32);
                shadowMaps.push_back(shadowMap);
                rg->AddPass({
                    .name          = passName.c_str(),
                    .type          = RHI::PassType::Graphics,
                    .size          = windowSize,
                    .setupCallback = [&, shadowMap](RHI::RenderGraphBuilder& builder)
                    {
                        shadowMaps[i] = builder.SetDepthStencil({.depthStencil = shadowMap});
                        // Read scene geometry buffers if needed
                    },
                    .compileCallback = [=](RHI::RenderGraphContext& context)
                    {
                        TL_LOG_INFO("Shadow pass compiled");
                    },
                    .executeCallback = [=](RHI::CommandList& cmd)
                    {
                        // Shadow map rendering
                    },
                });
            }

            // Lighting Pass
            RHI::RGImage* lightingRT;
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
                .compileCallback = [&](RHI::RenderGraphContext& context)
                {
                    TL_LOG_INFO("Lighting pass compiled");
                },
                .executeCallback = [=](RHI::CommandList& cmd)
                {
                    // Lighting calculations
                },
            });

            // SSAO Pass (optional)
            RHI::RGImage* ssaoRT;
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
                    .compileCallback = [=](RHI::RenderGraphContext& context)
                    {
                        TL_LOG_INFO("SSAO pass compiled");
                    },
                    .executeCallback = [=](RHI::CommandList& cmd)
                    {
                        // SSAO compute
                    },
                });
            }

            // Screen Space Reflections Pass (optional)
            RHI::RGImage* ssrRT;
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
                    .compileCallback = [=](RHI::RenderGraphContext& context)
                    {
                        TL_LOG_INFO("SSR pass compiled");
                    },
                    .executeCallback = [=](RHI::CommandList& cmd)
                    {
                        // SSR compute
                    },
                });
            }

            // FXAA/TAA/Tonemap Passes (chained, writing to the same handle)
            RHI::RGImage* postProcessInput = lightingRT;
            RHI::RGImage* postProcessOutput;

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
                    .compileCallback = [=](RHI::RenderGraphContext& context)
                    {
                        TL_LOG_INFO("FXAA pass compiled");
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
                    .compileCallback = [=](RHI::RenderGraphContext& context)
                    {
                        TL_LOG_INFO("TAA pass compiled");
                    },
                    .executeCallback = [=](RHI::CommandList& cmd)
                    {
                        // TAA compute
                    },
                });
                postProcessInput = postProcessOutput;
            }

            // Tonemap Pass
            RHI::RGImage* swapchainRT = rg->ImportSwapchain("final", *swapchain, RHI::Format::RGBA8_UNORM);
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
#else
        // Simple hello-world triangle
        {
            auto pass = rg->AddPass({
                .name          = "HelloWorld",
                .type          = RHI::PassType::Graphics,
                .size          = windowSize,
                .setupCallback = [&](RHI::RenderGraphBuilder& builder)
                {
                    auto colorAttachment   = rg->ImportSwapchain("color", *swapchain, RHI::Format::RGBA8_UNORM);
                    auto anotherAttachment = rg->CreateRenderTarget("another", windowSize, RHI::Format::RGBA8_UNORM);
                    builder.AddColorAttachment({.color = colorAttachment, .clearValue = {.f32{0.1f, 0.1f, 0.4f, 1.0f}}});
                    builder.AddColorAttachment({.color = anotherAttachment, .clearValue = {.f32{0.1f, 0.1f, 0.4f, 1.0f}}});
                },
                .executeCallback = [&](RHI::CommandList& cmd)
                {
                    RHI::BindGroupUpdateInfo updateInfo{
                        .buffers = {
                                    {0, 0, RHI::BufferBindingInfo{uniformBuffer, 0}},
                                    {1, 0, RHI::BufferBindingInfo{ib, 0}},
                                    {2, 0, RHI::BufferBindingInfo{positionsIB, 0}},
                                    {3, 0, RHI::BufferBindingInfo{normalsIB, 0}},
                                    {4, 0, RHI::BufferBindingInfo{texcoordsIB, 0}},
                                    },
                        .samplers = {
                                    {5, 0, sampler},
                                    },
                        // .images = {},
                    };

                    device->UpdateBindGroup(bindGroup, updateInfo);

                    cmd.SetViewport({
                        .offsetX  = 0,
                        .offsetY  = 0,
                        .width    = (float)windowSize.width,
                        .height   = (float)windowSize.height,
                        .minDepth = 0.0,
                        .maxDepth = 1.0,
                    });
                    cmd.SetScissor({
                        .offsetX = 0,
                        .offsetY = 0,
                        .width   = windowSize.width,
                        .height  = windowSize.height,
                    });
                    cmd.BindGraphicsPipeline(pipeline, RHI::BindGroupBindingInfo{bindGroup});
                    cmd.Draw({
                        .vertexCount   = 3,
                        .instanceCount = 1,
                        .firstVertex   = 0,
                        .firstInstance = 0,
                    });
                },
            });
        }
#endif
        rg->EndFrame();

        TL_MAYBE_UNUSED auto result = swapchain->Present();
        if (RHI::IsError(result))
        {
            result = swapchain->Configure({
                .size        = windowSize,
                .imageCount  = 1,
                .imageUsage  = RHI::ImageUsage::Color,
                .format      = RHI::Format::RGBA8_UNORM,
                .presentMode = RHI::SwapchainPresentMode::Fifo,
                .alphaMode   = RHI::SwapchainAlphaMode::None,
            });
            TL_ASSERT(RHI::IsSuccess(result));
        }

        if (GraphDumpEnabled)
        {
            rg->Dump();
            GraphDumpEnabled = false;
        }

        glfwGetWindowSize(window, (int*)&windowSize.width, (int*)&windowSize.height);

        glfwPollEvents();
        FrameMark;
    }
    // #endif

    // TL::Arena a;
    // while (!glfwWindowShouldClose(window))
    // {
    //     for (int i = 0 ; i < 100; i++)
    //     {
    //         a.Allocate<float>(100);
    //         auto b = TL::Allocate<int>();
    //         TL::Release(b);
    //         // auto b = malloc(sizeof(int));
    //         // free(b);
    //     }
    //     a.Collect();
    //     glfwPollEvents();
    // }

    device->DestroySwapchain(swapchain);
    device->DestroyRenderGraph(renderGraph);
    device->DestroyPipelineLayout(pipelineLayout);
    device->DestroyGraphicsPipeline(pipeline);
    RHI::DestroyVulkanDevice(device);
    glfwTerminate();

    // size_t memLeakCount, memLeakSize;
    // TL::MemPlumber::memLeakCheck(memLeakCount, memLeakSize);
    // TL_LOG_INFO("Detected {} leaked allocations, with total size {}.", memLeakCount, memLeakSize);
    return 0;
}