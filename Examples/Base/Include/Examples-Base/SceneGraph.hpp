#pragma once

#include <glm/glm.hpp>
#include <RHI/RHI.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace TL = RHI::TL;

namespace Shader
{
    inline static constexpr uint32_t MAX_LIGHTS = 16;

    struct alignas(16) DirLight
    {
        glm::vec3 direction;
        float _padding0;
        glm::vec3 ambientColor;
        float _padding1;
        glm::vec3 specularColor;
        float _padding2;
        glm::vec3 diffuseColor;
        float _padding3;
    };

    static_assert(sizeof(DirLight) % 16 == 0, "DirLight must be aligned to 16 bytes");

    struct alignas(16) PointLight
    {
        glm::vec3 position;
        float radius;
        glm::vec3 ambientColor;
        float _padding1;
        glm::vec3 specularColor;
        float _padding2;
        glm::vec3 diffuseColor;
        float _padding3;

        float attuation;
        float _padding4[3];
    };

    static_assert(sizeof(PointLight) % 16 == 0, "PointLight must be aligned to 16 bytes");

    struct alignas(16) SpotLight
    {
        glm::vec3 position;
        float radius;
        glm::vec3 direction;
        float innerAngle;

        glm::vec3 ambientColor;
        float _padding1;
        glm::vec3 specularColor;
        float _padding2;
        glm::vec3 diffuseColor;
        float _padding3;

        float attuation;
        float _padding4[3];
    };

    static_assert(sizeof(SpotLight) % 16 == 0, "SpotLight must be aligned to 16 bytes");

    struct alignas(16) PerFrame
    {
        glm::mat4 viewProjectionMatrix;
        glm::mat4 projectionMatrix;
        glm::mat4 viewMatrix;
        glm::mat4 inverseViewMatrix;

        DirLight dirLight;
        PointLight pointLights[MAX_LIGHTS];
        SpotLight spotLights[MAX_LIGHTS];
    };

    static_assert(sizeof(PerFrame) % 16 == 0, "PerFrame must be aligned to 16 bytes");

    struct alignas(16) PerDraw
    {
        glm::mat4 modelMatrix;
    };

    static_assert(sizeof(PerDraw) % 16 == 0, "ModelData must be aligned to 16 bytes");

}; // namespace Shader

struct Material
{
    RHI::Handle<RHI::Image> albedoMap;
    RHI::Handle<RHI::ImageView> albedoMapView;

    RHI::Handle<RHI::Image> normalMap;
    RHI::Handle<RHI::ImageView> normalMapView;

    RHI::Handle<RHI::Image> roughnessMap;
    RHI::Handle<RHI::ImageView> roughnessMapView;

    RHI::Handle<RHI::Image> metallicMap;
    RHI::Handle<RHI::ImageView> metallicMapView;

    RHI::Handle<RHI::BindGroup> bindGroup;
};

struct StaticMesh
{
    std::string name;

    uint32_t elementsCount;

    RHI::Handle<RHI::Buffer> indcies;
    RHI::Handle<RHI::Buffer> position;
    RHI::Handle<RHI::Buffer> normals;
    RHI::Handle<RHI::Buffer> uvs;
    RHI::Handle<RHI::Buffer> colors;

    RHI::Handle<Material> material;
};

class SceneGraphNode
{
public:
    glm::mat4 m_transform;
    TL::Vector<RHI::Handle<StaticMesh>> m_meshes;
};

class Scene
{
public:
    Scene(RHI::Context* context, const char* scenePath);

    void Shutdown(RHI::Context& context);

    RHI::Handle<Material> LoadMaterial(RHI::Context& context, const aiMaterial& material);
    RHI::Handle<StaticMesh> LoadStaticMesh(RHI::Context& context, RHI::Handle<Material> material, const aiMesh& mesh);

    Shader::PerFrame m_perFrameData = {};
    TL::Vector<Shader::PerDraw> m_perDrawData; // todo: make as array

    Shader::DirLight m_dirLight;
    TL::Vector<Shader::PointLight> m_pointLights;
    TL::Vector<Shader::SpotLight> m_spotLights;

    RHI::HandlePool<Material> m_materialOwner = RHI::HandlePool<Material>();
    RHI::HandlePool<StaticMesh> m_staticMeshOwner = RHI::HandlePool<StaticMesh>();

    TL::Vector<RHI::Handle<StaticMesh>> m_meshes;
    TL::Vector<RHI::Handle<Material>> m_materials;

    // Rendering related

    TL::Vector<SceneGraphNode> m_staticSceneNodes;

    RHI::Handle<RHI::Buffer> m_perFrameUniformBuffer;
    RHI::Handle<RHI::Buffer> m_perObjectUniformBuffer;
    RHI::Handle<RHI::Sampler> m_sampler;

    RHI::Handle<RHI::BindGroupLayout> m_bindGroupLayout;
    RHI::Handle<RHI::BindGroup> m_bindGroup;

    RHI::Handle<RHI::PipelineLayout> m_pipelineLayout;
    RHI::Handle<RHI::GraphicsPipeline> m_pbrPipeline;
};
