#pragma once

#include <RPI/Export.hpp>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <TL/Containers.hpp>
#include <TL/UniquePtr.hpp>

namespace Examples::RPI
{
    class RPI_EXPORT View
    {
    public:
        View();
        ~View();

        glm::mat4 GetWorldToViewMatrix() const { return m_worldToViewMatrix; }

        glm::mat3x4 GetWorldToViewMatrix3x4() const { return m_worldToViewMatrix; }

        glm::mat4 GetViewToWorldMatrix() const { return m_viewToWorldMatrix; }

        glm::mat3x4 GetViewToWorldMatrix3x4() const { return m_viewToWorldMatrix; }

        glm::mat4 GetViewToClipMatrix() const { return m_viewToClipMatrix; }

        glm::mat4 GetWorldToClipMatrix() const { return m_worldToClipMatrix; }

        glm::mat4 GetClipToWorldMatrix() const { return m_clipToWorldMatrix; }

        void SetCameraTransform(glm::vec3 position, glm::vec3 direction, glm::vec3 up);

        void SetWorldToViewMatrix(glm::mat4 worldToView);

        void SetProjectionPerspective(float ar, float fov, float near, float far);

        void SetProjectionOrthographic(float left, float right, float top, float bottom, float near, float far);

    private:
        TL::String m_name;

        glm::mat4 m_worldToViewMatrix;
        glm::mat4 m_viewToWorldMatrix;
        glm::mat4 m_viewToClipMatrix;
        glm::mat4 m_worldToClipMatrix;
        glm::mat4 m_clipToWorldMatrix;
    };
} // namespace Examples::RPI