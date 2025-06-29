// Fix swapchain resizing and moving into other monitor
// Fix validation errors and change API if needed!
// Fix all memory and resource leaks!

#include <RHI/RHI.hpp>

#include <TL/Defer.hpp>
#include <TL/FileSystem/File.hpp>
#include <TL/Log.hpp>
#include <TL/Utils.hpp>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <tracy/Tracy.hpp>

#include "Examples-Base/ApplicationBase.hpp"
#include "Examples-Base/Window.hpp"
#include "Renderer/Renderer.hpp"
#include "Camera.hpp"
#include "ImGuiManager.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace Engine;

inline static glm::mat4x4 ToGlmMatrix(const aiMatrix4x4& matrix)
{
    return glm::mat4(matrix.a1, matrix.b1, matrix.c1, matrix.d1, matrix.a2, matrix.b2, matrix.c2, matrix.d2, matrix.a3, matrix.b3, matrix.c3, matrix.d3, matrix.a4, matrix.b4, matrix.c4, matrix.d4);
}

class Playground final : public ApplicationBase
{
public:
    TL::Ptr<Engine::ImGuiManager> m_imguiManager = TL::CreatePtr<Engine::ImGuiManager>();
    TL::Ptr<Engine::Renderer>     m_renderer     = TL::CreatePtr<Engine::Renderer>();
    Engine::PresentationViewport  m_presentationViewport;

    Engine::Scene*     m_scene;
    Engine::SceneView* m_sceneView;
    Camera             m_camera;

    Playground()
        : ApplicationBase("", 1600, 900) // Empty title, will be set in OnInit
    {
        m_camera.SetPerspective(1600, 900, 45, 0.00001, 1000000);
    }

