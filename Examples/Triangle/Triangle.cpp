#include "Camera.hpp"

#include <Assets/Importer.hpp>
#include <Assets/Mesh.hpp>

#include <Examples-Base/ApplicationBase.hpp>

#include <RPI/Renderer.hpp>
#include <RPI/View.hpp>

#include <tracy/Tracy.hpp>

#include <TL/FileSystem/FileSystem.hpp>
#include <TL/Allocator/MemPlumber.hpp>

#include <ShaderInterface/Core.slang>

// FIXME: need to figure out my cooridnate system
// Left Hand. Y-Up


// auto sphereRadius = 10;
// auto distance = 10;

// for (int xi = 0; xi < 10; xi++)
// {
//     for (int xj = 0; xj < 10; xj++)
//     {
//         auto position = glm::vec3{ distance * i, 0, distance * j,  };
//         auto color    = glm::vec4{ (float)xi/10.0f, (float)xj/10.0f, (float)(xi + xj)/10.0f, 1.0f };
//     }
// }

// cameraPosition = glm::vec3{ 0.0f, -10.0, distance * 10.0 / 2.0 };
// cameraDirection = glm::vec3{ 0, 1, 0 };




using namespace Examples;

// Will only import if the package was modified
inline static Assets::Package LoadPackage(const char* importFile, const char* exportDir, const char* packageName)
{
    Assets::ImportInfo importInfo{
        .filePath = importFile,
        .outputPath = exportDir,
        .packageName = packageName,
    };

    if (std::filesystem::exists(exportDir) && std::filesystem::is_directory(exportDir))
    {
        auto outputPath = TL::String(exportDir) + "/" + packageName;
        auto package = TL::BinaryArchive::Load<Assets::Package>(outputPath.c_str());
        auto lastModifyTime = std::filesystem::last_write_time(importInfo.filePath).time_since_epoch().count();
        if (package.GetID() == lastModifyTime)
        {
            return package;
        }
    }
    std::filesystem::create_directory(exportDir);
    std::filesystem::current_path(exportDir);
    auto package = Assets::Import(importInfo);
    TL::BinaryArchive::Save(package, packageName);
    return package;
}

class RenderImpl final : public RPI::Renderer
{
public:
    RHI::Handle<RHI::Buffer> m_uniformBuffer;
    RHI::Handle<RHI::Buffer> m_uniformBuffer2;

    RHI::Handle<RHI::BindGroup> m_bindGroup;

    RHI::Handle<RHI::PipelineLayout> m_pipelineLayout;
    RHI::Handle<RHI::GraphicsPipeline> m_graphicsPipeline;
    RHI::Handle<RHI::Pass> m_pass;

