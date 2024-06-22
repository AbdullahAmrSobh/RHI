#include "Examples-Base/ApplicationBase.hpp"
#include "Examples-Base/SceneGraph.hpp"

#include <functional>

inline static glm::mat4 ConvertMatrix(const aiMatrix4x4& matrix)
{
    // clang-format off
    glm::mat4 result(matrix.a1, matrix.b1, matrix.c1, matrix.d1,
                     matrix.a2, matrix.b2, matrix.c2, matrix.d2,
                     matrix.a3, matrix.b3, matrix.c3, matrix.d3,
                     matrix.a4, matrix.b4, matrix.c4, matrix.d4);
    // clang-format on
    return result;
}

inline static glm::vec3 ConvertVector(const aiVector3D& vector)
{
    return { vector.x, vector.y, vector.z };
}

inline static glm::vec3 ConvertColor(const aiColor3D& color)
{
    return { color.r, color.g, color.b };
}

inline static void ProcessNode(RHI::Context& context, glm::mat4 parentTransform, const aiNode* node, std::function<void(glm::mat4, const aiNode&)> fn)
{
    fn(parentTransform, *node);

    auto currentTransform = ConvertMatrix(node->mTransformation);
    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        auto childTransform = ConvertMatrix(node->mChildren[i]->mTransformation);
        ProcessNode(context, parentTransform * currentTransform * childTransform, node->mChildren[i], fn);
    }
}

Scene::Scene(RHI::Context* pContext, const char* scenePath)
{
    auto& context = *pContext;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(scenePath, aiProcess_Triangulate | aiProcess_GenSmoothNormals);

    TL::UnorderedMap<uint32_t, RHI::Handle<Material>> materialLookup;
    TL::UnorderedMap<uint32_t, RHI::Handle<StaticMesh>> staticMeshLookup;

    TL::UnorderedMap<const char*, TL::Vector<aiLight>> lightsLookup;

    // load scene lights
    for (uint32_t i = 0; i < scene->mNumLights; i++)
    {
        auto& light = *scene->mLights[i];
        lightsLookup[light.mName.C_Str()].push_back(light);
    }

    // load materials from scene
    // for (uint32_t i = 0; i < scene->mNumMaterials; i++)
    // {
    //     auto& material = *scene->mMaterials[i];
    //     materialLookup[i] = LoadMaterial(context, material);
    // }

    // load meshes from scene
    for (uint32_t i = 0; i < scene->mNumMeshes; i++)
    {
        auto& mesh = *scene->mMeshes[i];
        auto materialHandle = materialLookup[mesh.mMaterialIndex];
        staticMeshLookup[i] = LoadStaticMesh(context, materialHandle, mesh);
    }

    // load scene static meshes
    auto processNodeCallback = [&](glm::mat4 nodeTransform, const aiNode& aiNode)
    {
        // load node lights
        for (auto light : lightsLookup[aiNode.mName.C_Str()])
        {
            auto position = nodeTransform * glm::vec4{ ConvertVector(light.mPosition), 1.0f };
            auto direction = nodeTransform * glm::vec4{ ConvertVector(light.mDirection), 1.0f };

            switch (light.mType)
            {
            case aiLightSource_UNDEFINED:
            case aiLightSource_DIRECTIONAL:
                {
                    Shader::DirectionalLight dirLight{};
                    dirLight.direction = direction;
                    dirLight.ambientColor = ConvertColor(light.mColorAmbient);
                    dirLight.diffuseColor = ConvertColor(light.mColorDiffuse);
                    dirLight.specularColor = ConvertColor(light.mColorSpecular);
                }
                break;
            case aiLightSource_POINT:
                {
                    Shader::PointLight pointLight{};
                    pointLight.position = position;
                    pointLight.ambientColor = ConvertColor(light.mColorAmbient);
                    pointLight.diffuseColor = ConvertColor(light.mColorDiffuse);
                    pointLight.specularColor = ConvertColor(light.mColorSpecular);
                    pointLight.attuation = light.mAttenuationLinear;
                }
                break;
            case aiLightSource_SPOT:
                {
                    Shader::SpotLight spotLight{};
                    spotLight.position = position;
                    spotLight.direction = direction;
                    spotLight.innerAngle = light.mAngleInnerCone;
                    spotLight.ambientColor = ConvertColor(light.mColorAmbient);
                    spotLight.diffuseColor = ConvertColor(light.mColorDiffuse);
                    spotLight.specularColor = ConvertColor(light.mColorSpecular);
                }
            case aiLightSource_AMBIENT:
            case aiLightSource_AREA:
            case _aiLightSource_Force32Bit: break;
            }
        }

        SceneGraphNode node{};

        for (uint32_t i = 0; i < aiNode.mNumMeshes; i++)
        {
            node.m_meshes.push_back(staticMeshLookup[aiNode.mMeshes[i]]);
        }

        if (aiNode.mNumMeshes)
        {
            m_staticSceneNodes.push_back(node);
            Shader::ObjectTransform perDraw{};
            perDraw.modelMatrix = nodeTransform;
            m_perDrawData.push_back(perDraw);
        }
    };

    ProcessNode(context, ConvertMatrix(scene->mRootNode->mTransformation), scene->mRootNode, processNodeCallback);
}

