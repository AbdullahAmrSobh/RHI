#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>
#include <TL/Span.hpp>

#include <glm/glm.hpp>

namespace Engine
{
    enum class SceneViewUsageFlags
    {
        None              = 0,
        Camera            = 1 << 0,
        Shadow            = 1 << 1,
        ReflectiveCubeMap = 1 << 2,
    };

    class SceneView
    {
    public:
    private:
        TL::String  m_name;
        glm::mat4x4 m_worldToViewMatrix;
        glm::mat4x4 m_viewToWorldMatrix;
        glm::mat4x4 m_viewToClipMatrix;
        glm::vec3   m_position;
        glm::vec2   m_clipSpaceOffset;
    };

    class LightComponent {};
    class ModelComponent {};

} // namespace Engine