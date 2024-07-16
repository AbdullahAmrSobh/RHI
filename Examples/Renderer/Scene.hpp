#pragma once

#include <RHI/RHI.hpp>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

enum class Result;

namespace TL
{
    template<typename T>
    using Handle = RHI::Handle<T>;
    template<typename T>
    using Ptr = RHI::Ptr<T>;
    // containers, allocators, memory-management, utilities, threads

    using String = RHI::TL::String;
    template<typename T>
    using Vector = RHI::TL::Vector<T>;
} // namespace TL

namespace RE
{
    struct ModelDesc;
    struct MeshDesc;
    struct SkinnedMeshDesc;
    struct MaterialDesc;
    struct LightDesc;
    struct SkyboxDesc;
    struct TerrainDesc;
    struct OceanDesc;
    struct RayCastResult;
    struct Ray;
    struct TextDesc;
    struct CircleDesc;
    struct RectngleDesc;
    struct LineDesc;
    struct CurveDesc;
    struct BoxDesc;
    struct SphereDesc;
    struct CylinderDesc;
    struct ConeDesc;
    struct TorusDesc;
    struct PlaneDesc;
    struct GizmoDesc;
    struct FontDesc;

    class Primitive;
    class Frustum;
    class Camera;
    class Model;
    class Mesh;
    class SkinnedMesh;
    class Material;
    class Light;
    class Skybox;
    class Terrain;
    class Ocean;
    class BoundingBox;
    class Text;
    class Font;

    class Scene
    {
    public:
        virtual ~Scene() = default;

        virtual Result Init() = 0;

        virtual void Shutdown() = 0;

        virtual Camera* CreateCamera() = 0;

        virtual void DestroyCamera(Camera* camera) = 0;

        virtual Model* CreateModel(const ModelDesc& desc) = 0;

        virtual void DestroyModel(Model* model) = 0;

        virtual Mesh* CreateMesh(const MeshDesc& desc) = 0;

        virtual void DestroyMesh(Mesh* mesh) = 0;

        virtual SkinnedMesh* CreateSkinnedMesh(const SkinnedMeshDesc& desc) = 0;

        virtual void DestroySkinnedMesh(SkinnedMesh* skinned_mesh) = 0;

        virtual Material* CreateMaterial(const MaterialDesc& desc) = 0;

        virtual void DestroyMaterial(Material* material) = 0;

        virtual Light* CreateLight(const LightDesc& desc) = 0;

        virtual void DestroyLight(Light* light) = 0;

        virtual Skybox* CreateSkybox(const SkyboxDesc& desc) = 0;

        virtual void DestroySkybox(Skybox* skybox) = 0;

        virtual Terrain* CreateTerrain(const TerrainDesc& desc) = 0;

        virtual void DestroyTerrain(Terrain* terrain) = 0;

        virtual Ocean* CreateOcean(const OceanDesc& desc) = 0;

        virtual void DestroyOcean(Ocean* ocean) = 0;

        virtual RayCastResult RayCast(const Ray& ray, const BoundingBox& box) = 0;

        virtual Font* CreateFont(const FontDesc& desc) = 0;

        virtual void DestroyFont(Font* font) = 0;

        virtual Text* CreateText(const TextDesc& desc) = 0;

        virtual void DestroyText(Text* text) = 0;

        virtual Primitive* CreateCircle(const CircleDesc& desc) = 0;

        virtual Primitive* CreateRectangle(const RectngleDesc& desc) = 0;

        virtual Primitive* CreateLine(const LineDesc& desc) = 0;

        virtual Primitive* CreateCurve(const CurveDesc& desc) = 0;

        virtual Primitive* CreateBox(const BoxDesc& desc) = 0;

        virtual Primitive* CreateSphere(const SphereDesc& desc) = 0;

        virtual Primitive* CreateCylinder(const CylinderDesc& desc) = 0;

        virtual Primitive* CreateCone(const ConeDesc& desc) = 0;

        virtual Primitive* CreateTorus(const TorusDesc& desc) = 0;

        virtual Primitive* CreatePlane(const PlaneDesc& desc) = 0;

        virtual void DestroyPrimitive(Primitive* primitive) = 0;

        virtual class Gizmo* CreateGizmo(const GizmoDesc& desc) = 0;

        virtual void DestroyGizmo(Gizmo* gizmo) = 0;

        virtual void SetViewTransform(glm::mat4 transform) = 0;

        virtual void SetPerspectiveProjection(float fov, float ar, float nearPlane, float farPlane) = 0;

        virtual void SetOrthographicProjection(float left, float right, float bottom, float top, float nearPlane, float farPlane) = 0;

        virtual size_t EstimateMemoryUsage() = 0;
    };
} // namespace RE