    Engine::StaticMeshLOD* ImportStaticMesh(const aiMesh* mesh) const
    {
        // Indices
        TL::Vector<uint32_t> indices;
        for (unsigned i = 0; i < mesh->mNumFaces; ++i)
        {
            const aiFace& face = mesh->mFaces[i];
            for (unsigned j = 0; j < face.mNumIndices; ++j)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        // Positions
        TL::Vector<glm::vec3> positions;
        positions.reserve(mesh->mNumVertices);
        for (unsigned i = 0; i < mesh->mNumVertices; ++i)
        {
            const aiVector3D& v = mesh->mVertices[i];
            positions.emplace_back(v.x, v.y, v.z);
        }

        // Normals
        TL::Vector<glm::vec3> normals;
        if (mesh->HasNormals())
        {
            normals.reserve(mesh->mNumVertices);
            for (unsigned i = 0; i < mesh->mNumVertices; ++i)
            {
                const aiVector3D& n = mesh->mNormals[i];
                normals.emplace_back(n.x, n.y, n.z);
            }
        }

        // UVs (only first channel)
        TL::Vector<glm::vec2> uvs;
        if (mesh->HasTextureCoords(0))
        {
            uvs.reserve(mesh->mNumVertices);
            for (unsigned i = 0; i < mesh->mNumVertices; ++i)
            {
                const aiVector3D& uv = mesh->mTextureCoords[0][i];
                uvs.emplace_back(uv.x, uv.y);
            }
        }

        auto* staticMesh = Engine::GeometryBufferPool::ptr->CreateStaticMeshLOD(indices, positions, normals, uvs);

        return staticMesh;
    }

    // Helper function to recursively enumerate all meshes in the scene graph.
    // The callback is called as: callback(meshIndex, globalTransform, node)
    void EnumerateSceneMeshes(const aiScene* scene, TL::Function<void(uint32_t, aiMatrix4x4, const aiNode*)> callback, const aiNode* node = nullptr, const aiMatrix4x4& parentTransform = aiMatrix4x4()) const
    {
        if (!node)
            node = scene->mRootNode;

        // Compute the global transform for this node
        aiMatrix4x4 globalTransform = parentTransform * node->mTransformation;

        // Iterate over all meshes referenced by this node
        for (unsigned i = 0; i < node->mNumMeshes; ++i)
        {
            unsigned meshIndex = node->mMeshes[i];
            callback(meshIndex, globalTransform, node);
        }

        // Recurse into children
        for (unsigned i = 0; i < node->mNumChildren; ++i)
        {
            EnumerateSceneMeshes(scene, callback, node->mChildren[i], globalTransform);
        }
    }

    // Iterates over all textures in the scene (embedded, referenced, and material textures)
    // and calls the provided callback for each texture found.
    // The callback is called as: callback(texturePathOrId, aiTextureType, aiTexture*)
    void ImportTextures(const aiScene* scene, TL::Function<void(const std::string&, aiTextureType, const aiTexture*)> callback) const
    {
        // 1. Embedded textures (scene->mTextures)
        for (unsigned i = 0; i < scene->mNumTextures; ++i)
        {
            const aiTexture* tex   = scene->mTextures[i];
            // Embedded textures are referenced by "*<index>" in material slots
            std::string      texId = "*" + std::to_string(i);
            callback(texId, aiTextureType_NONE, tex);
        }

        // 2. Material-referenced textures (file paths or embedded)
        for (unsigned m = 0; m < scene->mNumMaterials; ++m)
        {
            const aiMaterial* material = scene->mMaterials[m];
            // Iterate over all possible texture types
            for (int type = aiTextureType_NONE; type <= aiTextureType_UNKNOWN; ++type)
            {
                aiTextureType texType  = static_cast<aiTextureType>(type);
                unsigned int  texCount = material->GetTextureCount(texType);
                for (unsigned int t = 0; t < texCount; ++t)
                {
                    aiString path;
                    if (material->GetTexture(texType, t, &path) == AI_SUCCESS)
                    {
                        const char*      texPath     = path.C_Str();
                        // If the path starts with '*', it's an embedded texture
                        const aiTexture* embeddedTex = nullptr;
                        if (texPath[0] == '*')
                        {
                            int idx = std::atoi(texPath + 1);
                            if (idx >= 0 && idx < int(scene->mNumTextures))
                                embeddedTex = scene->mTextures[idx];
                        }
                        callback(texPath, texType, embeddedTex);
                    }
                }
            }
        }
    }

    void ImportScene(const aiScene* scene)
    {
        EnumerateSceneMeshes(scene, [this, scene](uint32_t index, aiMatrix4x4 modelToWorldMat, const aiNode* node)
            {
                if (index > 40)
                    return;

                auto mesh       = scene->mMeshes[index];
                auto staticMesh = ImportStaticMesh(mesh);
                m_scene->AddStaticMesh(staticMesh, ToGlmMatrix(modelToWorldMat));
            });

        ImportTextures(scene, [this, scene](const std::string& path, aiTextureType type, const aiTexture* texture)
            {
                // TL_LOG_INFO("Loading texture: {}", path);
            });
    }

    void LoadFromArgPath()
    {
        Assimp::Importer importer;
        auto             flags = aiProcess_MakeLeftHanded |
                     aiProcess_Triangulate |
                     aiProcess_GenNormals |
                     aiProcess_PreTransformVertices;
        auto scene = importer.ReadFile("C:/Users/abdul/Desktop/Main.1_Sponza/NewSponza_Main_glTF_002.gltf", flags);
        ImportScene(scene);
    }

    void OnInit() override
    {
        ZoneScoped;

        RHI::BackendType backend;
        switch (ApplicationBase::GetLaunchSettings().backend)
        {
        case Engine::CommandLine::LaunchSettings::Backend::Vulkan:
            backend = RHI::BackendType::Vulkan1_3;
            m_window->SetTitle("Playground - RHI::Vulkan 1.3");
            break;
        case Engine::CommandLine::LaunchSettings::Backend::WebGPU:
            backend = RHI::BackendType::WebGPU;
            m_window->SetTitle("Playground - RHI::WebGPU");
            break;
        case Engine::CommandLine::LaunchSettings::Backend::D3D12:
            backend = RHI::BackendType::DirectX12_2;
            m_window->SetTitle("Playground - RHI::D3D12");
            break;
        default:
            m_window->SetTitle("Playground - RHI::Vulkan 1.3");
            backend = RHI::BackendType::Vulkan1_3;
            break;
        }

        m_imguiManager->Init(m_window);
        auto result = m_renderer->Init(backend);
        TL_ASSERT(RHI::IsSuccess(result), "Failed to initialize renderer");
        m_presentationViewport = m_renderer->CreatePresentationViewport(m_window);
        auto handler           = [this](const auto& e)
        {
            m_camera.ProcessEvent(e);
            return false;
        };
        m_window->Subscribe(handler);

        m_scene     = m_renderer->CreateScene();
        m_sceneView = m_scene->CreateView();

        GPU::SceneView sceneViewData{
            glm::identity<glm::mat4x4>(),
            glm::identity<glm::mat4x4>(),
        };
        m_sceneView->m_sceneViewUB.Update(sceneViewData);

        LoadFromArgPath();
    }

    void OnShutdown() override
    {
        ZoneScoped;

        m_imguiManager->Shutdown();
        m_renderer->DestroyPresentationViewport(m_presentationViewport);
        m_scene->DestroyView(m_sceneView);
        m_renderer->DestroyScene(m_scene);
        m_renderer->Shutdown();
    }

    void OnUpdate(Timestep ts) override
    {
        ZoneScoped;

        m_camera.Update(ts);

        auto worldToView = m_camera.GetView();
        auto viewToClip  = m_camera.GetProjection();

        GPU::SceneView sceneViewData{
            worldToView,
            viewToClip,
        };

        m_sceneView->m_sceneViewUB.Update(sceneViewData);
    }

    void Render() override
    {
        ZoneScoped;

        // Begin ImGui frame and create a fullscreen dockspace over the main viewport

        ImGui::NewFrame();
        // ImGui::DockSpaceOverViewport();
        // Example window
        ImGui::Begin("Window1");
        ImGui::Text("hello");

        ImGui::End();
        ImGui::ShowDemoWindow();
        ImGui::Render();
        ImGui::UpdatePlatformWindows();

        if (ImGui::IsKeyDown(ImGuiKey_F1))
        {
            m_renderer->GetRenderGraph()->Debug_CaptureNextFrame();
        }


        m_renderer->Render(m_scene, m_presentationViewport);

        FrameMark;
    }
};

#include <Examples-Base/Entry.hpp>

int main(int argc, const char* argv[])
{
    TL::Span args{argv, (size_t)argc};
    // TL::MemPlumber::start();
    auto     result = Entry<Playground>(args);
    // size_t memLeakCount, memLeakSize;
    // TL::MemPlumber::memLeakCheck(memLeakCount, memLeakSize);
    return result;
}
