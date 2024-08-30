#include "Assets/Importer.hpp"
#include "Assets/Mesh.hpp"
#include "Assets/Image.hpp"
#include "Assets/SceneGraph.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <TL/Containers.hpp>
#include <TL/Span.hpp>
#include <TL/Block.hpp>
#include <TL/FileSystem/FileSystem.hpp>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <filesystem>
#include <functional>

namespace Examples::Assets
{
    template<typename V>
    void GetAssimpMaterialProperty(const aiMaterial& aiMaterial, const char* propertyKey, uint32_t slot, uint32_t index, V& materialParameterProperty)
    {
        using PropertyType = typename V::value_type;
        PropertyType property;
        if (aiMaterial.Get(propertyKey, slot, index, property) == aiReturn_SUCCESS)
        {
            materialParameterProperty = property;
        }
    }

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

    inline static TL::Vector<uint32_t> ConvertAssimpFaces(TL::Span<aiFace> faces)
    {
        TL::Vector<uint32_t> indexData;
        indexData.reserve(faces.size() * 3);
        for (uint32_t faceIndex = 0; faceIndex < faces.size(); faceIndex++)
        {
            const auto& face = faces[faceIndex];
            TL_ASSERT(face.mNumIndices == 3)
            indexData.push_back(face.mIndices[0]);
            indexData.push_back(face.mIndices[1]);
            indexData.push_back(face.mIndices[2]);
        }
        return indexData;
    }

    inline static TL::Vector<glm::vec2> ConvertAssimpTextureCoords(TL::Span<aiVector3D> uvw)
    {
        TL::Vector<glm::vec2> uvs;
        uvs.reserve(uvw.size());
        for (auto vec : uvw)
        {
            uvs.push_back({ vec.x, vec.y });
        }
        return uvs;
    }

    struct Converter
    {
        std::filesystem::path scenePath;
        std::filesystem::path outputPath;
        std::filesystem::path compressonatorPath;

        TL::UnorderedMap<std::filesystem::path, std::filesystem::path> pathLookup; // maps an assimp old path to new exported path

        std::filesystem::path ResolvePath(std::filesystem::path oldPath, std::filesystem::path newPath)
        {
            pathLookup[oldPath] = newPath;
            return newPath;
        }

        std::filesystem::path ImportScene(const aiScene& aiScene, TL::Span<std::filesystem::path> meshes, TL::Span<std::filesystem::path> materials)
        {
            SceneGraph sceneGraph;

            std::function<void(const aiNode*, SceneGraph::Node*)> convertNode = [&](const aiNode* aiNode, SceneGraph::Node* parentNode)
            {
                // Create a new node in the scene graph
                SceneGraph::Node* node = sceneGraph.AddNode(parentNode);

                // Convert Assimp's transformation matrix to glm::mat4
                node->relativeTransform = ConvertMatrix(aiNode->mTransformation);

                // Assume each aiNode has either a mesh, material, light, or camera attached (simplified).
                for (auto meshIndex = 0; meshIndex < aiNode->mNumMeshes; meshIndex++)
                {
                    auto aiMesh = aiScene.mMeshes[meshIndex];
                    Model model{
                        .mesh = TL::String(meshes[meshIndex].string()),
                        .material = TL::String(materials[aiMesh->mMaterialIndex].string()),
                        .transform = ConvertMatrix(aiNode->mTransformation)
                    };
                    node->models.push_back(model);
                }

                // Light and Camera would be processed similarly if present

                // Recursively convert child nodes
                for (uint32_t i = 0; i < aiNode->mNumChildren; ++i)
                {
                    convertNode(aiNode->mChildren[i], node);
                }
            };

            // Start conversion from the root node
            convertNode(aiScene.mRootNode, nullptr); // Root node has no parent, so pass -1 or a sentinel value
            auto fileName = outputPath / "scene.fgscene";
            std::fstream file{ fileName, std::ios::binary | std::ios::out };
            TL::BinaryArchive encoder{ file };
            encoder.Encode(sceneGraph);
            return fileName;
        }

