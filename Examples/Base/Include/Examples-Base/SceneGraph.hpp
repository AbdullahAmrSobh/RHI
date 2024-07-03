#pragma once

#include "Examples-Base/Common.hpp"


#include <glm/glm.hpp>
#include <RHI/RHI.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Shader
{
#include "ShaderInterface/Core.slang"
}

namespace MaterialTextureMapKind
{
    enum MaterialTextureMapKind_T
    {
        Color,
        Normal,
        Roughness,
        Metallic,
        Count,
    };
}

namespace MeshVertexAttributeType
{
    enum MeshVertexAttributeType_T
    {
        Index,
        Position,
        Normal,
        Color,
        TexCoord,
        Count
    };
}

struct Material
{
    Handle<RHI::Image> image[];
};

struct StaticMesh
{
    std::string name;

    uint32_t elementsCount;

    RHI::BufferBindingInfo attributes[MeshVertexAttributeType::Count];

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

    Shader::SceneTransform m_perFrameData = {};
    TL::Vector<Shader::ObjectTransform> m_perDrawData; // todo: make as array

    Shader::DirectionalLight m_dirLight;
    TL::Vector<Shader::PointLight> m_pointLights;
    TL::Vector<Shader::SpotLight> m_spotLights;

    RHI::HandlePool<Material> m_materialOwner = RHI::HandlePool<Material>();
    RHI::HandlePool<StaticMesh> m_staticMeshOwner = RHI::HandlePool<StaticMesh>();

    TL::Vector<RHI::Handle<StaticMesh>> m_meshes;
    TL::Vector<RHI::Handle<Material>> m_materials;

    // Rendering related

    RHI::Scissor m_scissor;
    RHI::Viewport m_viewport;

    TL::Vector<SceneGraphNode> m_graph;

    RHI::Handle<RHI::Buffer> m_objectModelMatercies;
    RHI::Handle<RHI::Sampler> m_sampler;
};
