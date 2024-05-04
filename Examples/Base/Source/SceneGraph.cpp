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

// inline static std::string PrefixString(const char* prefix, aiString string)
// {
//     std::string result = prefix;
//     result += string.C_Str();
//     return result;
// }

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
                    Shader::DirLight dirLight{};
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
            Shader::PerDraw perDraw{};
            perDraw.modelMatrix = nodeTransform;
            m_perDrawData.push_back(perDraw);
        }
    };

    ProcessNode(context, ConvertMatrix(scene->mRootNode->mTransformation), scene->mRootNode, processNodeCallback);

    SetupGPUResources(context);
    LoadPipeline(context, "./Shaders/Basic.spv");
}

void Scene::UpdateUniformBuffers(RHI::Context& context, glm::mat4 view, glm::mat4 projection)
{
    m_perFrameData.viewMatrix = view;
    m_perFrameData.projectionMatrix = projection;
    m_perFrameData.viewProjectionMatrix = view * projection;
    m_perFrameData.inverseViewMatrix = glm::inverse(view);

    auto ptr = context.MapBuffer(m_perFrameUniformBuffer);
    memcpy(ptr, &m_perFrameData, sizeof(Shader::PerFrame));
    context.UnmapBuffer(m_perFrameUniformBuffer);
}

void Scene::Draw(RHI::CommandList& commandList) const
{
    ZoneScoped;
    RHI::Handle<RHI::BindGroup> bindGroups = { m_bindGroup };

    RHI::DrawInfo drawInfo{};
    drawInfo.pipelineState = m_pbrPipeline;
    drawInfo.bindGroups = { bindGroups };

    uint32_t nodeIndex = 0;
    for (const auto& node : m_staticSceneNodes)
    {
        for (const auto& handle : node.m_meshes)
        {
            auto mesh = m_staticMeshOwner.Get(handle);
            RHI::Handle<RHI::BindGroup> bindGroupss[] = { m_bindGroup };
            drawInfo.bindGroups = bindGroupss;
            drawInfo.dynamicOffset = { uint32_t(sizeof(Shader::PerDraw) * nodeIndex) };
            drawInfo.parameters.elementCount = mesh->elementsCount;
            drawInfo.vertexBuffers = { mesh->position, mesh->normals };
            if (mesh->indcies != RHI::NullHandle)
            {
                drawInfo.indexBuffers = mesh->indcies;
            }

            commandList.Draw(drawInfo);
        }
        nodeIndex++;
    }
}

void Scene::SetupGPUResources(RHI::Context& context)
{
    m_sampler = context.CreateSampler(RHI::SamplerCreateInfo{});

    RHI::BindGroupLayoutCreateInfo bindGroupLayoutInfo{};
    // Per frame uniform buffer
    bindGroupLayoutInfo.bindings[0].access = RHI::Access::Read;
    bindGroupLayoutInfo.bindings[0].stages |= RHI::ShaderStage::Vertex;
    bindGroupLayoutInfo.bindings[0].stages |= RHI::ShaderStage::Pixel;
    bindGroupLayoutInfo.bindings[0].type = RHI::ShaderBindingType::UniformBuffer;
    bindGroupLayoutInfo.bindings[0].arrayCount = 1;

    // per object uniform buffer
    bindGroupLayoutInfo.bindings[1].access = RHI::Access::Read;
    bindGroupLayoutInfo.bindings[1].stages |= RHI::ShaderStage::Vertex;
    bindGroupLayoutInfo.bindings[1].stages |= RHI::ShaderStage::Pixel;
    bindGroupLayoutInfo.bindings[1].type = RHI::ShaderBindingType::DynamicUniformBuffer;
    bindGroupLayoutInfo.bindings[1].arrayCount = 1;

    // TODO: add textures

    m_bindGroupLayout = context.CreateBindGroupLayout(bindGroupLayoutInfo);
    m_pipelineLayout = context.CreatePipelineLayout({ m_bindGroupLayout });
    m_bindGroup = context.CreateBindGroup(m_bindGroupLayout);

    // create buffers

    RHI::BufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.usageFlags = RHI::BufferUsage::Uniform;
    bufferCreateInfo.byteSize = sizeof(Shader::PerFrame);
    m_perFrameUniformBuffer = context.CreateBuffer(bufferCreateInfo).GetValue();
    bufferCreateInfo.byteSize = sizeof(Shader::PerDraw) * m_perDrawData.size();
    m_perObjectUniformBuffer = context.CreateBuffer(bufferCreateInfo).GetValue();

    auto ptr = context.MapBuffer(m_perObjectUniformBuffer);
    memcpy(ptr, m_perDrawData.data(), m_perDrawData.size() * sizeof(Shader::PerDraw));
    context.UnmapBuffer(m_perObjectUniformBuffer);

    // update bind groups
    RHI::BindGroupData data{};
    data.BindBuffers(0, m_perFrameUniformBuffer);
    data.BindBuffers(1, m_perObjectUniformBuffer, true, sizeof(Shader::PerDraw));
    context.UpdateBindGroup(m_bindGroup, data);
}