        TL::Vector<std::filesystem::path> ImportMeshes(TL::Span<aiMesh* const> aiMeshes)
        {
            auto currentSaveDirectory = outputPath / "./meshes";
            if (std::filesystem::exists(currentSaveDirectory) == false)
            {
                TL_ASSERT(std::filesystem::create_directory(currentSaveDirectory));
            }
            TL::Vector<std::filesystem::path> output;
            for (auto aiMesh : aiMeshes)
            {
                Mesh mesh(aiMesh->mName.C_Str());
                if (aiMesh->HasFaces())
                {
                    auto indexData = ConvertAssimpFaces(TL::Span<aiFace>{ aiMesh->mFaces, aiMesh->mNumFaces });
                    mesh.AddAttribute<uint32_t>(AttributeNames::Faces, indexData);
                }
                if (aiMesh->HasPositions())
                {
                    mesh.AddAttribute<aiVector3D>(AttributeNames::Positions, { aiMesh->mVertices, aiMesh->mNumVertices });
                }
                if (aiMesh->HasNormals())
                {
                    mesh.AddAttribute<aiVector3D>(AttributeNames::Normals, { aiMesh->mNormals, aiMesh->mNumVertices });
                }
                if (aiMesh->HasTextureCoords(0))
                {
                    auto texCoordData = ConvertAssimpTextureCoords({ aiMesh->mTextureCoords[0], aiMesh->GetNumUVChannels() });
                    mesh.AddAttribute<glm::vec2>(AttributeNames::TextureCoords, texCoordData);
                }

                // TODO add other attributes if/when needed

                auto fileName = currentSaveDirectory / aiMesh->mName.C_Str();
                fileName.replace_extension(".fgmesh");

                std::fstream file{ fileName, std::ios::binary | std::ios::out };
                TL::BinaryArchive encoder{ file };
                encoder.Encode(mesh);
                output.push_back(fileName.lexically_relative(outputPath));
            }
            return output;
        }

        TL::Vector<std::filesystem::path> ImportMaterials(TL::Span<aiMaterial* const> aiMaterials)
        {
            auto currentSaveDirectory = outputPath / "./materials";
            if (std::filesystem::exists(currentSaveDirectory) == false)
            {
                TL_ASSERT(std::filesystem::create_directory(currentSaveDirectory));
            }

            uint32_t nameIndex = 0;
            TL::Vector<std::filesystem::path> output;
            for (auto aiMaterial : aiMaterials)
            {
                Material material{ aiMaterial->GetName().C_Str() };
                GetAssimpMaterialProperty(*aiMaterial, AI_MATKEY_COLOR_DIFFUSE, material.colorDiffuse);
                GetAssimpMaterialProperty(*aiMaterial, AI_MATKEY_COLOR_SPECULAR, material.colorSpecular);
                GetAssimpMaterialProperty(*aiMaterial, AI_MATKEY_COLOR_AMBIENT, material.colorAmbient);
                GetAssimpMaterialProperty(*aiMaterial, AI_MATKEY_COLOR_EMISSIVE, material.colorEmissive);
                GetAssimpMaterialProperty(*aiMaterial, AI_MATKEY_COLOR_TRANSPARENT, material.colorTransparent);
                GetAssimpMaterialProperty(*aiMaterial, AI_MATKEY_COLOR_REFLECTIVE, material.colorReflective);
                GetAssimpMaterialProperty(*aiMaterial, AI_MATKEY_REFLECTIVITY, material.reflectivity);
                GetAssimpMaterialProperty(*aiMaterial, AI_MATKEY_ENABLE_WIREFRAME, material.wireframe);
                GetAssimpMaterialProperty(*aiMaterial, AI_MATKEY_TWOSIDED, material.twosided);
                GetAssimpMaterialProperty(*aiMaterial, AI_MATKEY_SHADING_MODEL, material.shadingModel);
                GetAssimpMaterialProperty(*aiMaterial, AI_MATKEY_BLEND_FUNC, material.blendFunc);
                GetAssimpMaterialProperty(*aiMaterial, AI_MATKEY_OPACITY, material.opacity);
                GetAssimpMaterialProperty(*aiMaterial, AI_MATKEY_SHININESS, material.shininess);
                GetAssimpMaterialProperty(*aiMaterial, AI_MATKEY_SHININESS_STRENGTH, material.shininessStrength);
                GetAssimpMaterialProperty(*aiMaterial, AI_MATKEY_REFRACTI, material.refracti);

                auto getTexture = [&](aiTextureType type, uint32_t slot)
                {
                    std::filesystem::path texturesSaveDir = "./textures";

                    aiString temp;
                    aiMaterial->GetTexture(type, slot, &temp);

                    std::filesystem::path oldPath = temp.C_Str();
                    std::filesystem::path newPath = texturesSaveDir / oldPath.filename().replace_extension(".dds");
                    pathLookup[oldPath] = newPath;
                    return newPath;
                };

                material.specularMap = getTexture(aiTextureType_DIFFUSE, 0);
                material.diffuseMap = getTexture(aiTextureType_SPECULAR, 0);
                material.ambientMap = getTexture(aiTextureType_AMBIENT, 0);
                material.emissiveMap = getTexture(aiTextureType_EMISSIVE, 0);
                material.heightMap = getTexture(aiTextureType_HEIGHT, 0);
                material.normalsMap = getTexture(aiTextureType_NORMALS, 0);
                material.shininessMap = getTexture(aiTextureType_SHININESS, 0);
                material.opacityMap = getTexture(aiTextureType_OPACITY, 0);
                material.displacementMap = getTexture(aiTextureType_DISPLACEMENT, 0);
                material.lightmapMap = getTexture(aiTextureType_LIGHTMAP, 0);
                material.reflectionMap = getTexture(aiTextureType_REFLECTION, 0);
                material.baseColorMap = getTexture(aiTextureType_BASE_COLOR, 0);
                material.normalCameraMap = getTexture(aiTextureType_NORMAL_CAMERA, 0);
                material.emissionColorMap = getTexture(aiTextureType_EMISSION_COLOR, 0);
                material.metalnessMap = getTexture(aiTextureType_METALNESS, 0);
                material.diffuseRoughnessMap = getTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0);
                material.ambientOcclusionMap = getTexture(aiTextureType_AMBIENT_OCCLUSION, 0);
                material.sheenMap = getTexture(aiTextureType_SHEEN, 0);
                material.clearcoatMap = getTexture(aiTextureType_CLEARCOAT, 0);
                material.transmissionMap = getTexture(aiTextureType_TRANSMISSION, 0);
                material.unknownMap = getTexture(aiTextureType_UNKNOWN, 0);

                auto fileName = currentSaveDirectory / aiMaterial->GetName().C_Str();
                fileName.replace_extension(".fgmesh");
                if (aiMaterial->GetName().length == 0)
                {
                    fileName = currentSaveDirectory / std::format("unnamed-{}.fgmesh", nameIndex++);
                }

                std::fstream file{ fileName, std::ios::binary | std::ios::out };
                TL::BinaryArchive encoder{ file };
                encoder.Encode(material);
                output.push_back(fileName.lexically_relative(outputPath));
            }
            return output;
        }

