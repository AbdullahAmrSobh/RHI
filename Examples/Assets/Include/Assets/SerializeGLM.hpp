#pragma once

#include <TL/Serialization/Binary.hpp>

#include <glm/glm.hpp>

namespace TL
{
    inline static void Encode(BinaryArchive& archive, const glm::vec2& value)
    {
        TL::Encode(archive, value.x);
        TL::Encode(archive, value.y);
    }

    inline static void Decode(BinaryArchive& archive, glm::vec2& value)
    {
        TL::Decode(archive, value.x);
        TL::Decode(archive, value.y);
    }

    inline static void Encode(BinaryArchive& archive, const glm::vec3& value)
    {
        TL::Encode(archive, value.x);
        TL::Encode(archive, value.y);
        TL::Encode(archive, value.z);
    }

    inline static void Decode(BinaryArchive& archive, glm::vec3& value)
    {
        TL::Decode(archive, value.x);
        TL::Decode(archive, value.y);
        TL::Decode(archive, value.z);
    }

    inline static void Encode(BinaryArchive& archive, const glm::vec4& value)
    {
        TL::Encode(archive, value.x);
        TL::Encode(archive, value.y);
        TL::Encode(archive, value.z);
        TL::Encode(archive, value.w);
    }

    inline static void Encode(BinaryArchive& archive, glm::vec4& value)
    {
        TL::Encode(archive, value.x);
        TL::Encode(archive, value.y);
        TL::Encode(archive, value.z);
        TL::Encode(archive, value.w);
    }

    inline static void Encode(BinaryArchive& archive, const glm::mat4& value)
    {
        TL::Encode(archive, TL::Block::Create(value));
    }

    inline static void Decode(BinaryArchive& archive, glm::mat4& value)
    {
        TL::Decode(archive, Block{ &value[0][0], 16 });
    }
}; // namespace TL