    RPI::ResultCode OnInit() override
    {
        auto renderGraph = m_renderGraph.get();
        auto context = m_context.get();

        RHI::PassCreateInfo createInfo{
            .name = "Main",
            .flags = RHI::PassFlags::Graphics,
        };
        m_pass = renderGraph->CreatePass(createInfo);

        RHI::ImageViewInfo viewInfo;
        viewInfo.type = RHI::ImageViewType::View2D;
        viewInfo.subresources.imageAspects = RHI::ImageAspect::Color;
        viewInfo.subresources.mipBase = 0;
        viewInfo.subresources.mipLevelCount = 1;
        viewInfo.subresources.arrayBase = 0;
        viewInfo.subresources.arrayCount = 1;
        viewInfo.swizzle.r = RHI::ComponentSwizzle::Identity;
        viewInfo.swizzle.g = RHI::ComponentSwizzle::Identity;
        viewInfo.swizzle.b = RHI::ComponentSwizzle::Identity;
        viewInfo.swizzle.a = RHI::ComponentSwizzle::Identity;
        renderGraph->PassUseImage(m_pass, m_outputAttachment, viewInfo, RHI::ImageUsage::Color, RHI::ShaderStage::None, RHI::Access::None);
        renderGraph->PassResize(m_pass, { 1600, 1200 });
        context->CompileRenderGraph(*renderGraph);

        TL::Vector<uint32_t> spv;
        auto spvBlock = TL::ReadBinaryFile("./Shaders/Basic.spv");
        spv.resize(spvBlock.size / 4);
        memcpy(spv.data(), spvBlock.ptr, spvBlock.size);
        auto shaderModule = context->CreateShaderModule(spv);
        TL::Allocator::Release(spvBlock, alignof(char));

        RHI::BufferCreateInfo uniformBufferCI{
            .name = "Uniform-Buffer",
            .heapType = RHI::MemoryType::GPUShared,
            .usageFlags = RHI::BufferUsage::Uniform,
            .byteSize = TL::AlignUp(sizeof(SI::ViewCB), 256ull),
        };
        m_uniformBuffer = context->CreateBuffer(uniformBufferCI).GetValue();

        uniformBufferCI.name = "PerModelTransform";
        uniformBufferCI.byteSize = TL::AlignUp(256, 256) * 12;
        m_uniformBuffer2 = context->CreateBuffer(uniformBufferCI).GetValue();

        // clang-format off
        RHI::BindGroupLayoutCreateInfo bindGroupLayoutCI{
            .name = "BGL-ViewUB",
            .bindings =
            {
                {
                    .type = RHI::BindingType::UniformBuffer,
                    .access = RHI::Access::Read,
                    .arrayCount = 1,
                    .stages = RHI::ShaderStage::Pixel | RHI::ShaderStage::Vertex
                },
                {
                    .type = RHI::BindingType::DynamicUniformBuffer,
                    .access = RHI::Access::Read,
                    .arrayCount = 1,
                    .stages = RHI::ShaderStage::Pixel | RHI::ShaderStage::Vertex
                },
            }
        };
        auto bindGroupLayout = context->CreateBindGroupLayout(bindGroupLayoutCI);

        m_bindGroup = context->CreateBindGroup(bindGroupLayout);

        RHI::BindGroupUpdateInfo bindGroupUpdateInfo{
            .buffers =
            {
                {
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .buffer = m_uniformBuffer,
                    .subregions = {}
                },
                {
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .buffer = m_uniformBuffer2,
                    .subregions = { RHI::BufferSubregion{0, 256} }
                },
            }
        };
        context->UpdateBindGroup(m_bindGroup, bindGroupUpdateInfo);
        // clang-format on

        m_pipelineLayout = context->CreatePipelineLayout({ .name = nullptr, .layouts = { bindGroupLayout } });

        context->DestroyBindGroupLayout(bindGroupLayout);

        RHI::GraphicsPipelineCreateInfo pipelineCI{};
        pipelineCI.name = "Basic";
        pipelineCI.vertexShaderName = "VSMain";
        pipelineCI.vertexShaderModule = shaderModule.get();
        pipelineCI.pixelShaderName = "PSMain";
        pipelineCI.pixelShaderModule = shaderModule.get();
        pipelineCI.layout = m_pipelineLayout;
        pipelineCI.inputAssemblerState = {
            .bindings{
                      {
                    .binding = 0,
                    .stride = sizeof(glm::vec3),
                    .stepRate = RHI::PipelineVertexInputRate::PerVertex,
                } },
            .attributes{
                      {
                    .location = 0,
                    .binding = 0,
                    .format = RHI::Format::RGBA32_FLOAT,
                    .offset = 0,
                },
                      },
        };
        pipelineCI.renderTargetLayout.colorAttachmentsFormats[0] = RHI::Format::RGBA8_UNORM;
        pipelineCI.colorBlendState.blendStates[0].blendEnable = true;
        pipelineCI.colorBlendState.blendStates[0].colorBlendOp = RHI::BlendEquation::Add;
        pipelineCI.colorBlendState.blendStates[0].srcColor = RHI::BlendFactor::SrcAlpha;
        pipelineCI.colorBlendState.blendStates[0].dstColor = RHI::BlendFactor::OneMinusSrcAlpha;
        pipelineCI.colorBlendState.blendStates[0].alphaBlendOp = RHI::BlendEquation::Add;
        pipelineCI.colorBlendState.blendStates[0].srcAlpha = RHI::BlendFactor::One;
        pipelineCI.colorBlendState.blendStates[0].dstAlpha = RHI::BlendFactor::Zero;
        pipelineCI.colorBlendState.blendStates[0].writeMask = RHI::ColorWriteMask::All;
        pipelineCI.topologyMode = RHI::PipelineTopologyMode::Triangles;
        pipelineCI.rasterizationState.cullMode = RHI::PipelineRasterizerStateCullMode::None;
        pipelineCI.rasterizationState.fillMode = RHI::PipelineRasterizerStateFillMode::Triangle;
        pipelineCI.rasterizationState.frontFace = RHI::PipelineRasterizerStateFrontFace::CounterClockwise;
        pipelineCI.rasterizationState.lineWidth = 1.0;
        pipelineCI.multisampleState.sampleCount = RHI::SampleCount::Samples1;
        pipelineCI.multisampleState.sampleShading = false;
        pipelineCI.depthStencilState.depthTestEnable = true;
        pipelineCI.depthStencilState.depthWriteEnable = true;
        pipelineCI.depthStencilState.compareOperator = RHI::CompareOperator::Less;
        pipelineCI.depthStencilState.stencilTestEnable = false;
        m_graphicsPipeline = context->CreateGraphicsPipeline(pipelineCI);

        return RPI::ResultCode::Sucess;
    }

