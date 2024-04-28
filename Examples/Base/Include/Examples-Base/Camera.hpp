/*
 * Basic camera class
 *
 * Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Examples-Base/Timestep.hpp"

class Camera
{
private:
    friend class ApplicationBase;

    float fov;
    float znear, zfar;

    inline void UpdateViewMatrix()
    {
        glm::mat4 rotM = glm::mat4(1.0f);
        glm::mat4 transM;

        rotM = glm::rotate(rotM, glm::radians(rotation.x * (flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::vec3 translation = position;
        if (flipY)
        {
            translation.y *= -1.0f;
        }
        transM = glm::translate(glm::mat4(1.0f), translation);

        if (type == CameraType::FirstPerson)
        {
            matrices.view = rotM * transM;
        }
        else
        {
            matrices.view = transM * rotM;
        }

        viewPos = glm::vec4(position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
    }

    enum CameraType
    {
        LookAt,
        FirstPerson
    };

    CameraType type = CameraType::FirstPerson;

    glm::vec3 rotation = glm::vec3();
    glm::vec3 position = glm::vec3();
    glm::vec4 viewPos = glm::vec4();

    float rotationSpeed = 1.0f;
    float movementSpeed = 1.0f;

    bool flipY = false;

    struct
    {
        glm::mat4 perspective;
        glm::mat4 view;
    } matrices;

    struct
    {
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
    } keys;

public:
    inline glm::mat4 GetProjection() const
    {
        return matrices.perspective;
    }

    inline glm::mat4 GetView() const
    {
        return matrices.view;
    }

    inline bool Moving()
    {
        return keys.left || keys.right || keys.up || keys.down;
    }

    inline float GetNearClip()
    {
        return znear;
    }

    inline float GetFarClip()
    {
        return zfar;
    }

    inline void SetPerspective(float fov, float aspect, float znear, float zfar)
    {
        this->fov = fov;
        this->znear = znear;
        this->zfar = zfar;
        matrices.perspective = glm::perspectiveRH(glm::radians(fov), aspect, znear, zfar);
        if (flipY)
        {
            matrices.perspective[1][1] *= -1.0f;
        }
    }

    inline void UpdateAspectRatio(float aspect)
    {
        matrices.perspective = glm::perspectiveRH(glm::radians(fov), aspect, znear, zfar);
        if (flipY)
        {
            matrices.perspective[1][1] *= -1.0f;
        }
    }

    inline void SetPosition(glm::vec3 position)
    {
        this->position = position;
        UpdateViewMatrix();
    }

    inline void SetRotation(glm::vec3 rotation)
    {
        this->rotation = rotation;
        UpdateViewMatrix();
    }

    inline void Rotate(glm::vec3 delta)
    {
        this->rotation += delta;
        UpdateViewMatrix();
    }

    inline void SetTranslation(glm::vec3 translation)
    {
        this->position = translation;
        UpdateViewMatrix();
    }

    inline void Translate(glm::vec3 delta)
    {
        this->position += delta;
        UpdateViewMatrix();
    }

    inline void SetRotationSpeed(float rotationSpeed)
    {
        this->rotationSpeed = rotationSpeed;
    }

    inline void SetMovementSpeed(float movementSpeed)
    {
        this->movementSpeed = movementSpeed;
    }

    inline void Update(Timestep deltaTime)
    {
        if (type == CameraType::FirstPerson)
        {
            if (Moving())
            {
                glm::vec3 camFront;
                camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
                camFront.y = sin(glm::radians(rotation.x));
                camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
                camFront = glm::normalize(camFront);

                float moveSpeed = deltaTime.Miliseconds() * movementSpeed;

                if (keys.up)
                    position += camFront * moveSpeed;
                if (keys.down)
                    position -= camFront * moveSpeed;
                if (keys.left)
                    position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
                if (keys.right)
                    position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
            }
        }
        UpdateViewMatrix();
    }
};