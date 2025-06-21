/*
 * Basic camera class
 *
 * Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Examples-Base/Timestep.hpp"
#include "Examples-Base/Window.hpp"

/// @paragraph Coordinate-System The coordinate system used in the engine is a left-handed Y-up coordinate system.

namespace Constants
{
    const glm::vec3 PositiveX = glm::vec3(1.0f, 0.0f, 0.0f);
    const glm::vec3 PositiveY = glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::vec3 PositiveZ = glm::vec3(0.0f, 0.0f, 1.0f);
    const glm::vec3 Up        = PositiveY;
    const glm::vec3 Right     = PositiveX;
    const glm::vec3 Forward   = PositiveZ;
} // namespace Constants

class Camera
{
private:
    enum CameraControlType
    {
        LookAt,
        FirstPerson
    };

    enum class ProjectionMode
    {
        Perspective,
        Orthographic,
    };

    CameraControlType cameraType     = CameraControlType::FirstPerson;
    ProjectionMode    projectionMode = ProjectionMode::Perspective;

    glm::vec3 m_rotation = glm::vec3();
    glm::vec3 m_position = glm::vec3();
    glm::vec4 m_viewPos  = glm::vec4();

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
        bool left  = false;
        bool right = false;
        bool up    = false;
        bool down  = false;
    } m_keys;

    inline void UpdateViewMatrix()
    {
        glm::mat4 rotM = glm::mat4(1.0f);
        rotM           = glm::rotate(rotM, glm::radians(m_rotation.x), Constants::PositiveX);
        rotM           = glm::rotate(rotM, glm::radians(m_rotation.y), Constants::PositiveY);
        rotM           = glm::rotate(rotM, glm::radians(m_rotation.z), Constants::PositiveZ);

        glm::mat4 transM = glm::translate(glm::mat4(1.0f), m_position);

        if (cameraType == CameraControlType::FirstPerson)
        {
            m_matrices.worldToViewMat = rotM * transM;
        }
        else
        {
            m_matrices.worldToViewMat = transM * rotM;
        }

        m_viewPos = glm::vec4(-m_position.x, m_position.y, -m_position.z, 1.0f);
    }

public:
    inline glm::mat4 GetProjection() const
    {
        return m_matrices.viewToClipMat;
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

    inline glm::vec3 GetPosition() const
    {
        return m_position;
    }

    inline void SetPerspective(float width, float height, float fov, float znear, float zfar)
    {
        this->m_fov              = fov;
        this->m_znear            = znear;
        this->m_zfar             = zfar;
        auto aspect              = width / height;
        m_matrices.viewToClipMat = glm::perspectiveLH_ZO(glm::radians(fov), aspect, znear, zfar);
    }

    inline void SetOrthographic(float width, float height, float znear, float zfar)
    {
        this->m_fov              = 0;
        this->m_znear            = znear;
        this->m_zfar             = zfar;
        float halfWidth          = width / 2.0f;
        float halfHeight         = height / 2.0f;
        m_matrices.viewToClipMat = glm::orthoLH_ZO(-halfWidth, halfWidth, -halfHeight, halfHeight, znear, zfar);
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
        this->m_rotation.y -= delta.x * m_rotationSpeed;
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
        if (cameraType == CameraControlType::FirstPerson)
        {
            if (Moving())
            {
                glm::vec3 camFront;
                camFront.x = -cos(glm::radians(m_rotation.x)) * sin(glm::radians(m_rotation.y));
                camFront.y = sin(glm::radians(m_rotation.x));
                camFront.z = cos(glm::radians(m_rotation.x)) * cos(glm::radians(m_rotation.y));
                camFront   = glm::normalize(camFront);
                if (m_keys.up) m_position -= camFront * float(deltaTime.Miliseconds() * m_movementSpeed);
                if (m_keys.down) m_position += camFront * float(deltaTime.Miliseconds() * m_movementSpeed);
                if (m_keys.left) m_position -= glm::normalize(glm::cross(camFront, Constants::Up)) * float(deltaTime.Miliseconds() * m_movementSpeed);
                if (m_keys.right) m_position += glm::normalize(glm::cross(camFront, Constants::Up)) * float(deltaTime.Miliseconds() * m_movementSpeed);
            }
        }
        UpdateViewMatrix();
    }

    bool ProcessEvent(const Engine::WindowEvent& event)
    {
        switch (event.type)
        {
        case Engine::WindowEventType::Resized:
            {
                auto [width, height] = event.size;
                if (projectionMode == ProjectionMode::Perspective)
                {
                    SetPerspective(width, height, 60.0f, m_znear, m_zfar);
                }
                else if (projectionMode == ProjectionMode::Orthographic)
                {
                    SetOrthographic(width, height, m_znear, m_zfar);
                }
                break;
            }
        case Engine::WindowEventType::KeyInput:
            {
                // Example assumes WASD for movement
                bool pressed = (event.keyInput.state == Engine::KeyState::Press || event.keyInput.state == Engine::KeyState::Repeat);
                bool released = (event.keyInput.state == Engine::KeyState::Release);
                switch (event.keyInput.code)
                {
                case Engine::KeyCode::W: m_keys.up = pressed; m_keys.down = released ? false : m_keys.down; break;
                case Engine::KeyCode::S: m_keys.down = pressed; m_keys.up = released ? false : m_keys.up; break;
                case Engine::KeyCode::A: m_keys.left = pressed; m_keys.right = released ? false : m_keys.right; break;
                case Engine::KeyCode::D: m_keys.right = pressed; m_keys.left = released ? false : m_keys.left; break;
                default: break;
                }
                break;
            }
        case Engine::WindowEventType::CursorMoved:
            {
                // Window returns cursor relative to top-left
                if (event.window->GetMouseState(Engine::MouseCode::ButtonLeft, Engine::KeyState::Repeat) ||
                    event.window->GetMouseState(Engine::MouseCode::ButtonLeft, Engine::KeyState::Press))
                {
                    auto [x, y] = event.window->GetCursorDeltaPosition();
                    y *= -1;
                    Rotate({x, y});
                }

                break;
            }
        case Engine::WindowEventType::MouseScrolled:
            {
                SetMovementSpeed(m_movementSpeed + event.scrolled.x);
                break;
            }
        default: break;
        }

        return false;
    }
};