void Scene::Shutdown(RHI::Context& context)
{
    for (auto meshHandle : m_meshes)
    {
        auto mesh = m_staticMeshOwner.Get(meshHandle);
        if (auto buffer = mesh->indcies)
        {
            context.DestroyBuffer(buffer);
        }

        if (auto buffer = mesh->position)
        {
            context.DestroyBuffer(buffer);
        }

        if (auto buffer = mesh->normals)
        {
            context.DestroyBuffer(buffer);
        }

        if (auto buffer = mesh->uvs)
        {
            context.DestroyBuffer(buffer);
        }

        if (auto buffer = mesh->colors)
        {
            context.DestroyBuffer(buffer);
        }
    }

    context.DestroyBuffer(m_perFrameUniformBuffer);
    context.DestroyBuffer(m_perObjectUniformBuffer);
    context.DestroySampler(m_sampler);
    context.DestroyBindGroupLayout(m_bindGroupLayout);
    context.DestroyBindGroup(m_bindGroup);
    context.DestroyPipelineLayout(m_pipelineLayout);
    context.DestroyGraphicsPipeline(m_pbrPipeline);
}

// RHI::Handle<Material> Scene::LoadMaterial( [[maybe_unused]] RHI::Context& context, const aiMaterial& aiMaterial)
// {
//     aiString albedoPath, normalPath, roughnessPath, metallicPath;
//     aiMaterial.GetTexture(aiTextureType_DIFFUSE, 0, &albedoPath);
//     aiMaterial.GetTexture(aiTextureType_NORMALS, 0, &normalPath);
//     aiMaterial.GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughnessPath);
//     aiMaterial.GetTexture(aiTextureType_METALNESS, 0, &metallicPath);

//     TL::String prefixPath = "C:/Users/abdul/Desktop/Main.1_Sponza/"; // TODO: add proper fix

//     RHI::ImageCreateInfo imageInfo{};
//     imageInfo.usageFlags = RHI::ImageUsage::ShaderResource;
//     imageInfo.usageFlags |= RHI::ImageUsage::CopyDst;
//     imageInfo.format = RHI::Format::BC1_UNORM;
//     imageInfo.type = RHI::ImageType::Image2D;
//     imageInfo.size.depth = 1;
//     imageInfo.mipLevels = 1;
//     imageInfo.arrayCount = 1;
//     imageInfo.sampleCount = RHI::SampleCount::Samples1;

//     RHI::Handle<RHI::Image> albedoImage = RHI::NullHandle;
//     if (albedoPath.length)
//     {
//         auto resourcePath = prefixPath + albedoPath.C_Str();
//         resourcePath.replace(resourcePath.length() - 3, resourcePath.length(), "dds");
//         auto imageData = ReadBinaryFile(resourcePath);
//         imageInfo.name = resourcePath.c_str();
//         imageInfo.size.width = 4096;
//         imageInfo.size.height = 4096;
//         // albedoImage = RHI::CreateImageWithData<uint8_t>(context, imageInfo, imageData).GetValue();
//     }

