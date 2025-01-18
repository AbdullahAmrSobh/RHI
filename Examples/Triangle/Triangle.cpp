// Fix swapchain resizing and moving into other monitor
// Fix validation errors and change API if needed!
// Fix all memory and resource leaks!

#include <RHI/RHI.hpp>

#include <TL/Allocator/MemPlumber.hpp>
#include <TL/Defer.hpp>
#include <TL/FileSystem/FileSystem.hpp>
#include <TL/Log.hpp>
#include <TL/Utils.hpp>

#include <RHI-Vulkan/Loader.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <tracy/Tracy.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

#include "Camera.hpp"
#include "Examples-Base/ApplicationBase.hpp"
#include "dds_image/dds.hpp"
#include "stb_image.h"

using namespace Examples;

template<typename T>
using Handle = RHI::Handle<T>;

namespace Shader // TODO: this should be reflected from slang-shaders using some tool
{
    static constexpr uint32_t kMaxDirLights   = 32; // Maximum number of directional lights.
    static constexpr uint32_t kMaxPointLights = 32; // Maximum number of point lights.
    static constexpr uint32_t kMaxSpotLights  = 32; // Maximum number of spot lights.

    struct alignas(16) DirectionalLight
    {
        alignas(16) glm::vec3 direction; // 12 bytes
        float intensity;                 // 4 bytes
        alignas(16) glm::vec3 color;     // 12 bytes, align to 16 bytes
    };

    struct alignas(16) PointLight
    {
        alignas(16) glm::vec3 position; // 12 bytes
        float radius;                   // 4 bytes
        alignas(16) glm::vec3 color;    // 12 bytes, align to 16 bytes
        float intensity;                // 4 bytes
    };

    struct alignas(16) SpotLight
    {
        alignas(16) glm::vec3 position;  // 12 bytes
        float radius;                    // 4 bytes
        alignas(16) glm::vec3 direction; // 12 bytes, align to 16 bytes
        float angle;                     // 4 bytes
        alignas(16) glm::vec3 color;     // 12 bytes, align to 16 bytes
        float intensity;                 // 4 bytes
    };

    struct alignas(16) Scene
    {
        alignas(16) glm::mat4x4 worldToViewMatrix; // 64 bytes
        alignas(16) glm::mat4x4 viewToClipMatrix;  // 64 bytes
        // alignas(16) glm::mat4x4 viewToWorldMatrix;        // 64 bytes
        alignas(16) glm::vec3 cameraPosition;             // 12 bytes (align to 16 bytes)
        alignas(16) glm::vec3 ambientIntensity;           // 12 bytes (align to 16 bytes)
        uint32_t         numDirectionalLights;            // 4 bytes
        uint32_t         numPointLights;                  // 4 bytes
        uint32_t         numSpotLights;                   // 4 bytes
        DirectionalLight directionalLight[kMaxDirLights]; // Aligned array of DirectionalLight
        PointLight       pointLights[kMaxPointLights];    // Aligned array of PointLight
        SpotLight        spotLights[kMaxSpotLights];      // Aligned array of SpotLight
    };

    struct alignas(16) Material
    {
        alignas(16) glm::vec3 albedo;       // 12 bytes
        uint32_t albedoMapIndex;            // 4 bytes (align to 16 bytes)
        uint32_t normalMapIndex;            // 4 bytes
        float    metallic;                  // 4 bytes
        float    roughness;                 // 4 bytes
        uint32_t metallicRoughnessMapIndex; // 4 bytes (align to 16 bytes)
    };

    struct alignas(16) PerDraw
    {
        alignas(16) glm::mat4x4 modelToWorldMatrix; // 64 bytes
        Material material;                          // Material struct, aligned to 16 bytes
    };

    inline constexpr auto kUniformMinOffsetAlignment = 64;
    inline constexpr auto kAlignedSceneSize          = (sizeof(Scene) + kUniformMinOffsetAlignment - 1) & ~(kUniformMinOffsetAlignment - 1);
    inline constexpr auto kAlignedPerDrawSize        = (sizeof(PerDraw) + kUniformMinOffsetAlignment - 1) & ~(kUniformMinOffsetAlignment - 1);

} // namespace Shader

