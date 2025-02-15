#pragma once

#include <cstdint>
#include <glm/ext.hpp>
#include <glm/glm.hpp>


namespace Engine
{
    struct DrawIndexedArguments
    {
        uint32_t indexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        int32_t  vertexOffset;
        uint32_t firstInstance;
    };

    struct Transform
    {
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
    };

    struct DrawItem
    {
        void*     mesh     = nullptr;
        void*     material = nullptr;
        Transform transform;
    };

} // namespace Engine