// Fix swapchain resizing and moving into other monitor
// Fix validation errors and change API if needed!
// Fix all memory and resource leaks!

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

#include <Examples-Base/ApplicationBase.hpp>

#include <RHI/RHI.hpp>
#include <RHI-Vulkan/Loader.hpp>

#include <tracy/Tracy.hpp>

#include <TL/FileSystem/FileSystem.hpp>
#include <TL/Allocator/MemPlumber.hpp>
#include <TL/Defer.hpp>

#include "Camera.hpp"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

glm::mat4 convertMatrix(fastgltf::math::fmat4x4 matrix)
{
    // clang-format off
    return glm::mat4
    {
        { matrix.col(0).x(), matrix.col(0).y(), matrix.col(0).z(), matrix.col(0).w(), },
        { matrix.col(1).x(), matrix.col(1).y(), matrix.col(1).z(), matrix.col(1).w(), },
        { matrix.col(2).x(), matrix.col(2).y(), matrix.col(2).z(), matrix.col(2).w(), },
        { matrix.col(3).x(), matrix.col(3).y(), matrix.col(3).z(), matrix.col(3).w(), },
    };
    // clang-format on
}

namespace Shader
{
    struct SceneGlobalBuffer
    {
        glm::mat4 worldToViewMatrix;
        glm::mat4 viewToClipMatrix;
    };

    struct PerDrawBuffer
    {
        glm::mat4 modelToWorldMatrix;
    };
}; // namespace Shader

class BufferView
{
public:
    RHI::Handle<RHI::Buffer> buffer;
    RHI::BufferSubregion     subregion;
};

class Mesh
{
public:
    uint32_t       m_elementsCount;
    uint32_t       m_materialIndex;
    BufferView     m_indexBuffer;
    BufferView     m_positionVB;
    BufferView     m_normalsVB;
    BufferView     m_uv0VB;
    RHI::IndexType m_indexType;

    void Draw(RHI::CommandList& commandList)
    {
        commandList.BindIndexBuffer({.buffer = m_indexBuffer.buffer, .offset = m_indexBuffer.subregion.offset}, m_indexType);
        commandList.BindVertexBuffers(
            0,
            {
                RHI::BufferBindingInfo{.buffer = m_positionVB.buffer, .offset = m_positionVB.subregion.offset},
                RHI::BufferBindingInfo{.buffer = m_normalsVB.buffer, .offset = m_normalsVB.subregion.offset},
                RHI::BufferBindingInfo{.buffer = m_uv0VB.buffer, .offset = m_uv0VB.subregion.offset},
            });
        commandList.Draw({m_elementsCount, 1, 0, 0, 0});
    }
};

inline static BufferView CreateBufferView(RHI::Device& device, size_t size, TL::Flags<RHI::BufferUsage> usage)
{
    return BufferView{
        .buffer = device
                      .CreateBuffer({
                          .usageFlags = usage,
                          .byteSize   = size,
                      })
                      .GetValue(),
        .subregion = {0, size}};
}

// Helper function for loading vertex attributes
template<typename T>
BufferView LoadAttribute(RHI::Device& device, const fastgltf::Asset& asset, const fastgltf::Attribute* attribute)
{
    auto& accessor         = asset.accessors[attribute->accessorIndex];
    auto  deviceBufferView = CreateBufferView(device, accessor.count * sizeof(T), RHI::BufferUsage::Vertex);
    auto  deviceBufferPtr  = device.MapBuffer(deviceBufferView.buffer);
    fastgltf::copyFromAccessor<T>(asset, accessor, deviceBufferPtr);
    device.UnmapBuffer(deviceBufferView.buffer);
    return deviceBufferView;
}

// Helper function for loading index buffer
BufferView LoadIndexAccessor(RHI::Device& device, Mesh& out_mesh, const fastgltf::Asset& asset, const fastgltf::Accessor& accessor)
{
    auto dataSizeBytes             = accessor.count * fastgltf::getComponentByteSize(accessor.componentType);
    auto attributeVertexBufferView = CreateBufferView(device, dataSizeBytes, RHI::BufferUsage::Index);
    auto deviceBufferPtr           = device.MapBuffer(attributeVertexBufferView.buffer);
    TL_defer
    {
        device.UnmapBuffer(attributeVertexBufferView.buffer);
    };
    if (accessor.componentType == fastgltf::ComponentType::UnsignedInt)
    {
        fastgltf::copyFromAccessor<uint32_t>(asset, accessor, deviceBufferPtr);
        out_mesh.m_indexType = RHI::IndexType::uint32;
    }
    else
    {
        fastgltf::copyFromAccessor<uint16_t>(asset, accessor, deviceBufferPtr);
        out_mesh.m_indexType = RHI::IndexType::uint16;
    }
    return attributeVertexBufferView;
}

