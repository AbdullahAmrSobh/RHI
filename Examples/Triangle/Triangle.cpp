// Fix swapchain resizing and moving into other monitor
// Fix validation errors and change API if needed!
// Fix all memory and resource leaks!
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

#include <RHI/RHI.hpp>

#include <TL/Allocator/MemPlumber.hpp>
#include <TL/Defer.hpp>
#include <TL/FileSystem/FileSystem.hpp>
#include <TL/Utils.hpp>

#include <Examples-Base/ApplicationBase.hpp>
#include <RHI-Vulkan/Loader.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <tracy/Tracy.hpp>

#include "Camera.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

#include "stb_image.h"

namespace Shader // TODO: this should be reflected from slang-shaders using some tool
{
    static constexpr uint32_t kMaxPointLights = 32; // Maximum number of point lights.
    static constexpr uint32_t kMaxSpotLights  = 32; // Maximum number of spot lights.

    struct DirectionalLight
    {
        alignas(glm::vec4) glm::vec3 direction; // The direction of the directional light.
        alignas(glm::vec4) glm::vec3 color;     // The color of the directional light.
        float intensity;
    };

    struct PointLight
    {
        alignas(glm::vec4) glm::vec3 position; // The position of the point light.
        float radius;                          // The radius of the point light.
        alignas(glm::vec4) glm::vec3 color;    // The color of the point light.
        float intensity;                       // The intensity of the point light.
    };

    struct SpotLight
    {
        alignas(glm::vec4) glm::vec3 position;  // The position of the spot light.
        alignas(glm::vec4) glm::vec3 direction; // The direction of the spot light.
        float radius;                           // The radius of the spot light.
        float angle;                            // The angle of the spot light.
        alignas(glm::vec4) glm::vec3 color;     // The color of the spot light.
        float intensity;                        // The intensity of the spot light.
    };

    struct SceneGlobalBuffer
    {
        glm::mat4x4 worldToViewMatrix;                 // The world-to-view matrix transforms from world space to view space.
        glm::mat4x4 viewToClipMatrix;                  // The view-to-clip matrix transforms from view space to clip space.
        glm::mat4x4 viewToWorldMatrix;                 // The view-to-world matrix transforms from view space to world space.
        alignas(glm::vec4) glm::vec3 cameraPosition;   // The position of the camera in world space.
        alignas(glm::vec4) glm::vec3 ambientIntensity; // The intensity of the ambient light in the scene.
        DirectionalLight directionalLight;             // The directional light in the scene.
        uint32_t         numPointLights;               // The number of point lights in the scene.
        PointLight       pointLights[kMaxPointLights]; // The point lights in the scene.
        uint32_t         numSpotLights;                // The number of spot lights in the scene.
        SpotLight        spotLights[kMaxSpotLights];   // The spot lights in the scene.
    };

    struct alignas(256) PerDrawBuffer
    {
        glm::mat4x4 modelToWorldMatrix;
        alignas(glm::vec4) glm::vec3 albedo; // The albedo color of the material.
        uint32_t albedoMapIndex;             // The index of the albedo map texture.
        uint32_t normalMapIndex;             // The index of the normal map texture.
        float    metallic;                   // The metallic property of the material.
        float    roughness;                  // The roughness property of the material.
        uint32_t metallicRoughnessMapIndex;  // The index of the metallic-roughness map texture.
    };
} // namespace Shader

class BufferView
{
public:
    RHI::Handle<RHI::Buffer> buffer;
    RHI::BufferSubregion     subregion;
};

struct Mesh
{
    uint32_t       m_elementsCount;
    uint32_t       m_materialIndex;
    BufferView     m_indexBuffer;
    BufferView     m_positionVB;
    BufferView     m_normalsVB;
    BufferView     m_uv0VB;
    RHI::IndexType m_indexType;
};

inline static glm::mat4 convertMatrix(const fastgltf::math::fmat4x4& matrix)
{
    return {
        {matrix.col(0).x(), matrix.col(0).y(), matrix.col(0).z(), matrix.col(0).w()},
        {matrix.col(1).x(), matrix.col(1).y(), matrix.col(1).z(), matrix.col(1).w()},
        {matrix.col(2).x(), matrix.col(2).y(), matrix.col(2).z(), matrix.col(2).w()},
        {matrix.col(3).x(), matrix.col(3).y(), matrix.col(3).z(), matrix.col(3).w()},
    };
}

