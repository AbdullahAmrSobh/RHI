#pragma once

#include <Examples-Base/ApplicationBase.hpp>
#include <Examples-Base/Common.hpp>

#include <RHI/RHI.hpp>

#include <glm/glm.hpp>
#include "glm/ext.hpp"

#include "ShaderInterface/Core.slang"

#include <TL/Containers.hpp>

namespace Examples
{
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
        TL::Vector<Handle<RHI::Image>> images;
        TL::Vector<Handle<RHI::ImageView>> imagesViews;

        TL::Vector<ObjectTransform> m_meshesTransform;
        TL::Vector<uint32_t> m_meshesStatic;
    };
} // namespace Examples