//     RHI::Handle<RHI::Image> normalImage = RHI::NullHandle;
//     if (normalPath.length)
//     {
//         auto resourcePath = prefixPath + normalPath.C_Str();
//         resourcePath.replace(resourcePath.length() - 3, resourcePath.length(), "dds");
//         auto imageData = ReadBinaryFile(resourcePath);
//         imageInfo.name = resourcePath.c_str();
//         imageInfo.size.width = 4096;
//         imageInfo.size.height = 4096;
//         // albedoImage = RHI::CreateImageWithData<uint8_t>(context, imageInfo, imageData).GetValue();
//     }

//     RHI::Handle<RHI::Image> roughnessImage = RHI::NullHandle;
//     if (roughnessPath.length)
//     {
//         auto resourcePath = prefixPath + roughnessPath.C_Str();
//         resourcePath.replace(resourcePath.length() - 3, resourcePath.length(), "dds");
//         auto imageData = ReadBinaryFile(resourcePath);
//         imageInfo.name = resourcePath.c_str();
//         imageInfo.size.width = 4096;
//         imageInfo.size.height = 4096;
//         // albedoImage = RHI::CreateImageWithData<uint8_t>(context, imageInfo, imageData).GetValue();
//     }

//     RHI::Handle<RHI::Image> metallicImage = RHI::NullHandle;
//     if (metallicPath.length)
//     {
//         auto resourcePath = prefixPath + metallicPath.C_Str();
//         resourcePath.replace(resourcePath.length() - 3, resourcePath.length(), "dds");
//         auto imageData = ReadBinaryFile(resourcePath);
//         imageInfo.name = resourcePath.c_str();
//         imageInfo.size.width = 4096;
//         imageInfo.size.height = 4096;
//         // albedoImage = RHI::CreateImageWithData<uint8_t>(context, imageInfo, imageData).GetValue();
//     }

//     Material material{};
//     material.albedoMap = albedoImage;
//     material.normalMap = normalImage;
//     material.roughnessMap = roughnessImage;
//     material.metallicMap = metallicImage;
//     auto materialHandle = m_materialOwner.Emplace(std::move(material));
//     m_materials.push_back(materialHandle);
//     return materialHandle;
// }

RHI::Handle<StaticMesh> Scene::LoadStaticMesh(RHI::Context& context, RHI::Handle<Material> material, const aiMesh& aiMesh)
{
    StaticMesh mesh{};
    mesh.name = std::string(aiMesh.mName.C_Str());
    mesh.material = material;

    // load mesh data
    if (aiMesh.HasPositions())
    {
        mesh.position = RHI::CreateBufferWithData<aiVector3D>(context, RHI::BufferUsage::Vertex, TL::Span{ aiMesh.mVertices, aiMesh.mNumVertices }).GetValue();
        mesh.elementsCount = aiMesh.mNumVertices;
    }

    if (aiMesh.HasNormals())
    {
        mesh.normals = RHI::CreateBufferWithData<aiVector3D>(context, RHI::BufferUsage::Vertex, TL::Span{ aiMesh.mNormals, aiMesh.mNumVertices }).GetValue();
    }

    if (aiMesh.HasTextureCoords(0))
    {
        TL::Vector<glm::vec2> uvs;
        uvs.reserve(aiMesh.mNumVertices);
        for (uint32_t i = 0; i < aiMesh.mNumVertices; i++)
        {
            uvs.push_back({ aiMesh.mTextureCoords[0][i].x, aiMesh.mTextureCoords[0][i].y });
        }
        mesh.uvs = RHI::CreateBufferWithData<glm::vec2>(context, RHI::BufferUsage::Vertex, uvs).GetValue();
    }

    if (aiMesh.HasFaces())
    {
        TL::Vector<uint32_t> indices;
        indices.reserve(aiMesh.mNumFaces * 3);
        for (uint32_t i = 0; i < aiMesh.mNumFaces; i++)
        {
            indices.push_back(aiMesh.mFaces[i].mIndices[0]);
            indices.push_back(aiMesh.mFaces[i].mIndices[1]);
            indices.push_back(aiMesh.mFaces[i].mIndices[2]);
        }
        mesh.indcies = RHI::CreateBufferWithData<uint32_t>(context, RHI::BufferUsage::Index, indices).GetValue();
        mesh.elementsCount = (uint32_t)indices.size();
    }

    auto handle = m_staticMeshOwner.Emplace(std::move(mesh));
    m_meshes.push_back(handle);
    return handle;
}