inline static TL::Ptr<RHI::ShaderModule> LoadShaderModule(RHI::Device* device, const char* path)
{
    auto                 code = TL::ReadBinaryFile(path);
    TL::Vector<uint32_t> spirv(code.size / sizeof(uint32_t), 0);
    memcpy(spirv.data(), code.ptr, code.size);
    TL::Allocator::Release(code, 1);
    return device->CreateShaderModule({
        .name = path,
        .code = spirv,
    });
}

inline static BufferView CreateBufferView(RHI::Device& device, size_t size, TL::Flags<RHI::BufferUsage> usage)
{
    RHI::BufferCreateInfo bufferCI{
        .usageFlags = usage,
        .byteSize   = size,
    };
    auto [buffer, error] = device.CreateBuffer(bufferCI);
    TL_ASSERT(RHI::IsSuccess(error));
    return {
        .buffer    = buffer,
        .subregion = {0, size},
    };
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
    else if (accessor.componentType == fastgltf::ComponentType::UnsignedShort)
    {
        fastgltf::copyFromAccessor<uint16_t>(asset, accessor, deviceBufferPtr);
        out_mesh.m_indexType = RHI::IndexType::uint16;
    }
    else
    {
        TL_LOG_ERROR("Unsupported component type in index accessor");
        return {};
    }
    return attributeVertexBufferView;
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

    RHI::Device*                             m_device;
    RHI::Swapchain*                          m_swapchain;
    RHI::RenderGraph*                        m_renderGraph;
    RHI::Handle<RHI::BindGroup>              m_bindGroup;
    RHI::Handle<RHI::PipelineLayout>         m_pipelineLayout;
    RHI::Handle<RHI::GraphicsPipeline>       m_graphicsPipeline;
    UniformBuffer<Shader::SceneGlobalBuffer> m_sceneGlobalUB;
    UniformBuffer<Shader::PerDrawBuffer>     m_perDrawUB;
    TL::Vector<Mesh>                         m_meshes;
    Camera                                   m_camera;

    RHI::Handle<RHI::Sampler>           m_sampler;
    TL::Vector<RHI::Handle<RHI::Image>> m_textures;

    struct DrawItem
    {
        uint32_t              meshIndex;
        Shader::PerDrawBuffer modelUB;
    };

    TL::Vector<DrawItem> m_sceneDrawList;

    RHI::Handle<RHI::Image> loadImage(const fastgltf::Asset& asset, const fastgltf::Image& image)
    {
        int      width, height, nrChannels;
        stbi_uc* data = nullptr;
        std::visit(
            fastgltf::visitor{
                [&](auto& arg)
                {
                    TL_UNREACHABLE();
                },
                [&](fastgltf::sources::URI& filePath)
                {
                    TL_ASSERT(filePath.fileByteOffset == 0);                                       // We don't support offsets with stbi.
                    TL_ASSERT(filePath.uri.isLocalPath());                                         // We're only capable of loading local files.
                    const TL::String path(filePath.uri.path().begin(), filePath.uri.path().end()); // Thanks C++.
                    data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
                },
                [&](fastgltf::sources::Array& vector)
                {
                    data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(vector.bytes.data()), static_cast<int>(vector.bytes.size()), &width, &height, &nrChannels, 4);
                },
                [&](fastgltf::sources::BufferView& view)
                {
                    auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                    auto& buffer     = asset.buffers[bufferView.bufferIndex];
                    std::visit(
                        fastgltf::visitor{
                            [](auto& arg)
                            {
                                TL_UNREACHABLE();
                            },
                            [&](fastgltf::sources::Array& vector)
                            {
                                data = stbi_load_from_memory(
                                    reinterpret_cast<const stbi_uc*>(vector.bytes.data() + bufferView.byteOffset), static_cast<int>(bufferView.byteLength), &width, &height, &nrChannels, 4);
                            }},
                        buffer.data);
                },
            },
            image.data);

        TL_ASSERT(data);

        RHI::ImageCreateInfo imageCI = {
            .usageFlags = RHI::ImageUsage::ShaderResource | RHI::ImageUsage::CopyDst,
            .type       = RHI::ImageType::Image2D,
            .size       = {
                      .width  = (unsigned)width,
                      .height = (unsigned)height,
            },
            .format = RHI::Format::RGBA8_UNORM,
        };
        auto [texture, error] = RHI::CreateImageWithContent(*m_device, imageCI, TL::Block{.ptr = data, .size = uint32_t(width * height * 4)});
        stbi_image_free(data);
        return texture;
    }

    void LoadScene()
    {
        auto path = GetLaunchSettings().sceneFileLocation;

        if (!std::filesystem::exists(path))
        {
            // TL_LOG_ERROR("Failed to find {}", path);
        }

        static constexpr auto supportedExtensions =
            fastgltf::Extensions::KHR_mesh_quantization |
            fastgltf::Extensions::KHR_texture_transform |
            fastgltf::Extensions::KHR_materials_variants;
        fastgltf::Parser parser(supportedExtensions);

        auto [fileErr, gltfFile] = fastgltf::MappedGltfFile::FromPath(path);
        if (fileErr != fastgltf::Error::None)
        {
            TL::String msg = fastgltf::getErrorMessage(fileErr).data();
            TL_LOG_ERROR("Failed to load glTF: {}", msg);
        }

        constexpr auto gltfOptions =
            fastgltf::Options::DontRequireValidAssetMember |
            fastgltf::Options::AllowDouble |
            fastgltf::Options::LoadExternalBuffers |
            // fastgltf::Options::LoadExternalImages |
            fastgltf::Options::GenerateMeshIndices;
        auto [assetError, asset] = parser.loadGltf(gltfFile, path.parent_path(), gltfOptions);
        if (assetError != fastgltf::Error::None)
        {
            TL::String msg = fastgltf::getErrorMessage(assetError).data();
            TL_LOG_ERROR("Failed to load glTF: {}", msg);
        }

        const int DefaultMaterialValue = asset.materials.size();

        TL::Map<size_t, uint32_t> primitiveIndexToSceneMeshIndex;
        for (int meshIndex = 0; meshIndex < asset.meshes.size(); meshIndex++)
        {
            const auto& mesh = asset.meshes[meshIndex];
            for (int primitiveIndex = 0; primitiveIndex < mesh.primitives.size(); primitiveIndex++)
            {
                const auto& primitive = mesh.primitives[primitiveIndex];
                TL_ASSERT(primitive.indicesAccessor.has_value());
                TL_ASSERT(primitive.type == fastgltf::PrimitiveType::Triangles);
                TL_ASSERT(primitive.findAttribute("POSITION") != primitive.attributes.end());
                TL_ASSERT(primitive.findAttribute("NORMAL") != primitive.attributes.end());
                TL_ASSERT(primitive.findAttribute("TEXCOORD_0") != primitive.attributes.end());
                auto indexBufferAccessor = asset.accessors[primitive.indicesAccessor.value()];
                Mesh sceneMesh{
                    .m_elementsCount = (uint32_t)indexBufferAccessor.count,
                    .m_materialIndex = (uint32_t)primitive.materialIndex.value_or(DefaultMaterialValue),
                    .m_indexBuffer   = LoadIndexAccessor(*m_device, sceneMesh, asset, indexBufferAccessor),
                    .m_positionVB    = LoadAttribute<fastgltf::math::f32vec3>(*m_device, asset, primitive.findAttribute("POSITION")),
                    .m_normalsVB     = LoadAttribute<fastgltf::math::f32vec3>(*m_device, asset, primitive.findAttribute("NORMAL")),
                    .m_uv0VB         = LoadAttribute<fastgltf::math::f32vec2>(*m_device, asset, primitive.findAttribute("TEXCOORD_0")),
                    .m_indexType     = indexBufferAccessor.componentType == fastgltf::ComponentType::UnsignedInt ? RHI::IndexType::uint32 : RHI::IndexType::uint16,
                };
                m_meshes.push_back(sceneMesh);
                primitiveIndexToSceneMeshIndex[TL::HashCombine(meshIndex, primitiveIndex)] = m_meshes.size() - 1;
            }
        }

        auto [width, height] = m_window->GetWindowSize();
        fastgltf::iterateSceneNodes(
            asset, 0, fastgltf::math::fmat4x4(), [&](fastgltf::Node& node, fastgltf::math::fmat4x4 _matrix)
            {
                auto matrix = convertMatrix(_matrix);
                if (node.meshIndex)
                {
                    const auto& mesh = asset.meshes[node.meshIndex.value()];
                    for (uint32_t primitiveIndex = 0; primitiveIndex < mesh.primitives.size(); primitiveIndex++)
                    {
                        const auto& primitive = mesh.primitives[primitiveIndex];

                        auto sceneMeshIndex = primitiveIndexToSceneMeshIndex[TL::HashCombine(node.meshIndex.value(), primitiveIndex)];

                        const auto& material = asset.materials[primitive.materialIndex.value_or(0)];

                        // get primitive index in m_meshes
                        m_sceneDrawList.push_back({
                            .meshIndex = sceneMeshIndex,
                            .modelUB   = {
                                  .modelToWorldMatrix        = matrix,
                                  .albedo                    = {material.pbrData.baseColorFactor.x(), material.pbrData.baseColorFactor.y(), material.pbrData.baseColorFactor.z()},
                                  .albedoMapIndex            = material.pbrData.baseColorTexture ? (uint32_t)material.pbrData.baseColorTexture->textureIndex : uint32_t(-1),
                                  .normalMapIndex            = material.normalTexture ? (uint32_t)material.normalTexture->textureIndex : uint32_t(-1),
                                  .metallic                  = material.pbrData.metallicFactor,
                                  .roughness                 = material.pbrData.roughnessFactor,
                                  .metallicRoughnessMapIndex = material.pbrData.metallicRoughnessTexture ? (uint32_t)material.pbrData.metallicRoughnessTexture->textureIndex : uint32_t(-1)},
                        });
                    }
                }

                if (node.cameraIndex)
                {
                    auto      camera = asset.cameras[node.cameraIndex.value()];
                    glm::vec3 scale, translation, skew;
                    glm::quat orientation;
                    glm::vec4 perspective;
                    glm::decompose(matrix, scale, orientation, translation, skew, perspective);
                    translation.y *= -1;
                    orientation = glm::conjugate(orientation);
                    m_camera.SetPosition(translation);
                    m_camera.SetRotation(glm::eulerAngles(orientation));

                    if (auto ortho = std::get_if<fastgltf::Camera::Orthographic>(&camera.camera); ortho)
                    {
                        m_camera.SetOrthographic(
                            -ortho->xmag,
                            ortho->xmag,
                            -ortho->ymag,
                            ortho->ymag,
                            ortho->znear,
                            ortho->zfar);
                    }
                    else if (auto perspective = std::get_if<fastgltf::Camera::Perspective>(&camera.camera); perspective)
                    {
                        m_camera.SetPerspective(
                            glm::degrees(perspective->yfov),
                            perspective->aspectRatio.value_or((float)width / (float)height),
                            perspective->znear,
                            perspective->zfar.value_or(100000.0f));
                    }
                    else
                    {
                        TL_UNREACHABLE();
                    }
                }
            });

        for (const auto& image : asset.images)
        {
            // m_textures.push_back(loadImage(asset, image));
        }

        m_device->UpdateBindGroup(
            m_bindGroup,
            {
                // .images = {
                //     {
                //         .dstBinding = 3,
                //         .images     = m_textures,
                //     },
                // },
                .buffers = {
                    {
                        .dstBinding = 0,
                        .buffers    = m_sceneGlobalUB.view.buffer,
                    },
                    {
                        .dstBinding = 1,
                        .buffers    = m_perDrawUB.view.buffer,
                        .subregions = {
                            {0, m_perDrawUB.stride},
                        },
                    },
                },
                .samplers = {
                    {
                        .dstBinding = 2,
                        .samplers   = m_sampler,
                    },
                },
            });
    }

    void OnInit() override
    {
        ZoneScoped;

        auto [width, height] = m_window->GetWindowSize();
        m_camera.SetPerspective(30.0f, (float)width / (float)height, 0.00001f, 100000.0f);
        m_camera.m_window = m_window.get();

        RHI::ApplicationInfo appInfo{
            .applicationName    = "Example",
            .applicationVersion = {0, 1, 0},
            .engineName         = "Forge",
            .engineVersion      = {0, 1, 0},
        };
        m_device = RHI::CreateVulkanDevice(appInfo);

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

        m_renderGraph = m_device->CreateRenderGraph();

        auto bindGroupLayout = m_device->CreateBindGroupLayout({
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
                {
                    .type   = RHI::BindingType::Sampler,
                    .stages = RHI::ShaderStage::Pixel,
                },
                {
                    .type       = RHI::BindingType::SampledImage,
                    .arrayCount = RHI::BindlessArraySize,
                    .stages     = RHI::ShaderStage::Pixel,
                },
            },
        });

        m_pipelineLayout = m_device->CreatePipelineLayout({
            .name    = "graphics-pipeline-layout",
            .layouts = {bindGroupLayout},
        });

        auto vertexModule  = LoadShaderModule(m_device, "./Shaders/Basic.vertex.spv");
        auto pixelModule   = LoadShaderModule(m_device, "./Shaders/Basic.pixel.spv");
        m_graphicsPipeline = m_device->CreateGraphicsPipeline({
            .name               = "Hello-Triangle",
            .vertexShaderName   = "VSMain",
            .vertexShaderModule = vertexModule.get(),
            .pixelShaderName    = "PSMain",
            .pixelShaderModule  = pixelModule.get(),
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
            .rasterizationState{
                .cullMode  = RHI::PipelineRasterizerStateCullMode::BackFace,
                .frontFace = RHI::PipelineRasterizerStateFrontFace::Clockwise,
            },
            .depthStencilState{
                .depthTestEnable  = true,
                .depthWriteEnable = true,
            },
        });

        m_sceneGlobalUB.Init(*m_device);
        m_perDrawUB.Init(*m_device, 512);

        m_sampler = m_device->CreateSampler({.name = "default_sampler"});

        m_bindGroup = m_device->CreateBindGroup(bindGroupLayout);
        m_device->DestroyBindGroupLayout(bindGroupLayout);

        LoadScene();
    }

    void OnShutdown() override
    {
        ZoneScoped;
        for (auto& mesh : m_meshes)
        {
            m_device->DestroyBuffer(mesh.m_indexBuffer.buffer);
            m_device->DestroyBuffer(mesh.m_positionVB.buffer);
            m_device->DestroyBuffer(mesh.m_normalsVB.buffer);
            m_device->DestroyBuffer(mesh.m_uv0VB.buffer);
        }
        m_device->DestroyBuffer(m_sceneGlobalUB.view.buffer);
        m_device->DestroyBuffer(m_perDrawUB.view.buffer);
        m_device->DestroyRenderGraph(m_renderGraph);
        m_device->DestroyBindGroup(m_bindGroup);
        m_device->DestroyGraphicsPipeline(m_graphicsPipeline);
        m_device->DestroyPipelineLayout(m_pipelineLayout);
        m_device->DestroySwapchain(m_swapchain);
        RHI::DestroyVulkanDevice(m_device);
    }

    void OnUpdate(Timestep ts) override
    {
        ZoneScoped;
        m_camera.Update(ts);

        m_sceneGlobalUB.Get()->worldToViewMatrix = m_camera.GetView();
        m_sceneGlobalUB.Get()->viewToClipMatrix  = m_camera.GetProjection();
        m_sceneGlobalUB.Get()->cameraPosition    = m_camera.GetPosition();
        m_sceneGlobalUB.Get()->directionalLight  = {
            glm::vec3(0.4, 1.0, 0.0), // direction
            glm::vec3(1.0, 1.0, 1.0), // color
            1.0                       // intensity
        };

        for (int i = 0; i < m_sceneDrawList.size(); i++)
        {
            *m_perDrawUB.Get(i) = m_sceneDrawList[i].modelUB;
        }
    }

    void Render() override
    {
        ZoneScoped;
        auto [width, height] = m_window->GetWindowSize();

        static auto colorAttachment = m_renderGraph->ImportSwapchain("main-output", *m_swapchain, RHI::Format::RGBA8_UNORM);
        static auto depthAttachment = m_renderGraph->CreateImage({
            .name       = "DepthImage",
            .usageFlags = RHI::ImageUsage::Depth,
            .type       = RHI::ImageType::Image2D,
            .size       = {width, height, 1},
            .format     = RHI::Format::D32,
        });

        m_renderGraph->BeginFrame({width, height});
        [[maybe_unused]] auto pass = m_renderGraph->AddPass({
            .name          = "main-buffer",
            .queue         = RHI::QueueType::Graphics,
            .setupCallback = [&](RHI::RenderGraph& renderGraph, RHI::Pass& pass)
            {
                m_renderGraph->UseRenderTarget(pass, {.attachment = colorAttachment, .clearValue = {.f32{0.1f, 0.1f, 0.4f, 1.0f}}});
                m_renderGraph->UseRenderTarget(pass, {.attachment = depthAttachment, .clearValue = {.depthStencil = {1.0f, 0}}});
            },
            .compileCallback = [&](RHI::RenderGraph& renderGraph, RHI::Pass& pass)
            {
            },
            .executeCallback = [&](RHI::CommandList& commandList)
            {
                commandList.SetViewport({
                    .offsetX  = 0.0f,
                    .offsetY  = 0.0f,
                    .width    = (float)width,
                    .height   = (float)height,
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f,
                });
                commandList.SetSicssor({
                    .offsetX = 0,
                    .offsetY = 0,
                    .width   = width,
                    .height  = height,
                });

                for (uint32_t index = 0; index < m_sceneDrawList.size(); index++)
                {
                    auto drawItem = m_sceneDrawList[index];

                    commandList.BindGraphicsPipeline(m_graphicsPipeline, RHI::BindGroupBindingInfo{m_bindGroup, {index * 256}});
                    m_meshes[index].Draw(commandList);
                }
            },
        });

        pass->Resize({width, height});

        m_renderGraph->EndFrame();

        m_device->CollectResources();

        FrameMark;
    }

    void OnEvent(Event& event) override
    {
        ZoneScoped;
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