    void OnShutdown() override
    {
        m_context->DestroyBuffer(m_uniformBuffer);
        m_context->DestroyBindGroup(m_bindGroup);
        m_context->DestroyPipelineLayout(m_pipelineLayout);
        m_context->DestroyGraphicsPipeline(m_graphicsPipeline);
    }

    void OnRender(TL::Span<RPI::Mesh> meshes) override
    {
        auto& frame = m_frameRingbuffer.Get();

        auto commandList = frame.m_commandPool->Allocate(RHI::QueueType::Graphics, RHI::CommandListLevel::Primary);

        RHI::CommandListBeginInfo beginInfo{};
        beginInfo.renderGraph = m_renderGraph.get();
        beginInfo.pass = m_pass;
        beginInfo.loadStoreOperations = {
            {{}, RHI::LoadOperation::Discard, RHI::StoreOperation::Store}
        };
        commandList->Begin(beginInfo);
        auto size = m_renderGraph->GetPassSize(m_pass);
        RHI::Viewport viewport{};
        viewport.width = (float)size.width;
        viewport.height = (float)size.height;
        viewport.maxDepth = 1.0f;
        RHI::Scissor scissor{};
        scissor.width = size.width;
        scissor.height = size.height;
        commandList->SetViewport(viewport);
        commandList->SetSicssor(scissor);

        uint32_t i = 0;
        for (auto mesh : meshes)
        {
            RHI::DrawInfo drawInfo{
                .pipelineState = m_graphicsPipeline,
                .bindGroups = {
                               { m_bindGroup,
                      { 256 * i++ } } },
                .vertexBuffers = { mesh.m_positionVB },
                .indexBuffer = { mesh.m_indexIB },
                .parameters = { mesh.m_elementsCount, 1, 0, 0 },
            };
            commandList->Draw(drawInfo);
        }

        commandList->End();

        m_renderGraph->Submit(m_pass, commandList);
    }
};

TL::Ptr<RenderImpl> CreateDeferred()
{
    return TL::CreatePtr<RenderImpl>();
}

class Playground final : public ApplicationBase
{
public:
    Playground()
        : ApplicationBase("Hello, Triangle", 1600, 1200)
        , m_renderer(CreateDeferred())
    {
    }

    // RHI::BufferPool m_bufferPool;

    struct MeshTransform
    {
        re_mat4 modelToWorldMatrix;
        re_vec4 color;
        char _padding[256 - sizeof(glm::mat4) - sizeof(glm::vec4)];
    };

    TL::Vector<MeshTransform> m_transforms;
    TL::Vector<RPI::Mesh> m_meshes;