inline static fastgltf::Asset LoadAsset(std::filesystem::path path)
{
    if (!std::filesystem::exists(path))
    {
        // TL_LOG_ERROR("Failed to find {}", path);
    }

    // Parse the glTF file and get the constructed asset
    static constexpr auto supportedExtensions = fastgltf::Extensions::KHR_mesh_quantization | fastgltf::Extensions::KHR_texture_transform |
                                                fastgltf::Extensions::KHR_materials_variants;

    fastgltf::Parser parser(supportedExtensions);

    constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble |
                                 fastgltf::Options::LoadExternalBuffers | fastgltf::Options::LoadExternalImages |
                                 fastgltf::Options::GenerateMeshIndices;

    auto gltfFile = fastgltf::MappedGltfFile::FromPath(path);
    if (!bool(gltfFile))
    {
        TL::String errorMsg = fastgltf::getErrorMessage(gltfFile.error()).data();
        TL_LOG_ERROR("Failed to load glTF: {}", errorMsg);
    }

    auto asset = parser.loadGltf(gltfFile.get(), path.parent_path(), gltfOptions);
    if (asset.error() != fastgltf::Error::None)
    {
        TL::String errorMsg = fastgltf::getErrorMessage(asset.error()).data();
        TL_LOG_ERROR("Failed to load glTF: {}", errorMsg);
    }

    return std::move(asset.get());
}

// Main Load function
inline static Mesh Load(RHI::Device& device, const fastgltf::Asset& asset, const fastgltf::Mesh& mesh)
{
    Mesh out_mesh{};
    for (auto it = mesh.primitives.begin(); it != mesh.primitives.end(); ++it)
    {
        auto* positionIt = it->findAttribute("POSITION");

        TL_ASSERT(positionIt != it->attributes.end());
        TL_ASSERT(it->indicesAccessor.has_value());
        TL_ASSERT(it->type == fastgltf::PrimitiveType::Triangles);

        auto indexAccessor       = asset.accessors[it->indicesAccessor.value()];
        out_mesh.m_elementsCount = (uint32_t)indexAccessor.count;
        out_mesh.m_indexBuffer   = LoadIndexAccessor(device, out_mesh, asset, indexAccessor);

        if (auto attribute = it->findAttribute("POSITION"); attribute != it->attributes.end())
            out_mesh.m_positionVB = LoadAttribute<fastgltf::math::f32vec3>(device, asset, attribute);

        if (auto attribute = it->findAttribute("NORMAL"); attribute != it->attributes.end())
            out_mesh.m_normalsVB = LoadAttribute<fastgltf::math::f32vec3>(device, asset, attribute);

        if (auto attribute = it->findAttribute("TEXCOORD_0"); attribute != it->attributes.end())
            out_mesh.m_uv0VB = LoadAttribute<fastgltf::math::f32vec2>(device, asset, attribute);
        else TL_LOG_WARNNING("Missing tex coord attribute");
    }
    return out_mesh;
}