void Scene::LoadPipeline(RHI::Context& context, const char* shaderPath)
{
    auto shaderCode = ReadBinaryFile(shaderPath);
    auto shaderModule = context.CreateShaderModule(shaderCode);
    RHI::GraphicsPipelineCreateInfo createInfo{};
    // clang-format off
    createInfo.inputAssemblerState.attributes[0] = { .location = 0, .binding = 0, .format = RHI::Format::RGB32_FLOAT, .offset = 0 };
    createInfo.inputAssemblerState.attributes[1] = { .location = 1, .binding = 1, .format = RHI::Format::RGB32_FLOAT, .offset = 0 };
    // createInfo.inputAssemblerState.attributes[2] = { .location = 2, .binding = 2, .format = RHI::Format::RG32_FLOAT,  .offset = 0 };
    createInfo.inputAssemblerState.bindings[0] = { .binding = 0, .stride = RHI::GetFormatByteSize(RHI::Format::RGB32_FLOAT), .stepRate = RHI::PipelineVertexInputRate::PerVertex };
    createInfo.inputAssemblerState.bindings[1] = { .binding = 1, .stride = RHI::GetFormatByteSize(RHI::Format::RGB32_FLOAT), .stepRate = RHI::PipelineVertexInputRate::PerVertex };
    // createInfo.inputAssemblerState.bindings[2] = { .binding = 2, .stride = RHI::GetFormatByteSize(RHI::Format::RG32_FLOAT),  .stepRate = RHI::PipelineVertexInputRate::PerVertex };
    // clang-format on
    createInfo.vertexShaderName = "VSMain";
    createInfo.pixelShaderName = "PSMain";
    createInfo.vertexShaderModule = shaderModule.get();
    createInfo.pixelShaderModule = shaderModule.get();
    createInfo.layout = m_pipelineLayout;
    createInfo.renderTargetLayout.colorAttachmentsFormats[0] = RHI::Format::BGRA8_UNORM;
    createInfo.renderTargetLayout.depthAttachmentFormat = RHI::Format::D32;
    createInfo.depthStencilState.depthTestEnable = true;
    createInfo.depthStencilState.depthWriteEnable = true;
    createInfo.depthStencilState.compareOperator = RHI::CompareOperator::Less;
    m_pbrPipeline = context.CreateGraphicsPipeline(createInfo);
}

// RHI::Handle<Material> Scene::LoadMaterial(RHI::Context& context, const aiMaterial& aiMaterial)
// {
//     aiString albedoPath, normalPath, roughnessPath, metallicPath;
//     aiMaterial.GetTexture(aiTextureType_DIFFUSE, 0, &albedoPath);
//     aiMaterial.GetTexture(aiTextureType_NORMALS, 0, &normalPath);
//     aiMaterial.GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughnessPath);
//     aiMaterial.GetTexture(aiTextureType_METALNESS, 0, &metallicPath);

//     const char* prefixPath = "I:/Main.1_Sponza/"; // TODO: add proper fix
//     auto albedoData = LoadImage(PrefixString(prefixPath, albedoPath));
//     // auto normalData = LoadImage(PrefixString(prefixPath, normalPath));
//     // auto roughnessData = LoadImage(PrefixString(prefixPath, roughnessPath));
//     // auto metallicData = LoadImage(PrefixString(prefixPath, metallicPath));

//     RHI::ImageCreateInfo imageInfo{};
//     imageInfo.name = "albedo";
//     imageInfo.usageFlags = RHI::ImageUsage::ShaderResource;
//     imageInfo.usageFlags |= RHI::ImageUsage::CopyDst;
//     imageInfo.format = RHI::Format::RGBA8_UNORM;
//     imageInfo.type = RHI::ImageType::Image2D;
//     imageInfo.size.width = albedoData.width;
//     imageInfo.size.height = albedoData.height;
//     imageInfo.size.depth = 1;
//     imageInfo.mipLevels = 1;
//     imageInfo.arrayCount = 1;
//     imageInfo.sampleCount = RHI::SampleCount::Samples1;
//     auto albedoTexture = RHI::CreateImageWithData<uint8_t>(context, imageInfo, albedoData.data).GetValue();
//     // imageInfo.debugName = "normal";
//     // auto normalTexture = RHI::CreateImageWithData<uint8_t>(context, imageInfo, normalData.data).GetValue();
//     // imageInfo.debugName = "roughness";
//     // imageInfo.format = RHI::Format::RGBA8_UNORM;
//     // auto roughnessTexture = RHI::CreateImageWithData<uint8_t>(context, imageInfo, roughnessData.data).GetValue();
//     // imageInfo.debugName = "metallic";
//     // auto metallicTexture = RHI::CreateImageWithData<uint8_t>(context, imageInfo, metallicData.data).GetValue();

//     auto [handle, material] = m_materialOwner.New();
//     material.albedoMap = albedoTexture;
//     // material.normalMap = normalTexture;
//     // material.roughnessMap = roughnessTexture;
//     // material.metallicMap = metallicTexture;
//     return handle;
// }

RHI::Handle<StaticMesh> Scene::LoadStaticMesh(RHI::Context& context, RHI::Handle<Material> material, const aiMesh& aiMesh)
{
    StaticMesh mesh {};
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
        // mesh.uvs = RHI::CreateBufferWithData<aiVector2D>(context, RHI::BufferUsage::Vertex, TL::Span{ aiMesh.mTextureCoords[0], aiMesh.mNumVertices }).GetValue();
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

    return m_staticMeshOwner.Emplace(std::move(mesh));
}
