/*
 * Basic camera class
 *
 * Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Examples-Base/Timestep.hpp"
#include "Examples-Base/Event.hpp"

using namespace Examples;

namespace Constants
{
    constexpr glm::vec3 Forward = { 0.0f, 0.0f, 1.0f };
    constexpr glm::vec3 PositiveZ = { 0.0f, 0.0f, 1.0f };

    constexpr glm::vec3 Right = { 1.0f, 0.0f, 0.0f };
    constexpr glm::vec3 PositiveX = { 1.0f, 0.0f, 0.0f };

    constexpr glm::vec3 Up = { 0.0f, 1.0f, 0.0f };
    constexpr glm::vec3 PositiveY = { 0.0f, 1.0f, 0.0f };
} // namespace Constants

class Camera
{
private:
    enum CameraType
    {
        LookAt,
        FirstPerson
    };

    enum class ProjectionMode
    {
        Perspective,
        Orthographic,
    };

    CameraType cameraType = CameraType::FirstPerson;
    ProjectionMode projectionMode = ProjectionMode::Perspective;

    glm::vec3 m_rotation = glm::vec3();
    glm::vec3 m_position = glm::vec3();
    glm::vec4 m_viewPos = glm::vec4();

    float m_rotationSpeed = 1.0f;
    float m_movementSpeed = 3.0f;

    float m_fov;
    float m_znear, m_zfar;

    struct
    {
        glm::mat4 viewToClipMat;
        glm::mat4 worldToViewMat;
    } m_matrices;

    struct
    {
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
    } m_keys;

    inline void UpdateViewMatrix()
    {
        glm::mat4 rotM = glm::mat4(1.0f);
        rotM = glm::rotate(rotM, glm::radians(m_rotation.x), Constants::PositiveX);
        rotM = glm::rotate(rotM, glm::radians(m_rotation.y), Constants::PositiveY);
        rotM = glm::rotate(rotM, glm::radians(m_rotation.z), Constants::PositiveZ);

        glm::mat4 transM = glm::translate(glm::mat4(1.0f), m_position);

        if (cameraType == CameraType::FirstPerson)
        {
            m_matrices.worldToViewMat = rotM * transM;
        }
        else
        {
            m_matrices.worldToViewMat = transM * rotM;
        }

        m_viewPos = glm::vec4(m_position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
    }

public:
    class Window* m_window;

    inline glm::mat4 GetProjection() const
    {
        glm::mat4 flipY = {
            {1.0f,  0.0f,  0.0f,  0.0f},
            { 0.0f, -1.0f, 0.0f,  0.0f},
            { 0.0f, 0.0f,  1.0f, 0.0f},
            { 0.0f, 0.0f,  0.0f,  1.0f},
        };

        return m_matrices.viewToClipMat * flipY;
    }

    inline glm::mat4 GetView() const
    {
        return m_matrices.worldToViewMat;
    }

    inline bool Moving()
    {
        return m_keys.left || m_keys.right || m_keys.up || m_keys.down;
    }

    inline float GetNearClip()
    {
        return m_znear;
    }

    inline float GetFarClip()
    {
        return m_zfar;
    }

    inline void SetPerspective(float fov, float aspect, float znear, float zfar)
    {
        this->m_fov = fov;
        this->m_znear = znear;
        this->m_zfar = zfar;
        m_matrices.viewToClipMat = glm::perspective(glm::radians(fov), aspect, znear, zfar);
    }

    inline void SetOrthographic(float left, float right, float bottom, float top, float znear, float zfar)
    {
        this->m_fov = 0;
        this->m_znear = znear;
        this->m_zfar = zfar;
        m_matrices.viewToClipMat = glm::ortho(left, right, bottom, top, znear, zfar);
    }

    inline void SetPosition(glm::vec3 position)
    {
        this->m_position = position;
        UpdateViewMatrix();
    }

    inline void SetRotation(glm::vec3 rotation)
    {
        this->m_rotation = rotation;
        UpdateViewMatrix();
    }

    inline void Rotate(glm::vec2 delta)
    {
        this->m_rotation.y += delta.x * m_rotationSpeed;
        this->m_rotation.x += delta.y * m_rotationSpeed;
        UpdateViewMatrix();
    }

    inline void SetTranslation(glm::vec3 translation)
    {
        this->m_position = translation;
        UpdateViewMatrix();
    }

    inline void Translate(glm::vec3 delta)
    {
        this->m_position += delta;
        UpdateViewMatrix();
    }

    inline void SetRotationSpeed(float rotationSpeed)
    {
        this->m_rotationSpeed = rotationSpeed;
    }

    inline void SetMovementSpeed(float movementSpeed)
    {
        this->m_movementSpeed = movementSpeed;
    }

    inline void Update(Timestep deltaTime)
    {
        if (cameraType == CameraType::FirstPerson)
        {
            if (Moving())
            {
                glm::vec3 camFront;

                // Left-handed, Y-up coordinate system with proper angle assignments
                camFront.x = -cos(glm::radians(m_rotation.x)) * sin(glm::radians(m_rotation.y));
                camFront.y = sin(glm::radians(m_rotation.x));
                camFront.z = cos(glm::radians(m_rotation.x)) * cos(glm::radians(m_rotation.y));
                camFront = glm::normalize(camFront);

                camFront = glm::normalize(camFront);

                float moveSpeed = float(deltaTime.Miliseconds() * m_movementSpeed);

                if (m_keys.up)
                    m_position -= camFront * moveSpeed;
                if (m_keys.down)
                    m_position += camFront * moveSpeed;
                if (m_keys.left)
                    m_position -= glm::normalize(glm::cross(camFront, Constants::Up)) * moveSpeed;
                if (m_keys.right)
                    m_position += glm::normalize(glm::cross(camFront, Constants::Up)) * moveSpeed;
            }
        }
        UpdateViewMatrix();
    }

    void ProcessEvent(class Event& event);
};

inline void Camera::ProcessEvent(Event& e)
{
    switch (e.GetEventType())
    {
    case EventType::WindowResize:
        {
            auto& event = (WindowResizeEvent&)e;
            if (projectionMode == ProjectionMode::Perspective)
            {
                SetPerspective(60.0f, float(event.GetSize().width) / float(event.GetSize().height), m_znear, m_zfar);
            }
            else if (projectionMode == ProjectionMode::Orthographic)
            {
                auto [width, height] = event.GetSize();
                auto left = -float(width) / 2.0f;
                auto right = float(width) / 2.0f;
                auto top = float(height) / 2.0f;
                auto bottom = -float(height) / 2.0f;
                SetOrthographic(left, right, bottom, top, m_znear, m_zfar);
            }
            else
            {
                TL_UNREACHABLE();
            }
            break;
        }
    case EventType::KeyPressed:
        {
            auto& event = (KeyPressedEvent&)e;
            switch (event.GetKeyCode())
            {
            case KeyCode::W: m_keys.up = true; break;
            case KeyCode::A: m_keys.left = true; break;
            case KeyCode::S: m_keys.down = true; break;
            case KeyCode::D: m_keys.right = true; break;
            default:         break;
            };
            break;
        }
    case EventType::KeyReleased:
        {
            auto& event = (KeyReleasedEvent&)e;
            switch (event.GetKeyCode())
            {
            case KeyCode::W: m_keys.up = false; break;
            case KeyCode::A: m_keys.left = false; break;
            case KeyCode::S: m_keys.down = false; break;
            case KeyCode::D: m_keys.right = false; break;
            default:         break;
            };
            break;
        }
    case EventType::MouseMoved:
        {
            [[maybe_unused]] auto& event = (MouseMovedEvent&)e;

            // Window returns cursor relative to top-left
            if (m_window->IsMouseButtonPressed(MouseCode::ButtonLeft))
            {
                auto [x, y] = m_window->GetCursrorDeltaPosition();
                y *= -1;
                Rotate({ x, y });
            }

            break;
        }
    case EventType::MouseScrolled:
        {
            auto& event = (MouseScrolledEvent&)e;
            SetMovementSpeed(m_movementSpeed + event.GetXOffset());
            break;
        }
    default: break;
    }
}