inline static RHI::Handle<RHI::Image> LoadTexture(RHI::Device& device, const fastgltf::Asset& asset, const fastgltf::Image& image)
{
    auto format = [](int nrChannels) -> RHI::Format
    {
        if (nrChannels == 1) return RHI::Format::R8_UNORM;
        if (nrChannels == 2) return RHI::Format::RG8_UNORM;
        if (nrChannels == 4) return RHI::Format::RGBA8_UNORM;
        return RHI::Format::Unknown;
    };

    // clang-format off
    RHI::Handle<RHI::Image> out_image;
    std::visit(
        fastgltf::visitor{
            [](auto& arg) {
                TL_UNREACHABLE_MSG("No-op");
            },
            [&](fastgltf::sources::URI& filePath) {
                TL_ASSERT(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
                TL_ASSERT(filePath.uri.isLocalPath());   // We're only capable of loading local files.
                int width, height, nrChannels;
                const std::string path(filePath.uri.path().begin(), filePath.uri.path().end()); // Thanks C++.
                unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
                out_image = RHI::CreateImageWithContent(device, {
                    .name = nullptr, // TODO: Add name later
                    .usageFlags = RHI::ImageUsage::ShaderResource,
                    .type = RHI::ImageType::Image2D,
                    .size = {(uint32_t)width, (uint32_t)height, 1 },
                    .format = format(nrChannels),
                }, { data, (size_t)(width * height * nrChannels) }).GetValue();
                stbi_image_free(data);
            },
            [&](fastgltf::sources::Array& vector) {
                int width, height, nrChannels;
                unsigned char* data = stbi_load_from_memory(
                    reinterpret_cast<const stbi_uc*>(vector.bytes.data()),
                    static_cast<int>(vector.bytes.size()),
                    &width, &height, &nrChannels, 4
                );
                out_image = RHI::CreateImageWithContent(device, {
                    .name = nullptr, // TODO: Add name later
                    .usageFlags = RHI::ImageUsage::ShaderResource,
                    .type = RHI::ImageType::Image2D,
                    .size = {(uint32_t)width, (uint32_t)height, 1 },
                    .format = format(nrChannels),
                }, { data, (size_t)(width * height * nrChannels) }).GetValue();
                stbi_image_free(data);
            },
            [&](fastgltf::sources::BufferView& view) {
                auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                auto& buffer = asset.buffers[bufferView.bufferIndex];

                // We've already loaded every buffer into some GL buffer. However, with GL it's simpler
                // to just copy the buffer data again for the texture. This is just an example.
                std::visit(
                    fastgltf::visitor{
                        [](auto& arg) {
                            TL_UNREACHABLE_MSG("No-op");
                        },
                        [&](fastgltf::sources::Array& vector) {
                            int width, height, nrChannels;
                            unsigned char* data = stbi_load_from_memory(
                                reinterpret_cast<const stbi_uc*>(vector.bytes.data() + bufferView.byteOffset),
                                static_cast<int>(bufferView.byteLength),
                                &width, &height, &nrChannels, 4
                            );
                            out_image = RHI::CreateImageWithContent(device, {
                                .name = nullptr, // TODO: Add name later
                                .usageFlags = RHI::ImageUsage::ShaderResource,
                                .type = RHI::ImageType::Image2D,
                                .size = {(uint32_t)width, (uint32_t)height, 1 },
                                .format = format(nrChannels),
                            }, { data, (size_t)(width * height * nrChannels) }).GetValue();
                            stbi_image_free(data);
                        }
                    },
                    buffer.data
                );
            }
        },
        image.data
    );
    // clang-format on
    return out_image;
}

using namespace Examples;

template<typename T>
struct UniformBuffer
{
    BufferView           view;
    RHI::DeviceMemoryPtr persistantMappedPtr;
    size_t               stride;

    void Init(RHI::Device& device, uint32_t elementsCount = 1)
    {
        stride           = device.GetLimits().minDynamicUniformBufferAlignment;
        size_t sizeBytes = ((sizeof(T) + stride - 1) & -stride) * elementsCount;
        if (elementsCount == 1)
        {
            stride    = ~0ull;
            sizeBytes = sizeof(T);
        }
        view                = CreateBufferView(device, sizeBytes, RHI::BufferUsage::Uniform);
        persistantMappedPtr = device.MapBuffer(view.buffer);
    }

    void Shutdown(RHI::Device& device) { TL_UNREACHABLE(); }

    T* Get(int index = 0) { return (T*)((char*)persistantMappedPtr + (index * stride)); }

    RHI::BindGroupBuffersUpdateInfo Bind(uint32_t binding)
    {
        TL_ASSERT(view.buffer != RHI::NullHandle);
        return RHI::BindGroupBuffersUpdateInfo{
            .dstBinding = binding, .buffers = {view.buffer}, .subregions = RHI::BufferSubregion{view.subregion.offset, stride}};
    }
};

class Playground final : public ApplicationBase
{
public:
    Playground()
        : ApplicationBase("Playground", 1600, 900)
    {
    }

    RHI::Device*    m_device;
    RHI::Swapchain* m_swapchain;

    RHI::RenderGraph*         m_renderGraph;
    RHI::Handle<RHI::Pass>    m_mainPass;
    RHI::Handle<RHI::RGImage> m_colorAttachment;
    RHI::Handle<RHI::RGImage> m_depthAttachment;

    TL::Vector<uint32_t> m_meshIndexList;

    RHI::Handle<RHI::BindGroup>              m_bindGroup;
    RHI::Handle<RHI::PipelineLayout>         m_pipelineLayout;
    RHI::Handle<RHI::GraphicsPipeline>       m_graphicsPipeline;
    UniformBuffer<Shader::SceneGlobalBuffer> m_sceneGlobalUB;
    UniformBuffer<Shader::PerDrawBuffer>     m_perDrawUB;
    TL::Vector<Mesh>                         m_meshes;

    Camera m_camera;

    void InitContextAndSwapchain()
    {
        RHI::ApplicationInfo appInfo{
            .applicationName    = "Example",
            .applicationVersion = {0, 1, 0},
            .engineName         = "Forge",
            .engineVersion      = {0, 1, 0},
        };
        m_device = RHI::CreateVulkanDevice(appInfo);

        auto [width, height] = m_window->GetWindowSize();
        RHI::SwapchainCreateInfo swapchainInfo{
            .name          = "Swapchain",
            .imageSize     = {width, height},
            .imageUsage    = RHI::ImageUsage::Color,
            .imageFormat   = RHI::Format::RGBA8_UNORM,
            .minImageCount = 3,
            .presentMode   = RHI::SwapchainPresentMode::Fifo,
            .win32Window   = {m_window->GetNativeHandle()},
        };

        m_swapchain = m_device->CreateSwapchain(swapchainInfo);
    }

    void ShutdownContextAndSwapchain()
    {
        delete m_swapchain;
        RHI::DestroyVulkanDevice(m_device);
    }

    void InitPipelineAndLayout()
    {
        TL::Vector<uint32_t> spv;
        auto                 spvBlock = TL::ReadBinaryFile("./Shaders/Basic.spv");
        spv.resize(spvBlock.size / 4);
        memcpy(spv.data(), spvBlock.ptr, spvBlock.size);
        auto shaderModule = m_device->CreateShaderModule({.code = spv});
        TL::Allocator::Release(spvBlock, alignof(char));

        RHI::BindGroupLayoutCreateInfo bindGroupLayoutCI{
            .name = "BGL-ViewUB",
            .bindings{
                {
                    .type   = RHI::BindingType::UniformBuffer,
                    .stages = RHI::ShaderStage::Pixel | RHI::ShaderStage::Vertex,
                },
                {
                    .type   = RHI::BindingType::DynamicUniformBuffer,
                    .stages = RHI::ShaderStage::Pixel | RHI::ShaderStage::Vertex,
                },
            },
        };
        auto bindGroupLayout = m_device->CreateBindGroupLayout(bindGroupLayoutCI);

        RHI::PipelineLayoutCreateInfo layoutCI{.name = "graphics-pipeline-layout", .layouts = {bindGroupLayout}};
        m_pipelineLayout = m_device->CreatePipelineLayout(layoutCI);

        RHI::GraphicsPipelineCreateInfo pipelineCI{
            .name               = "Hello-Triangle",
            .vertexShaderName   = "VSMain",
            .vertexShaderModule = shaderModule.get(),
            .pixelShaderName    = "PSMain",
            .pixelShaderModule  = shaderModule.get(),
            .layout             = m_pipelineLayout,
            .vertexBufferBindings{
                {
                    .stride     = sizeof(glm::vec3),
                    .attributes = {{.format = RHI::Format::RGB32_FLOAT}},
                },
                {
                    .stride     = sizeof(glm::vec3),
                    .attributes = {{.format = RHI::Format::RGBA32_FLOAT}},
                },
                {
                    .stride     = sizeof(glm::vec2),
                    .attributes = {{.format = RHI::Format::RG32_FLOAT}},
                },
            },
            .renderTargetLayout{
                .colorAttachmentsFormats = RHI::Format::RGBA8_UNORM,
                .depthAttachmentFormat   = RHI::Format::D32,
            },
            .colorBlendState{.blendStates = {{.blendEnable = true}}},
            .rasterizationState{.cullMode = RHI::PipelineRasterizerStateCullMode::None},
            .depthStencilState{
                .depthTestEnable  = true,
                .depthWriteEnable = true,
            },
        };
        m_graphicsPipeline = m_device->CreateGraphicsPipeline(pipelineCI);

        // init and update bind groups

        RHI::BindGroupBuffersUpdateInfo sceneGlobalBinding{.dstBinding = 0, .buffers = m_sceneGlobalUB.view.buffer};
        RHI::BindGroupBuffersUpdateInfo perDrawBinding{
            .dstBinding = 1, .buffers = m_perDrawUB.view.buffer, .subregions = {{0, m_perDrawUB.stride}}};

        const RHI::BindGroupBuffersUpdateInfo buffersUpdateInfo[2] = {
            sceneGlobalBinding,
            perDrawBinding,
        };

        m_bindGroup = m_device->CreateBindGroup(bindGroupLayout);
        RHI::BindGroupUpdateInfo bindGroupUpdateInfo{
            .buffers = buffersUpdateInfo,
        };
        m_device->UpdateBindGroup(m_bindGroup, bindGroupUpdateInfo);
        m_device->DestroyBindGroupLayout(bindGroupLayout);
    }

    void ShutdownPipelineAndLayout()
    {
        m_device->DestroyBindGroup(m_bindGroup);
        m_device->DestroyGraphicsPipeline(m_graphicsPipeline);
        m_device->DestroyPipelineLayout(m_pipelineLayout);
    }

    void InitRenderGraph()
    {
        auto [width, height] = m_window->GetWindowSize();
        m_renderGraph        = m_device->CreateRenderGraph().release();

        RHI::PassCreateInfo passCI{
            .name  = "main-pass",
            .flags = RHI::PassFlags::Graphics,
        };
        m_mainPass = m_renderGraph->CreatePass(passCI);

        m_colorAttachment = m_renderGraph->ImportSwapchain("main-output", *m_swapchain);
        m_depthAttachment = m_renderGraph->CreateImage({
            .name       = "DepthImage",
            .usageFlags = RHI::ImageUsage::Depth,
            .type       = RHI::ImageType::Image2D,
            .size       = {width, height, 1},
            .format     = RHI::Format::D32,
        });

        m_renderGraph->PassUseImage(
            m_mainPass, m_colorAttachment, RHI::ImageUsage::Color, RHI::PipelineStage::ColorAttachmentOutput, RHI::Access::None);

        m_renderGraph->PassUseImage(
            m_mainPass,
            m_depthAttachment,
            RHI::ImageUsage::Depth,
            RHI::PipelineStage::LateFragmentTests | RHI::PipelineStage::EarlyFragmentTests,
            RHI::Access::ReadWrite);

        m_renderGraph->PassResize(m_mainPass, {width, height});
    }

    void ShutdownRenderGraph()
    {
        if (m_renderGraph) delete m_renderGraph;
    }

    TL::Vector<glm::mat4> matrixList{};

    TL::Vector<RHI::Handle<RHI::Image>> m_sceneTextures{};

    void OnInit() override
    {
        InitContextAndSwapchain();
        InitRenderGraph();

        m_sceneGlobalUB.Init(*m_device);
        m_perDrawUB.Init(*m_device, 512);

        auto asset = LoadAsset(GetLaunchSettings().sceneFileLocation);
        for (auto meshAsset : asset.meshes)
        {
            m_meshes.push_back(Load(*m_device, asset, meshAsset));
        }

        uint32_t i = 0;
        for (int i = 0; i < 512; i++)
        {
            m_perDrawUB.Get(i++)->modelToWorldMatrix = glm::identity<glm::mat4>();
        }

        fastgltf::iterateSceneNodes(
            asset,
            0,
            fastgltf::math::fmat4x4(),
            [&](fastgltf::Node& node, fastgltf::math::fmat4x4 _matrix)
        {
            auto matrix = convertMatrix(_matrix);
            if (node.cameraIndex.has_value())
            {
            }
            else if (node.meshIndex.has_value())
            {
                matrixList.push_back(matrix);
                m_meshIndexList.push_back((uint32_t)node.meshIndex.value());
            }
        });

        for (int i = 0; i < matrixList.size(); i++)
        {
            // m_meshIndexList[i];
            m_perDrawUB.Get(i)->modelToWorldMatrix = matrixList[i];
        }

        for (const auto& image : asset.images)
        {
            m_sceneTextures.push_back(LoadTexture(*m_device, asset, image));
        }

        for (int i = 0; i < matrixList.size(); i++)
        {
            TL_ASSERT(m_perDrawUB.Get(i)->modelToWorldMatrix == matrixList[i]);
        }

        InitPipelineAndLayout();

        auto [width, height] = m_window->GetWindowSize();
        m_camera.SetPerspective(30.0f, (float)width / (float)height, 0.00001f, 100000.0f);
        m_camera.m_window = m_window.get();
    }

    void OnShutdown() override
    {
        ShutdownRenderGraph();
        ShutdownPipelineAndLayout();
        ShutdownContextAndSwapchain();
    }

    void OnUpdate(Timestep ts) override
    {
        m_camera.Update(ts);

        m_sceneGlobalUB.Get()->worldToViewMatrix = m_camera.GetView();
        m_sceneGlobalUB.Get()->viewToClipMatrix  = m_camera.GetProjection();

        for (int i = 0; i < matrixList.size(); i++)
        {
            // m_meshIndexList[i];
            // m_perDrawUB.Get(i)->modelToWorldMatrix = matrixList[i] * m_camera.GetView() * m_camera.GetProjection();
            m_perDrawUB.Get(i)->modelToWorldMatrix = m_camera.GetProjection() * m_camera.GetView() * matrixList[i];
        }

        // for (int i = 0; i < matrixList.size(); i++)
        // {
        //     TL_ASSERT(m_perDrawUB.Get(i)->modelToWorldMatrix == matrixList[i]);
        // }
    }

    void Render() override
    {
        static RHI::ClearValue clearValue          = {.f32 = {0.3f, 0.5f, 0.7f, 1.0f}};
        static uint64_t        previousSubmitValue = 0;

        m_device->WaitTimelineValue(previousSubmitValue);

        auto [width, height] = m_window->GetWindowSize();
        m_renderGraph->PassResize(m_mainPass, {width, height});

        auto commandList = m_device->CreateCommandList({.queueType = RHI::QueueType::Graphics});

        commandList->Begin();
        commandList->BeginRenderPass({
            .renderGraph = m_renderGraph,
            .pass        = m_mainPass,
            .renderArea  = {0, 0, width, height},
            .colorAttachments =
                {
                    RHI::ColorAttachmentInfo{
                        .attachment = m_renderGraph->GetImage(m_colorAttachment),
                        .clearValue = {clearValue},
                    },
                },
            .depthStenciAttachments =
                RHI::DepthAttachmentInfo{
                    .attachment = m_renderGraph->GetImage(m_depthAttachment),
                },
        });
        commandList->SetViewport({
            .offsetX  = 0.0f,
            .offsetY  = 0.0f,
            .width    = (float)width,
            .height   = (float)height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        });
        commandList->SetSicssor({
            .offsetX = 0,
            .offsetY = 0,
            .width   = width,
            .height  = height,
        });

        for (auto index : m_meshIndexList)
        {
            commandList->BindGraphicsPipeline(
                m_graphicsPipeline, RHI::BindGroupBindingInfo{.bindGroup = m_bindGroup, .dynamicOffsets = {index * 256}});
            m_meshes[index].Draw(*commandList);
        }

        commandList->EndRenderPass();
        commandList->End();

        previousSubmitValue = m_device->QueueSubmit({
            .waitTimelineValue = previousSubmitValue,
            .waitPipelineStage = RHI::PipelineStage::TopOfPipe,
            .commandLists      = commandList,
            .swapchainToWait   = m_swapchain,
            .swapchainToSignal = m_swapchain,
        });

        auto presentResult = m_swapchain->Present();
        TL_ASSERT(presentResult == RHI::ResultCode::Success);

        m_device->CollectResources();
    }

    void OnEvent(Event& event) override
    {
        switch (event.GetEventType())
        {
        case EventType::WindowResize:
            {
                auto& e   = (WindowResizeEvent&)event;
                auto  res = m_swapchain->Recreate({e.GetSize().width, e.GetSize().height});
                TL_ASSERT(res == RHI::ResultCode::Success);
            }
            break;
        default:
            {
                m_camera.ProcessEvent(event);
            }
            break;
        }
    }
};

#include <Examples-Base/Entry.hpp>

int main(int argc, const char* argv[])
{
    using namespace Examples;
    TL::Span args{argv, (size_t)argc};
    TL::MemPlumber::start();
    auto   result = Entry<Playground>(args);
    size_t memLeakCount, memLeakSize;
    TL::MemPlumber::memLeakCheck(memLeakCount, memLeakSize);
    return result;
}
