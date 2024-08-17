#pragma once
#include <Examples-Base/Window.hpp>

#include "ShaderInterface/Core.slang"

#include <RHI/RHI.hpp>

namespace RPI
{
    class Window;

    struct MaterialIds
    {
        uint32_t diffuseID = UINT32_MAX;
        uint32_t specularID = UINT32_MAX;
        uint32_t ambientID = UINT32_MAX;
        uint32_t emissiveID = UINT32_MAX;
        uint32_t heightID = UINT32_MAX;
        uint32_t normalsID = UINT32_MAX;
        uint32_t shininessID = UINT32_MAX;
        uint32_t opacityID = UINT32_MAX;
        uint32_t displacementID = UINT32_MAX;
        uint32_t lightmapID = UINT32_MAX;
        uint32_t reflectionID = UINT32_MAX;
        uint32_t baseColorID = UINT32_MAX;
        uint32_t normalCameraID = UINT32_MAX;
        uint32_t emissionColorID = UINT32_MAX;
        uint32_t metalnessID = UINT32_MAX;
        uint32_t diffuseRoughnessID = UINT32_MAX;
        uint32_t ambientOcclusionID = UINT32_MAX;
        uint32_t sheenID = UINT32_MAX;
        uint32_t clearcoatID = UINT32_MAX;
        uint32_t transmissionID = UINT32_MAX;
    };

    class Mesh
    {
    public:
        uint32_t elementsCount;

        RHI::Handle<RHI::Buffer> m_index;
        RHI::Handle<RHI::Buffer> m_position;
        RHI::Handle<RHI::Buffer> m_normal;
        RHI::Handle<RHI::Buffer> m_texCoord;
    };

    class Scene
    {
    public:
        Scene(RHI::Context* context)
            : m_context(context)
        {
        }

        ~Scene() = default;

        // private:

        RHI::Context* m_context;

        glm::mat4 m_projectionMatrix;
        glm::mat4 m_viewMatrix;

        TL::Vector<Mesh*> m_meshes;

        TL::Vector<MaterialIds> materialIDs;
        TL::Vector<RHI::Handle<RHI::Image>> images;
        TL::Vector<RHI::Handle<RHI::ImageView>> imagesViews;

        TL::Vector<ObjectTransform> m_meshesTransform;
        TL::Vector<uint32_t> m_meshesStatic;
    };

    class Renderer
    {
    public:
        Renderer();

        virtual ~Renderer();

        RHI::ResultCode Init(const Examples::Window& window);

        void Shutdown();

        void Render(const Scene& scene);

        RHI::Result<RHI::Handle<RHI::Image>> CreateImageWithData(const RHI::ImageCreateInfo& createInfo, TL::Block content);
        RHI::Result<RHI::Handle<RHI::Buffer>> CreateBufferWithData(TL::Flags<RHI::BufferUsage> usageFlags, TL::Block content);
        RHI::Handle<RHI::Image> CreateImage(const char* filePath);

        TL::Ptr<Scene> CreateScene();

        virtual RHI::ResultCode OnInit() = 0;

        virtual void OnShutdown() = 0;

        virtual void OnRender(const Scene& scene) = 0;

        // protected:
        const Examples::Window* m_window;

        TL::Ptr<RHI::Context> m_context;
        TL::Ptr<RHI::Swapchain> m_swapchain;
        TL::Ptr<RHI::CommandPool> m_commandPool[2];
        TL::Ptr<RHI::Fence> m_frameFence[2];
    };

    void LoadScene(Renderer& renderer, Scene& scene, const char* sceneFileLocation);
    Renderer* CreateDeferredRenderer();
} // namespace Examples