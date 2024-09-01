#pragma once

#include <TL/Containers.hpp>

#include <glm/glm.hpp>

namespace TL
{
    inline static TL::String ToString(const glm::vec2& value)
    {
        return TL::String(std::format("({}, {})", value.x, value.y));
    }

    inline static TL::String ToString(const glm::vec3& value)
    {
        return TL::String(std::format("({}, {}, {})", value.x, value.y, value.z));
    }

    inline static TL::String ToString(const glm::vec4& value)
    {
        return TL::String(std::format("({}, {}, {}, {})", value.x, value.y, value.z, value.w));
    }
}; // namespace TL