struct SceneDrawable
{
    uint32_t  elementsOffset;
    uint32_t  elementsCount;
    uint32_t  materialIndex;
    glm::mat4 modelToWorldMatrix;
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

template<typename T>
inline static glm::vec2 convertVec3(const fastgltf::math::vec<T, 2>& vec)
{
    return {vec.x(), vec.y()};
}

template<typename T>
inline static glm::vec3 convertVec3(const fastgltf::math::vec<T, 3>& vec)
{
    return {vec.x(), vec.y(), vec.z()};
}

template<typename T>
inline static glm::vec4 convertVec4(const fastgltf::math::vec<T, 4>& vec)
{
    return {vec.x(), vec.y(), vec.z(), vec.w()};
}

inline static glm::quat convertQuat(const fastgltf::math::fquat& quat)
{
    return {quat.x(), quat.y(), quat.z(), quat.w()};
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

class Playground final : public ApplicationBase
{
public:
    Playground()
        : ApplicationBase("Playground", 1600, 900)
    {
    }

    // RHI handles for device, swapchain, and render graph
    RHI::Device*           m_device;
    RHI::Swapchain*        m_swapchain;
    RHI::RenderGraph*      m_renderGraph;
    RHI::RenderGraphImage* m_colorAttachment;
    RHI::RenderGraphImage* m_depthAttachment;

    // Camera and scene data
    Shader::Scene                                   m_scene;
    TL::Vector<Shader::PerDraw>                     m_perDraws;
    TL::Vector<RHI::DrawIndexedIndirectCommandArgs> m_drawIndirectArgs;
    uint32_t                                        m_activeCameraIndex = 0;
    TL::Vector<Camera>                              m_cameras           = {Camera()};

    // Default sampler
    Handle<RHI::Sampler>           m_sampler;
    // Bindless textures array
    TL::Vector<Handle<RHI::Image>> m_images;

    // Buffer pools for geometry and uniforms
    Handle<RHI::Buffer> m_geometryBufferPool;
    Handle<RHI::Buffer> m_uniformBufferPool;

    // Handle to the bind group layout for the scene
    Handle<RHI::BindGroupLayout>  m_sceneBindGroupLayout;
    // Handle to the bind group layout for bindless textures
    Handle<RHI::BindGroupLayout>  m_bindlessTexturesBindGroupLayout;
    // Handle to the pipeline layout for the graphics pipeline
    Handle<RHI::PipelineLayout>   m_graphicsPipelineLayout;
    // Handle to the graphics pipeline
    Handle<RHI::GraphicsPipeline> m_graphicsPipeline;
    // Handle to the bind group for the scene
    Handle<RHI::BindGroup>        m_sceneBindGroup;
    // Handle to the bind group for bindless textures
    Handle<RHI::BindGroup>        m_bindlessTexturesBindGroup;

    size_t m_totalIndexCount;
    size_t m_totalVertexCount;

    Camera& GetActiveCamera()
    {
        return m_cameras[m_activeCameraIndex];
    }

    void LoadScene();
    void OnInit() override;
    void OnShutdown() override;
    void OnUpdate(Timestep ts) override;
    void Render() override;
    void OnEvent(Event& event) override;
};

void Playground::LoadScene()
{
    // Load GLTF asset
    auto path = GetLaunchSettings().sceneFileLocation;

    if (!std::filesystem::exists(path))
    {
        TL::String pathStr{path.string().c_str()};
        TL_LOG_ERROR("Failed to find {}", pathStr);
        return;
    }

    // Supported GLTF extensions
    static constexpr auto supportedExtensions =
        fastgltf::Extensions::KHR_mesh_quantization |
        fastgltf::Extensions::KHR_texture_transform |
        fastgltf::Extensions::KHR_materials_variants;
    fastgltf::Parser parser(supportedExtensions);

    // Load GLTF file
    auto [fileErr, gltfFile] = fastgltf::MappedGltfFile::FromPath(path);
    if (fileErr != fastgltf::Error::None)
    {
        TL::String msg = fastgltf::getErrorMessage(fileErr).data();
        TL_LOG_ERROR("Failed to load glTF: {}", msg);
        return;
    }

    constexpr auto gltfOptions =
        fastgltf::Options::DontRequireValidAssetMember |
        fastgltf::Options::AllowDouble |
        fastgltf::Options::LoadExternalBuffers |
        fastgltf::Options::GenerateMeshIndices;

    auto [assetError, asset] = parser.loadGltf(gltfFile, path.parent_path(), gltfOptions);
    if (assetError != fastgltf::Error::None)
    {
        TL::String msg = fastgltf::getErrorMessage(assetError).data();
        TL_LOG_ERROR("Failed to load glTF: {}", msg);
        return;
    }

    // Buffers for vertex and index data
    TL::Vector<uint32_t>  indexBufferData;
    TL::Vector<glm::vec3> vertexBufferPositionsData;
    TL::Vector<glm::vec3> vertexBufferNormalsData;
    TL::Vector<glm::vec2> vertexBufferTexcoordsData;

    size_t indexBufferDataOffset           = 0;
    size_t vertexBufferPositionsDataOffset = 0;
    size_t vertexBufferNormalsDataOffset   = 0;
    size_t vertexBufferTexcoordsDataOffset = 0;

    struct SceneDrawable
    {
        RHI::DrawIndexedIndirectCommandArgs drawIndirectArgs;
        uint32_t                            materialIndex;
        glm::mat4                           modelToWorldMatrix;
    };

    TL::Map<uint32_t, SceneDrawable> drawCalls;
    TL::Vector<SceneDrawable>        drawList;

    // Helper function to load primitives
    auto loadPrimitives = [&](const fastgltf::Primitive& primitive, glm::mat4 modelToWorldMatrix) -> SceneDrawable
    {
        auto positionAccessorIndex = primitive.findAttribute("POSITION")->accessorIndex;
        auto normalAccessorIndex   = primitive.findAttribute("NORMAL")->accessorIndex;
        auto texcoordAccessorIndex = primitive.findAttribute("TEXCOORD_0")->accessorIndex;
        auto indicesAccessor       = asset.accessors[primitive.indicesAccessor.value()];
        auto positionsAccessor     = asset.accessors[positionAccessorIndex];
        auto normalsAccessor       = asset.accessors[normalAccessorIndex];
        auto texcoordsAccessor     = asset.accessors[texcoordAccessorIndex];

        // If the position accessor is already in the drawCalls map, reuse existing data
        if (auto it = drawCalls.find(positionAccessorIndex); it != drawCalls.end())
        {
            return SceneDrawable{
                .drawIndirectArgs = {
                    .indexCount   = static_cast<uint32_t>(indicesAccessor.count),
                    .firstIndex   = static_cast<uint32_t>(it->second.drawIndirectArgs.firstIndex),
                    .vertexOffset = static_cast<int32_t>(it->second.drawIndirectArgs.vertexOffset),
                },
                .materialIndex      = static_cast<uint32_t>(primitive.materialIndex.value()),
                .modelToWorldMatrix = modelToWorldMatrix,
            };
        }

        // Calculate the current offsets in the respective buffers
        auto indexBufferCursor = indexBufferData.size();
        auto vbPositionsCursor = vertexBufferPositionsData.size();
        auto vbNormalsCursor   = vertexBufferNormalsData.size();
        auto vbTexcoordsCursor = vertexBufferTexcoordsData.size();

        // Resize buffers and copy data from accessors
        indexBufferData.resize(indexBufferData.size() + indicesAccessor.count);
        fastgltf::copyFromAccessor<uint32_t>(asset, indicesAccessor, indexBufferData.data() + indexBufferCursor);

        vertexBufferPositionsData.resize(vertexBufferPositionsData.size() + positionsAccessor.count);
        fastgltf::copyFromAccessor<fastgltf::math::fvec3>(asset, positionsAccessor, vertexBufferPositionsData.data() + vbPositionsCursor);

        vertexBufferNormalsData.resize(vertexBufferNormalsData.size() + normalsAccessor.count);
        fastgltf::copyFromAccessor<fastgltf::math::fvec3>(asset, normalsAccessor, vertexBufferNormalsData.data() + vbNormalsCursor);

        vertexBufferTexcoordsData.resize(vertexBufferTexcoordsData.size() + texcoordsAccessor.count);
        fastgltf::copyFromAccessor<fastgltf::math::fvec2>(asset, texcoordsAccessor, vertexBufferTexcoordsData.data() + vbTexcoordsCursor);

        // Add to drawCalls and return the SceneDrawable
        return drawCalls[positionAccessorIndex] = {
                   .drawIndirectArgs = {
                       .indexCount   = static_cast<uint32_t>(indicesAccessor.count),
                       .firstIndex   = static_cast<uint32_t>(indexBufferCursor), // Correct offset in the index buffer
                       .vertexOffset = static_cast<int32_t>(vbPositionsCursor),  // Correct starting vertex in the vertex buffer
                   },
                   .materialIndex      = static_cast<uint32_t>(primitive.materialIndex.value()),
                   .modelToWorldMatrix = modelToWorldMatrix,
               };
    };

    for (auto texture : asset.images)
    {
        std::visit(fastgltf::visitor{
                       [](auto& arg)
                       {
                       },
                       [&](fastgltf::sources::URI& filePath)
                       {
                           const char*           cachePath = "C:/Users/abdul/Desktop/Main.1_Sponza/cache/textures";
                           std::filesystem::path path      = filePath.uri.string();
                           path                            = path.filename().replace_extension("dds");
                           path                            = cachePath / path;

                           dds::Image image;
                           auto       res = dds::readFile(path.string().c_str(), &image);

                           RHI::ImageCreateInfo createInfo{
                               .name       = filePath.uri.c_str(),
                               .usageFlags = RHI::ImageUsage::ShaderResource | RHI::ImageUsage::CopyDst,
                               .type       = RHI::ImageType::Image2D,
                               .size       = {image.width, image.height, 1},
                               .format     = RHI::Format::BC7_UNORM,
                           };

                           auto texture = RHI::CreateImageWithContent(*m_device, createInfo, {.ptr = image.mipmaps[0].data(), .size = image.mipmaps[0].size()}).GetValue();
                           m_images.push_back(texture);
                       },
                   },
                   texture.data);
    }

    m_sampler = m_device->CreateSampler({.name = "Sampler"});

    // Iterate scene nodes
    auto [width, height] = m_window->GetWindowSize();
    fastgltf::iterateSceneNodes(
        asset, 0, fastgltf::math::fmat4x4(), [&](fastgltf::Node& node, fastgltf::math::fmat4x4 _matrix)
        {
            auto matrix = convertMatrix(_matrix);
            if (node.meshIndex)
            {
                auto& mesh = asset.meshes[node.meshIndex.value()];
                for (auto& primitive : mesh.primitives)
                {
                    drawList.push_back(loadPrimitives(primitive, matrix));
                }
            }

            if (node.lightIndex)
            {
                auto light = asset.lights[node.lightIndex.value()];
            }

            // if (node.cameraIndex)
            // {
            //     auto [camera, name] = asset.cameras[node.cameraIndex.value()];
            //     Camera newCam;

            //     glm::vec3 position;
            //     glm::quat rotation;
            //     glm::vec3 scale, skew;
            //     glm::vec4 perspective;

            //     // Decompose the transformation matrix
            //     if (glm::decompose(matrix, scale, rotation, position, skew, perspective))
            //     {
            //         newCam.SetPosition(position);
            //         newCam.SetRotation(glm::eulerAngles(rotation)); // Convert quaternion to Euler angles
            //     }
            //     else
            //     {
            //         TL_UNREACHABLE_MSG("Failed to decompose transformation matrix for camera.");
            //     }

            //     if (auto ortho = std::get_if<fastgltf::Camera::Orthographic>(&camera))
            //     {
            //         newCam.SetOrthographic(-ortho->xmag, ortho->xmag, ortho->ymag, -ortho->ymag, ortho->znear, ortho->zfar);
            //     }
            //     else if (auto perspective = std::get_if<fastgltf::Camera::Perspective>(&camera))
            //     {
            //         newCam.SetPerspective(
            //             perspective->yfov,
            //             perspective->aspectRatio.value_or(float(width) / float(height)),
            //             perspective->znear,
            //             perspective->zfar.value_or(10000.0f));
            //     }
            //     else
            //     {
            //         TL_UNREACHABLE_MSG("Unknown camera type.");
            //     }
            //     m_cameras.push_back(newCam);
            // }
        });

    // Create and upload buffers
    RHI::BufferCreateInfo geometryBufferCI{
        .usageFlags = RHI::BufferUsage::Index | RHI::BufferUsage::Vertex,
        .byteSize   = indexBufferData.size() * (sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(uint32_t)),
    };

    m_geometryBufferPool = m_device->CreateBuffer(geometryBufferCI).GetValue();
    {
        m_totalIndexCount  = indexBufferData.size();
        m_totalVertexCount = vertexBufferPositionsData.size();

        auto ptr = (char*)m_device->MapBuffer(m_geometryBufferPool);

        memcpy(ptr, indexBufferData.data(), indexBufferData.size() * sizeof(uint32_t));
        ptr += indexBufferData.size() * sizeof(uint32_t);

        memcpy(ptr, vertexBufferPositionsData.data(), vertexBufferPositionsData.size() * sizeof(glm::vec3));
        ptr += vertexBufferPositionsData.size() * sizeof(glm::vec3);

        memcpy(ptr, vertexBufferNormalsData.data(), vertexBufferNormalsData.size() * sizeof(glm::vec3));
        ptr += vertexBufferNormalsData.size() * sizeof(glm::vec3);

        memcpy(ptr, vertexBufferTexcoordsData.data(), vertexBufferTexcoordsData.size() * sizeof(glm::vec2));
        ptr += vertexBufferTexcoordsData.size() * sizeof(glm::vec2);

        m_device->UnmapBuffer(m_geometryBufferPool);
    }

    for (auto& drawable : drawList)
    {
        const auto& assetMaterial = asset.materials[drawable.materialIndex];

        Shader::PerDraw perDraw{};
        perDraw.modelToWorldMatrix = drawable.modelToWorldMatrix;
        perDraw.material.albedo    = convertVec4(assetMaterial.pbrData.baseColorFactor);
        perDraw.material.metallic  = assetMaterial.pbrData.metallicFactor;
        perDraw.material.roughness = assetMaterial.pbrData.roughnessFactor;
        auto assignTextureIndex = [](const auto& texture) -> uint32_t {
            return texture ? texture->textureIndex : UINT32_MAX;
        };
        perDraw.material.albedoMapIndex = assignTextureIndex(assetMaterial.pbrData.baseColorTexture);
        perDraw.material.normalMapIndex = assignTextureIndex(assetMaterial.normalTexture);
        perDraw.material.metallicRoughnessMapIndex = assignTextureIndex(assetMaterial.pbrData.metallicRoughnessTexture);
        m_perDraws.push_back(perDraw);
        m_drawIndirectArgs.push_back(drawable.drawIndirectArgs);
    }

    RHI::BufferCreateInfo uniformBufferCreateInfo{
        .usageFlags = RHI::BufferUsage::Uniform,
        .byteSize   = (m_perDraws.size() * Shader::kAlignedPerDrawSize) + Shader::kAlignedSceneSize,
    };
    m_uniformBufferPool = m_device->CreateBuffer(uniformBufferCreateInfo).GetValue();

    {
        auto ptr = (char*)m_device->MapBuffer(m_uniformBufferPool) + Shader::kAlignedSceneSize;
        for (size_t i = 0; i < m_perDraws.size(); i++)
        {
            memcpy(ptr, m_perDraws.data() + i, sizeof(Shader::PerDraw));
            ptr += Shader::kAlignedPerDrawSize;
        }
        m_device->UnmapBuffer(m_uniformBufferPool);
    }
}

void Playground::OnInit()
{
    ZoneScoped;

    auto [width, height] = m_window->GetWindowSize();

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

    m_renderGraph     = m_device->CreateRenderGraph();
    m_colorAttachment = m_renderGraph->ImportSwapchain("Color", *m_swapchain, RHI::Format::RGBA8_UNORM);
    m_depthAttachment = m_renderGraph->CreateImage({
        .name       = "Depth",
        .usageFlags = RHI::ImageUsage::Depth,
        .type       = RHI::ImageType::Image2D,
        .size       = {width, height, 1},
        .format     = RHI::Format::D32,
    });

    m_sceneBindGroupLayout = m_device->CreateBindGroupLayout({
        .name     = "BindGroupLayout-ViewUniformBuffer",
        .bindings = {
            {
                .type   = RHI::BindingType::UniformBuffer,
                .stages = RHI::ShaderStage::Pixel | RHI::ShaderStage::Vertex,
            },
            {
                .type   = RHI::BindingType::DynamicUniformBuffer,
                .stages = RHI::ShaderStage::Pixel | RHI::ShaderStage::Vertex,
            },
        },
    });

    m_bindlessTexturesBindGroupLayout = m_device->CreateBindGroupLayout({
        .name     = "BindGroupLayout-ViewUniformBuffer",
        .bindings = {
            {
                .type   = RHI::BindingType::Sampler,
                .stages = RHI::ShaderStage::Pixel,
            },
            {
                .type       = RHI::BindingType::SampledImage,
                .arrayCount = 1048576, // TODO: Bindless
                .stages     = RHI::ShaderStage::Pixel,
            },
        },
    });

    m_graphicsPipelineLayout    = m_device->CreatePipelineLayout({
           .name    = "graphics-pipeline-layout",
           .layouts = {m_sceneBindGroupLayout, m_bindlessTexturesBindGroupLayout},
    });
    m_sceneBindGroup            = m_device->CreateBindGroup(m_sceneBindGroupLayout);
    m_bindlessTexturesBindGroup = m_device->CreateBindGroup(m_bindlessTexturesBindGroupLayout);

    auto vertexModule  = LoadShaderModule(m_device, "./Shaders/Basic.vertex.spv");
    auto pixelModule   = LoadShaderModule(m_device, "./Shaders/Basic.fragment.spv");
    m_graphicsPipeline = m_device->CreateGraphicsPipeline({
        .name                 = "Hello-Triangle",
        .vertexShaderName     = "VSMain",
        .vertexShaderModule   = vertexModule.get(),
        .pixelShaderName      = "PSMain",
        .pixelShaderModule    = pixelModule.get(),
        .layout               = m_graphicsPipelineLayout,
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
        },
        .renderTargetLayout = {
            .colorAttachmentsFormats = RHI::Format::RGBA8_UNORM,
            .depthAttachmentFormat   = RHI::Format::D32,
        },
        .colorBlendState    = {.blendStates = {{.blendEnable = true}}},
        .rasterizationState = {
            .cullMode  = RHI::PipelineRasterizerStateCullMode::BackFace,
            .frontFace = RHI::PipelineRasterizerStateFrontFace::Clockwise,
        },
        .depthStencilState = {
            .depthTestEnable  = true,
            .depthWriteEnable = true,
        },
    });

    LoadScene();

    auto sceneSubregions   = RHI::BufferSubregion{0, Shader::kAlignedSceneSize};
    auto perDrawSubregions = RHI::BufferSubregion{Shader::kAlignedSceneSize, Shader::kAlignedPerDrawSize};
    // Update bind groups
    m_device->UpdateBindGroup(
        m_sceneBindGroup,
        RHI::BindGroupUpdateInfo{
            .buffers = {
                {
                    .dstBinding = 0,
                    .buffers    = {m_uniformBufferPool},
                    .subregions = sceneSubregions,
                },
                {
                    .dstBinding = 1,
                    .buffers    = {m_uniformBufferPool},
                    .subregions = perDrawSubregions,
                },
            },
        });

    m_device->UpdateBindGroup(
        m_bindlessTexturesBindGroup,
        RHI::BindGroupUpdateInfo{
            .images = {
                {
                    .dstBinding = 1,
                    .images     = m_images,
                },
            },
            .samplers = {
                {

                    .dstBinding = 0,
                    .samplers   = m_sampler,
                },
            },
        });
}

void Playground::OnShutdown()
{
    ZoneScoped;

    // if (m_bindGroup) m_device->DestroyBindGroup(m_bindGroup);
    // for (auto image : m_images)
    //     m_device->DestroyImage(image);
    // if (m_sampler) m_device->DestroySampler(m_sampler);
    // if (m_uniformBuffers) m_device->DestroyBuffer(m_uniformBuffers);
    // if (m_geometryVertexBuffer) m_device->DestroyBuffer(m_geometryVertexBuffer);
    // if (m_geometryIndexBuffer) m_device->DestroyBuffer(m_geometryIndexBuffer);

    m_device->DestroyRenderGraph(m_renderGraph);
    m_device->DestroySwapchain(m_swapchain);
    RHI::DestroyVulkanDevice(m_device);
}

void Playground::OnUpdate(Timestep ts)
{
    ZoneScoped;

    auto [width, height] = m_window->GetWindowSize();

    auto& camera = GetActiveCamera();
    camera.Update(ts);
    camera.SetPerspective(width, height, 30.0f, 0.0001f, 1000.0f);

    m_scene =
        {
            .worldToViewMatrix = camera.GetView(),
            .viewToClipMatrix  = camera.GetProjection(),
            // .viewToClipMatrix  = camera.GetProjection() * camera.GetView(),
            // .viewToClipMatrix  =  camera.GetView(),
            .cameraPosition    = camera.GetPosition(),
        };
    m_scene.numDirectionalLights          = 1;
    m_scene.directionalLight[0].color     = {1.0f, 1.0f, 1.0f};
    m_scene.directionalLight[0].direction = {0.4f, -1.0f, -0.4f};
    m_scene.directionalLight[0].intensity = 1.0f;

    auto ptr = (char*)m_device->MapBuffer(m_uniformBufferPool);
    memcpy(ptr, &m_scene, sizeof(Shader::Scene));
    m_device->UnmapBuffer(m_uniformBufferPool);
}

void Playground::Render()
{
    ZoneScoped;

    auto [width, height] = m_window->GetWindowSize();
    m_renderGraph->BeginFrame({width, height});
    [[maybe_unused]] auto pass = m_renderGraph->AddPass({
        .name          = "main-buffer",
        .queue         = RHI::QueueType::Graphics,
        .setupCallback = [&](RHI::RenderGraph& renderGraph, RHI::Pass& pass)
        {
            m_renderGraph->UseRenderTarget(pass, {.attachment = m_colorAttachment, .clearValue = {.f32{0.1f, 0.1f, 0.4f, 1.0f}}});
            m_renderGraph->UseRenderTarget(pass, {.attachment = m_depthAttachment, .clearValue = {.depthStencil = {1.0f, 0}}});
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

            auto indexDataSize    = sizeof(uint32_t) * m_totalIndexCount;   // Total size of index data
            auto positionDataSize = sizeof(glm::vec3) * m_totalVertexCount; // Total size of position data
            auto normalDataSize   = sizeof(glm::vec3) * m_totalVertexCount; // Total size of normal data

            // Calculate offsets
            auto positionOffset = indexDataSize;                     // Positions start after indices
            auto normalOffset   = positionOffset + positionDataSize; // Normals start after positions
            auto texcoordOffset = normalOffset + normalDataSize;     // Texcoords start after normals

            commandList.BindIndexBuffer({.buffer = m_geometryBufferPool}, RHI::IndexType::uint32);
            commandList.BindVertexBuffers(
                0,
                {
                    {.buffer = m_geometryBufferPool, .offset = positionOffset},
                    {.buffer = m_geometryBufferPool, .offset = normalOffset},
                    {.buffer = m_geometryBufferPool, .offset = texcoordOffset},
                });

            // Loop through draw calls and issue them with correct offsets
            for (size_t i = 0; i < m_drawIndirectArgs.size(); ++i)
            {
                const auto& drawArgs = m_drawIndirectArgs[i];

                commandList.BindGraphicsPipeline(
                    m_graphicsPipeline,
                    {
                        {
                            .bindGroup      = m_sceneBindGroup,
                            .dynamicOffsets = {static_cast<uint32_t>(i * Shader::kAlignedPerDrawSize)},
                        },
                        {
                            .bindGroup = m_bindlessTexturesBindGroup,
                        },
                    });

                commandList.Draw({
                    .elementsCount = drawArgs.indexCount,
                    .firstElement  = drawArgs.firstIndex,
                    .vertexOffset  = drawArgs.vertexOffset,
                });
            }
        },
    });

    pass->Resize({width, height});

    m_renderGraph->EndFrame();

    m_device->CollectResources();

    FrameMark;
}

void Playground::OnEvent(Event& event)
{
    ZoneScoped;
    switch (event.GetEventType())
    {
    case EventType::None:
    case EventType::WindowClose:
    case EventType::WindowResize:
        {
            auto& e   = (WindowResizeEvent&)event;
            auto  res = m_swapchain->Recreate({e.GetSize().width, e.GetSize().height});
            TL_ASSERT(res == RHI::ResultCode::Success);
        }
        break;
    case EventType::WindowFocus:
    case EventType::WindowLostFocus:
    case EventType::WindowMoved:
    case EventType::AppTick:
    case EventType::AppUpdate:
    case EventType::AppRender:
    case EventType::KeyPressed:
    case EventType::KeyReleased:
    case EventType::KeyTyped:
    case EventType::MouseButtonPressed:
    case EventType::MouseButtonReleased:
    case EventType::MouseMoved:
    case EventType::MouseScrolled:
    default:
        GetActiveCamera().ProcessEvent(event, *m_window);
        break;
    }
}

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
