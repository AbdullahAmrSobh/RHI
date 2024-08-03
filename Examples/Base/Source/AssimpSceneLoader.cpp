#include "Examples-Base/AssimpSceneLoader.hpp"
#include "Examples-Base/Renderer.hpp"
#include <Examples-Base/Scene.hpp>
#include <Examples-Base/Log.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Examples
{
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

    inline static TL2::Vector<aiVector2D> TruncateToVector2D(TL2::Span<const aiVector3D> values)
    {
        TL2::Vector<aiVector2D> result;
        result.resize(values.size());
        for (uint32_t i = 0; i < values.size(); i++)
        {
            result[i].x = values[i].x;
            result[i].y = values[i].y;
        }
        return result;
    }

    inline static void ProcessNode(glm::mat4 parentTransform, const aiNode* node, std::function<void(glm::mat4, const aiNode&)> callback)
    {
        callback(parentTransform, *node);
        auto transform = parentTransform * ConvertMatrix(node->mTransformation);
        for (uint32_t i = 0; i < node->mNumChildren; i++)
        {
            auto currentNode = node->mChildren[i];
            ProcessNode(ConvertMatrix(currentNode->mTransformation) * transform, currentNode, callback);
        }
    }

    inline static TL2::String ResolvePath(std::filesystem::path scenePath, TL2::String subPath)
    {
        auto fullPath = scenePath.parent_path() / subPath;
        fullPath.replace_extension(".dds");
        return { fullPath.string().c_str() };
    }

    void AssimpScenneLoader::LoadScene(Renderer& renderer, Scene& scene, const char* sceneFileLocation, const char* sceneTextureLocation)
    {
        (void)sceneTextureLocation;

        Assimp::Importer importer;
        auto aiSceneFlags = aiProcess_Triangulate | aiProcess_GenSmoothNormals;
        const aiScene* aiScene = importer.ReadFile(sceneFileLocation, aiSceneFlags);

        for (uint32_t i = 0; i < aiScene->mNumMeshes; i++)
        {
            const auto& aiMesh = *aiScene->mMeshes[i];

            auto mesh = scene.m_meshes.emplace_back(new Mesh());

            RHI_ASSERT(aiMesh.HasPositions());
            {
                mesh->m_position = renderer.CreateBufferWithData<aiVector3D>(RHI::BufferUsage::Vertex, { aiMesh.mVertices, aiMesh.mNumVertices }).GetValue();
                mesh->elementsCount = (uint32_t)aiMesh.mNumVertices;
            }

            RHI_ASSERT(aiMesh.HasNormals());
            {
                mesh->m_normal = renderer.CreateBufferWithData<aiVector3D>(RHI::BufferUsage::Vertex, { aiMesh.mNormals, aiMesh.mNumVertices }).GetValue();
            }

            RHI_ASSERT(aiMesh.HasTextureCoords(0));
            {
                auto texCoordData = TruncateToVector2D({ aiMesh.mTextureCoords[0], aiMesh.mNumVertices });
                mesh->m_texCoord = renderer.CreateBufferWithData<aiVector2D>(RHI::BufferUsage::Vertex, texCoordData).GetValue();
            }

            if (aiMesh.HasFaces())
            {
                TL2::Vector<uint32_t> indices;
                indices.reserve(aiMesh.mNumFaces * 3);
                for (uint32_t j = 0; j < aiMesh.mNumFaces; j++)
                {
                    indices.push_back(aiMesh.mFaces[j].mIndices[0]);
                    indices.push_back(aiMesh.mFaces[j].mIndices[1]);
                    indices.push_back(aiMesh.mFaces[j].mIndices[2]);
                }

                mesh->m_index = renderer.CreateBufferWithData<uint32_t>(RHI::BufferUsage::Index, indices).GetValue();
                mesh->elementsCount = (uint32_t)indices.size();
            }
        }

        for (uint32_t i = 0; i < aiScene->mNumMaterials; i++)
        {
            const auto& aiMaterial = *aiScene->mMaterials[i];
            Core::LogInfo("Loading Material {}", aiMaterial.GetName().C_Str());

            MaterialIds materialProperty{};
            aiString diffusePath, normalPath;
            if (auto result = aiMaterial.GetTexture(aiTextureType_DIFFUSE, 0, &diffusePath); result == AI_SUCCESS)
            {
                auto path = ResolvePath(sceneFileLocation, diffusePath.C_Str());
                Core::LogInfo("\t Loading {}", path);
                scene.images.push_back(renderer.CreateImage(path.c_str()));
                materialProperty.diffuseID = (uint32_t)scene.images.size() - 1;

                RHI::ImageViewCreateInfo imageViewCI{};
                imageViewCI.image = scene.images.back();
                imageViewCI.viewType = RHI::ImageViewType::View2D;
                imageViewCI.components = RHI::ComponentMapping{ RHI::ComponentSwizzle::Identity, RHI::ComponentSwizzle::Identity, RHI::ComponentSwizzle::Identity, RHI::ComponentSwizzle::Identity };
                imageViewCI.subresource.imageAspects = RHI::ImageAspect::Color;
                imageViewCI.subresource.arrayCount = 1;
                imageViewCI.subresource.mipLevelCount = 1;
                scene.imagesViews.push_back(renderer.m_context->CreateImageView(imageViewCI));
            }
            if (auto result = aiMaterial.GetTexture(aiTextureType_NORMALS, 0, &normalPath); result == AI_SUCCESS)
            {
                auto path = ResolvePath(sceneFileLocation, normalPath.C_Str());
                Core::LogInfo("\t Loading {}", path);
                scene.images.push_back(renderer.CreateImage(path.c_str()));
                materialProperty.normalID = (uint32_t)scene.images.size() - 1;

                RHI::ImageViewCreateInfo imageViewCI{};
                imageViewCI.image = scene.images.back();
                imageViewCI.viewType = RHI::ImageViewType::View2D;
                imageViewCI.components = RHI::ComponentMapping{ RHI::ComponentSwizzle::Identity, RHI::ComponentSwizzle::Identity, RHI::ComponentSwizzle::Identity, RHI::ComponentSwizzle::Identity };
                imageViewCI.subresource.imageAspects = RHI::ImageAspect::Color;
                imageViewCI.subresource.arrayCount = 1;
                imageViewCI.subresource.mipLevelCount = 1;
                scene.imagesViews.push_back(renderer.m_context->CreateImageView(imageViewCI));
            }
            scene.materialIDs.push_back(materialProperty);
        }

        // build scene graph tree
        ProcessNode(
            glm::identity<glm::mat4>(),
            aiScene->mRootNode,
            [&](glm::mat4 modelMatrix, const aiNode& aiNode)
        {
            for (uint32_t i = 0; i < aiNode.mNumMeshes; i++)
            {
                auto mesh = aiScene->mMeshes[aiNode.mMeshes[i]];
                auto material = scene.materialIDs[mesh->mMaterialIndex];

                ObjectTransform transform{};
                transform.modelMatrix = modelMatrix;
                transform.colorIndex = material.diffuseID;
                transform.normalIndex = material.normalID;

                scene.m_meshesTransform.push_back(transform);
                scene.m_meshesStatic.push_back(aiNode.mMeshes[i]);
            }
        });
    }
} // namespace Examples