    void OnInit() override
    {
        m_camera.m_window = m_window.get();
        m_camera.SetPerspective(60.0f, 1600.0f / 1200.0f, 0.1f, 10000.0f);
        m_camera.SetRotationSpeed(0.0002f);
        m_camera.SetPosition({ 0, 10, 0 });
        m_camera.SetRotation({ 0, 180, 0 });

        m_renderer->Init(*m_window);

        auto context = m_renderer->m_context.get();

        auto package = LoadPackage(m_launchSettings.sceneFileLocation.string().c_str(), "I:/Deccer-Cubes/", "pack.fgpack");
        std::error_code err;
        std::filesystem::current_path("I:/Deccer/Cubes", err);
        TL_ASSERT(err, "Failed to change WD {}", err.message().c_str());
        for (auto meshPath : package.GetMeshes())
        {
            auto meshAsset = TL::BinaryArchive::Load<Assets::Mesh>(meshPath.c_str());

            auto indexData = meshAsset.GetBuffer(Assets::AttributeNames::Indcies);
            auto positionsData = meshAsset.GetBuffer(Assets::AttributeNames::Positions);
            auto normalsData = meshAsset.GetBuffer(Assets::AttributeNames::Normals);

            // clang-format off
            auto indexIB = context->CreateBuffer({
                    .name = "Index-Buffer",
                    .heapType = RHI::MemoryType::GPUShared,
                    .usageFlags = RHI::BufferUsage::Index,
                    .byteSize = indexData->GetData().size,
                }).GetValue();
            auto positionVB = context->CreateBuffer({
                    .name = "Positions-VB",
                    .heapType = RHI::MemoryType::GPUShared,
                    .usageFlags = RHI::BufferUsage::Vertex,
                    .byteSize = positionsData->GetData().size,
                }).GetValue();
            auto normalVB = context->CreateBuffer({
                    .name = "Normals-VB",
                    .heapType = RHI::MemoryType::GPUShared,
                    .usageFlags = RHI::BufferUsage::Vertex,
                    .byteSize = normalsData->GetData().size,
                }).GetValue();
            {
                auto ptr = context->MapBuffer(indexIB);
                memcpy(ptr, indexData->GetData().ptr, indexData->GetData().size);
                context->UnmapBuffer(indexIB);
            }
            {
                auto ptr = context->MapBuffer(positionVB);
                memcpy(ptr, positionsData->GetData().ptr, positionsData->GetData().size);
                context->UnmapBuffer(positionVB);
            }
            {
                auto ptr = context->MapBuffer(normalVB);
                memcpy(ptr, normalsData->GetData().ptr, normalsData->GetData().size);
                context->UnmapBuffer(normalVB);
            }
            m_meshes.push_back({
                indexData->GetElementsCount(),
                indexIB,
                positionVB,
                normalVB,
            });
            // clang-format on
        }

        // for (auto scenePath : package.GetSceneGraphs())
        // {
        //     auto sceneAsset = TL::BinaryArchive::Load<Assets::SceneGraph>(scenePath.c_str());
        //     sceneAsset.Traverse([&](Assets::SceneNode* node)
        //     {
        //         m_transforms.push_back({ node->GetTransform() });
        //     });

        // }
        for (auto t : GenerateCubeTransformations())
        {
            m_transforms.push_back({
                .modelToWorldMatrix = t, .color = { 1.0, 0.4, 0.5, 1.0 },
                     ._padding = {}
            });
        }
    }

    std::vector<glm::mat4> GenerateCubeTransformations(int rows = 2, int columns = 3, float spacing = 1.0f)
    {
        std::vector<glm::mat4> transformations;

        // Loop through each position in the grid
        for (int row = 0; row < rows; ++row)
        {
            for (int col = 0; col < columns; ++col)
            {
                // Calculate the position for the current cube
                float x = col * (spacing + 1.0f); // 1 unit spacing between cubes
                float y = row * (spacing + 1.0f);

                // Create the transformation matrix for the current cube
                glm::mat4 transform = glm::mat4(1.0f); // Identity matrix
                transform = glm::translate(transform, glm::vec3(x, y, 0.0f));

                // Add the transformation matrix to the vector
                transformations.push_back(transform);
            }
        }

        return transformations;
    }

    void OnShutdown() override
    {
        auto context = m_renderer->m_context.get();
        for (auto mesh : m_meshes)
        {
            context->DestroyBuffer(mesh.m_indexIB);
            context->DestroyBuffer(mesh.m_normalVB);
            context->DestroyBuffer(mesh.m_positionVB);
        }
        m_renderer->Shutdown();
    }

    void OnUpdate(Timestep timestep) override
    {
        m_camera.Update(timestep);
    }

    void Render() override
    {
        SI::ViewCB view{};
        view.worldToClipMatrix = m_camera.GetProjection() * m_camera.GetView();
        view.worldToViewMatrix = m_camera.GetView();
        view.viewToClipMatrix = m_camera.GetProjection();

        auto ptr = m_renderer->m_context->MapBuffer(m_renderer->m_uniformBuffer);
        memcpy(ptr, &view, sizeof(SI::ViewCB));
        m_renderer->m_context->UnmapBuffer(m_renderer->m_uniformBuffer);

        auto ptr2 = m_renderer->m_context->MapBuffer(m_renderer->m_uniformBuffer2);
        memcpy(ptr2, m_transforms.data(), m_transforms.size() * sizeof(MeshTransform));
        m_renderer->m_context->UnmapBuffer(m_renderer->m_uniformBuffer2);

        m_renderer->Render(m_meshes);
    }

    void OnEvent(Event& e) override
    {
        m_camera.ProcessEvent(e);
    }

    Camera m_camera;

    TL::Ptr<RenderImpl> m_renderer;
};

#include <Examples-Base/Entry.hpp>

int main(int argc, const char* argv[])
{
    using namespace Examples;
    TL::Span args{ argv, (size_t)argc };
    TL::MemPlumber::start();
    auto result = Entry<Playground>(args);
    size_t memLeakCount, memLeakSize;
    TL::MemPlumber::memLeakCheck(memLeakCount, memLeakSize);
    return result;
}
