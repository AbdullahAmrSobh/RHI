#include "Assets/Importer.hpp"

#include "Assets/Package.hpp"
#include "Assets/Image.hpp"
#include "Assets/Mesh.hpp"
#include "Assets/SceneGraph.hpp"
#include "Assets/Material.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <TL/Containers.hpp>
#include <TL/Span.hpp>
#include <TL/Block.hpp>
#include <TL/FileSystem/FileSystem.hpp>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <functional>

namespace Examples::Assets
{
    // TODO: should be computed at runtime
    static constexpr const char* CompressonatorPath = "C:/Users/abdul/Desktop/compressonatorcli-4.5.52-win64/compressonatorcli.exe";

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
        const char* inputPath;
        const char* outputPath;

        TL::Map<TL::String, TL::String> pathLookup;

        const char* ResolvePath(const char* oldPath, const char* newPath)
        {
            pathLookup[oldPath] = newPath;
            return newPath;
        }

        TL::String SaveLocation(TL::String filename)
        {
            return TL::String(outputPath) + "/" + filename;
        }

        /// @brief Imports an Assimp scene into a SceneGraph.
        /// @param aiScene The Assimp scene to import.
        /// @param meshes A span of mesh names to reference in the scene.
        /// @param materials A span of material names to reference in the scene.
        /// @return The path where the scene graph was saved.
        // TL::String ImportScene(const aiScene& aiScene, TL::Span<TL::String> meshes, TL::Span<TL::String> materials)
        // {
        //     SceneGraph sceneGraph;

        //     std::function<void(const aiNode*, SceneNode*)> convertNode = [&](const aiNode* aiNode, SceneNode* parentNode)
        //     {
        //         // Create a new scene node and attach it to the parent.
        //         SceneNode* node = new SceneNode(aiNode->mName.C_Str());
        //         if (parentNode)
        //         {
        //             parentNode->AddChild(node);
        //         }
        //         else
        //         {
        //             sceneGraph.AddNode(node);
        //         }

        //         // Set the node's transformation.
        //         node->SetTransform(ConvertMatrix(aiNode->mTransformation));

        //         // Process mesh references.
        //         for (uint32_t meshIndex = 0; meshIndex < aiNode->mNumMeshes; ++meshIndex)
        //         {
        //             uint32_t aiMeshIndex = aiNode->mMeshes[meshIndex];
        //             if (aiMeshIndex < meshes.size())
        //             {
        //                 node->AddMesh(meshes[aiMeshIndex].c_str());
        //             }
        //         }

        //         // Process material references.
        //         for (uint32_t meshIndex = 0; meshIndex < aiNode->mNumMeshes; ++meshIndex)
        //         {
        //             auto aiMesh = aiScene.mMeshes[aiNode->mMeshes[meshIndex]];
        //             uint32_t materialIndex = aiMesh->mMaterialIndex;
        //             if (materialIndex < materials.size())
        //             {
        //                 node->AddMaterial(materials[materialIndex].c_str());
        //             }
        //         }

        //         // Recursively process children nodes.
        //         for (uint32_t i = 0; i < aiNode->mNumChildren; ++i)
        //         {
        //             convertNode(aiNode->mChildren[i], node);
        //         }
        //     };

        //     // Start the conversion from the root node.
        //     convertNode(aiScene.mRootNode, nullptr);

        //     // Save the scene graph to a file.
        //     auto path = SaveLocation("scene.fgscene");
        //     TL::BinaryArchive::Save(sceneGraph, path.c_str());

        //     return path;
        // }

        TL::Vector<TL::String> ImportMeshes(TL::Span<aiMesh* const> aiMeshes)
        {
            TL::String currentSaveDirectory = TL::String(outputPath) + "/meshes";
            if (std::filesystem::exists(currentSaveDirectory) == false)
            {
                TL_ASSERT(std::filesystem::create_directory(currentSaveDirectory));
            }

            TL::Vector<TL::String> output;
            uint32_t count = 0;
            for (auto aiMesh : aiMeshes)
            {
                Mesh mesh(aiMesh->mName.C_Str());
                if (aiMesh->HasFaces())
                {
                    auto indexData = ConvertAssimpFaces(TL::Span<aiFace>{ aiMesh->mFaces, aiMesh->mNumFaces });
                    mesh.AddAttribute<uint32_t>(AttributeNames::Indcies, indexData);
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

                auto fileName = SaveLocation(std::format("meshes/{}-{}.fgmesh", aiMesh->mName.C_Str(), count++).c_str());
                TL::BinaryArchive::Save(mesh, fileName.c_str());
                output.push_back(fileName);
            }
            return output;
        }

        TL::Vector<TL::String> ImportMaterials(TL::Span<aiMaterial* const> aiMaterials)
        {
            TL::String currentSaveDirectory = TL::String(outputPath) + "/materials";
            if (std::filesystem::exists(currentSaveDirectory) == false)
            {
                TL_ASSERT(std::filesystem::create_directory(currentSaveDirectory));
            }

            uint32_t nameIndex = 0;
            TL::Vector<TL::String> output;
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
                    pathLookup[oldPath.string().c_str()] = newPath.string().c_str();
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

                auto fileName = SaveLocation(std::format("materials/{}-{}.fgmaterial", aiMaterial->GetName().C_Str(), nameIndex++).c_str());
                TL::BinaryArchive::Save(material, fileName.c_str());
                output.push_back(fileName);
            }
            return output;
        }

        TL::Vector<TL::String> ImportTextures()
        {
            TL::String currentSaveDirectory = TL::String(outputPath) + "/textures";
            if (std::filesystem::exists(currentSaveDirectory) == false)
            {
                TL_ASSERT(std::filesystem::create_directory(currentSaveDirectory));
            }

            TL::Vector<TL::String> output;
            for (auto [oldPath, newPath] : pathLookup)
            {
                auto fullOldPath = std::filesystem::path(inputPath) / oldPath;
                auto command = std::format("{} -EncodeWith GPU -fd {} {} {}", CompressonatorPath, "BC7", fullOldPath.string(), currentSaveDirectory.c_str());
                std::system(command.c_str());
                output.push_back(newPath);
            }
            return output;
        }
    };

    Package Import(const ImportInfo& importInfo)
    {
        std::filesystem::path filePath = importInfo.filePath;
        uint64_t modifiyTime = 0;
        {
            modifiyTime = std::filesystem::last_write_time(filePath.parent_path()).time_since_epoch().count();
        }

        Assimp::Importer importer;
        const auto& aiScene = *importer.ReadFile(importInfo.filePath, aiProcess_Triangulate | aiProcess_GenSmoothNormals);

        std::filesystem::create_directory(importInfo.outputPath);

        Converter converter;
        converter.inputPath = filePath.parent_path().string().c_str();
        converter.outputPath = importInfo.outputPath;
        auto meshes = converter.ImportMeshes({ aiScene.mMeshes, aiScene.mNumMeshes });
        auto materials = converter.ImportMaterials({ aiScene.mMaterials, aiScene.mNumMaterials });
        auto textures = converter.ImportTextures();
        // auto sceneGraphs = converter.ImportScene(aiScene, meshes, materials);

        Package package(importInfo.packageName, modifiyTime);
        package.AddMeshes(meshes);
        package.AddMaterials(materials);
        package.AddImages(textures);
        // package.AddSceneGraphs(sceneGraphs);
        return package;
    }
} // namespace Examples::Assets