#pragma once

#include <Examples-Base/ApplicationBase.hpp>

#include <RHI/RHI.hpp>

#include <glm/glm.hpp>
#include "glm/ext.hpp"

#include "ShaderInterface/Core.slang"

namespace Examples
{
    struct MaterialIds
    {
        uint32_t diffuseID = UINT32_MAX;
        uint32_t normalID = UINT32_MAX;
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

        TL2::Vector<Mesh*> m_meshes;

        TL2::Vector<MaterialIds> materialIDs;
        TL2::Vector<Handle<RHI::Image>> images;
        TL2::Vector<Handle<RHI::ImageView>> imagesViews;

        TL2::Vector<ObjectTransform> m_meshesTransform;
        TL2::Vector<uint32_t> m_meshesStatic;
    };
} // namespace Examples