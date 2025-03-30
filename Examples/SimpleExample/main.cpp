#include <RHI/RHI.hpp>

#include <TL/Allocator/MemPlumber.hpp>
#include <TL/Defer.hpp>
#include <TL/FileSystem/FileSystem.hpp>
#include <TL/Log.hpp>
#include <TL/Utils.hpp>

#include <RHI-WebGPU/Loader.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <iostream>
#include <tracy/Tracy.hpp>

#include "CommandList.hpp"
#include "Device.hpp"

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
    auto device = (RHI::WebGPU::IDevice*)RHI::CreateWebGPUDevice();

    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    RHI::ImageSize2D windowSize = {800, 480};
    window                      = glfwCreateWindow(windowSize.width, windowSize.height, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    RHI::SwapchainCreateInfo swapchainCI{
        .name          = "Swapchain",
        .imageSize     = windowSize,
        .imageUsage    = {RHI::ImageUsage::Color},
        .imageFormat   = RHI::Format::RGBA8_UNORM,
        .minImageCount = 2,
        .alphaMode     = RHI::SwapchainAlphaMode::PostMultiplied,
        .presentMode   = RHI::SwapchainPresentMode::Fifo,
        .win32Window   = {.hwnd = glfwGetWin32Window(window)},
    };
    auto swapchain = device->CreateSwapchain(swapchainCI);

    auto renderGraph = device->CreateRenderGraph({});

    auto colorAttachment = renderGraph->ImportSwapchain("Swapchain-Color-Attachment", *swapchain, RHI::Format::RGBA8_UNORM);
    auto clearValue      = RHI::ClearValue{
             .f32{0.0f, 1.0f, 1.0f, 1.0f}
    };

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

    while (!glfwWindowShouldClose(window))
    {
        ZoneScopedN("Draw");
        renderGraph->BeginFrame(windowSize);
        auto pass = renderGraph->AddPass({
            .name          = "main-pass",
            .queue         = RHI::QueueType::Graphics,
            .setupCallback = [&](RHI::RenderGraph& renderGraph, RHI::Pass& pass)
            {
                renderGraph.UseColorAttachment(pass, {.view = colorAttachment, .clearValue = {clearValue}});
            },
            .compileCallback = [&](RHI::RenderGraph& renderGraph, RHI::Pass& pass)
            {
            },
            .executeCallback = [&](RHI::CommandList& commandList)
            {
                commandList.SetViewport({
                    .offsetX  = 0.0f,
                    .offsetY  = 0.0f,
                    .width    = (float)windowSize.width,
                    .height   = (float)windowSize.height,
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f,
                });
                commandList.SetScissor({
                    .offsetX = 0,
                    .offsetY = 0,
                    .width   = windowSize.width,
                    .height  = windowSize.height,
                });
            },
        });
        pass->Resize(windowSize);
        renderGraph->EndFrame();
        auto result = swapchain->Present();
        device->CollectResources();
        glfwPollEvents();
        FrameMark;
    }

    device->DestroySwapchain(swapchain);
    RHI::DestroyWebGPUDevice(device);
    glfwTerminate();
    return 0;
}