        TL::Vector<std::filesystem::path> ImportTextures()
        {
            auto currentSaveDirectory = outputPath / "./textures";
            if (std::filesystem::exists(currentSaveDirectory) == false)
            {
                TL_ASSERT(std::filesystem::create_directory(currentSaveDirectory));
            }
            TL::Vector<std::filesystem::path> output;
            for (auto [oldPath, newPath] : pathLookup)
            {
                auto fullOldPath = scenePath.parent_path() / oldPath;
                auto fullNewPath = outputPath / newPath;
                auto command = std::format("{} -EncodeWith GPU -fd {} {} {}", compressonatorPath.string(), "BC7", fullOldPath.string(), fullNewPath.string());
                std::system(command.c_str());
                output.push_back(newPath);
            }
            return output;
        }
    };

    Package Import(std::filesystem::path file)
    {
        Assimp::Importer aiImporter;
        auto aiSceneFlags = aiProcess_Triangulate | aiProcess_GenSmoothNormals;
        const auto& aiScene = *aiImporter.ReadFile(file.string().c_str(), aiSceneFlags);

        std::filesystem::create_directory(file.parent_path() / "cache");

        Converter converter;
        converter.scenePath = file;
        converter.outputPath = file.parent_path() / "cache";
        /// @todo find the path to compressonatorcli through downloaded cmake package
        converter.compressonatorPath = "C:/Users/abdul/Desktop/compressonatorcli-4.5.52-win64/compressonatorcli.exe";
        auto meshes = converter.ImportMeshes({ aiScene.mMeshes, aiScene.mNumMeshes });
        auto materials = converter.ImportMaterials({ aiScene.mMaterials, aiScene.mNumMaterials });
        auto sceneGraphs = converter.ImportScene(aiScene, meshes, materials);
        auto textures = converter.ImportTextures();

        /// @todo load embedded textures

        Package package(file.filename().string().c_str());
        package.AddMeshs(meshes);
        package.AddMaterials(materials);
        package.AddImages(textures);
        return package;
    }
} // namespace Examples::Assets