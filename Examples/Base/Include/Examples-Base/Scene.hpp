#pragma once

#include <Examples-Base/ApplicationBase.hpp>

#include <RHI/RHI.hpp>

#include <glm/glm.hpp>
#include "glm/ext.hpp"

namespace Examples
{
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

        TL::Vector<glm::mat4> m_meshesTransform;
        TL::Vector<uint32_t> m_meshesStatic;
    };
} // namespace Examples