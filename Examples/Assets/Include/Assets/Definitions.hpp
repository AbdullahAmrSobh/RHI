#pragma once

#include <cstdint>
#include <type_traits>

#include <glm/glm.hpp>

namespace Examples::Assets
{
    enum class VertexFormat
    {
        None,

        F32,
        F32Vec2,
        F32Vec3,
        F32Vec4,

        U8,
        U8Vec2,
        U8Vec3,
        U8Vec4,

        U16,
        U16Vec2,
        U16Vec3,
        U16Vec4,

        U32,
        U32Vec2,
        U32Vec3,
        U32Vec4,

        U64,
        U64Vec2,
        U64Vec3,
        U64Vec4,

        I8,
        I8Vec2,
        I8Vec3,
        I8Vec4,

        I16,
        I16Vec2,
        I16Vec3,
        I16Vec4,

        I32,
        I32Vec2,
        I32Vec3,
        I32Vec4,

        I64,
        I64Vec2,
        I64Vec3,
        I64Vec4,

        Custom,
    };

    enum class ImageFormat : uint8_t
    {
        Unknown,

        R8 , RG8,  RGB8,  RGBA8,
        R16, RG16, RGB16, RGBA16,
        R32, RG32, RGB32, RGBA32,

        BC1,
        BC2,
        BC3,
        BC4,
        BC4_Signed,
        BC5,
        BC5_Signed,
        BC6H,
        BC6H_SF,
        BC7,
        COUNT,
    };

    namespace AttributeNames
    {
        // clang-format off
        static constexpr const char* Indcies               = "Indcies";
        static constexpr const char* Normals               = "Normals";
        static constexpr const char* TangentsAndBitangents = "TangentsAndBitangents";
        static constexpr const char* VertexColors          = "VertexColors";
        static constexpr const char* TextureCoords         = "TextureCoords";
        static constexpr const char* Positions             = "Positions";
        static constexpr const char* Faces                 = "Faces";
        static constexpr const char* Bones                 = "Bones";
        static constexpr const char* TextureCoordsName     = "TextureCoordsName";
        // clang-format on
    }; // namespace AttributeNames

    template<typename VertexType>
    constexpr VertexFormat GetVertexFormat()
    {
        if constexpr (std::is_same_v<VertexType, float>)
            return VertexFormat::F32;
        else if constexpr (std::is_same_v<VertexType, glm::vec2>)
            return VertexFormat::F32Vec2;
        else if constexpr (std::is_same_v<VertexType, glm::vec3>)
            return VertexFormat::F32Vec3;
        else if constexpr (std::is_same_v<VertexType, glm::vec4>)
            return VertexFormat::F32Vec4;

        else if constexpr (std::is_same_v<VertexType, uint8_t>)
            return VertexFormat::U8;
        else if constexpr (std::is_same_v<VertexType, glm::u8vec2>)
            return VertexFormat::U8Vec2;
        else if constexpr (std::is_same_v<VertexType, glm::u8vec3>)
            return VertexFormat::U8Vec3;
        else if constexpr (std::is_same_v<VertexType, glm::u8vec4>)
            return VertexFormat::U8Vec4;

        else if constexpr (std::is_same_v<VertexType, uint16_t>)
            return VertexFormat::U16;
        else if constexpr (std::is_same_v<VertexType, glm::u16vec2>)
            return VertexFormat::U16Vec2;
        else if constexpr (std::is_same_v<VertexType, glm::u16vec3>)
            return VertexFormat::U16Vec3;
        else if constexpr (std::is_same_v<VertexType, glm::u16vec4>)
            return VertexFormat::U16Vec4;

        else if constexpr (std::is_same_v<VertexType, uint32_t>)
            return VertexFormat::U32;
        else if constexpr (std::is_same_v<VertexType, glm::u32vec2>)
            return VertexFormat::U32Vec2;
        else if constexpr (std::is_same_v<VertexType, glm::u32vec3>)
            return VertexFormat::U32Vec3;
        else if constexpr (std::is_same_v<VertexType, glm::u32vec4>)
            return VertexFormat::U32Vec4;

        else if constexpr (std::is_same_v<VertexType, uint64_t>)
            return VertexFormat::U64;
        else if constexpr (std::is_same_v<VertexType, glm::u64vec2>)
            return VertexFormat::U64Vec2;
        else if constexpr (std::is_same_v<VertexType, glm::u64vec3>)
            return VertexFormat::U64Vec3;
        else if constexpr (std::is_same_v<VertexType, glm::u64vec4>)
            return VertexFormat::U64Vec4;

        else if constexpr (std::is_same_v<VertexType, int8_t>)
            return VertexFormat::I8;
        else if constexpr (std::is_same_v<VertexType, glm::i8vec2>)
            return VertexFormat::I8Vec2;
        else if constexpr (std::is_same_v<VertexType, glm::i8vec3>)
            return VertexFormat::I8Vec3;
        else if constexpr (std::is_same_v<VertexType, glm::i8vec4>)
            return VertexFormat::I8Vec4;

        else if constexpr (std::is_same_v<VertexType, int16_t>)
            return VertexFormat::I16;
        else if constexpr (std::is_same_v<VertexType, glm::i16vec2>)
            return VertexFormat::I16Vec2;
        else if constexpr (std::is_same_v<VertexType, glm::i16vec3>)
            return VertexFormat::I16Vec3;
        else if constexpr (std::is_same_v<VertexType, glm::i16vec4>)
            return VertexFormat::I16Vec4;

        else if constexpr (std::is_same_v<VertexType, int32_t>)
            return VertexFormat::I32;
        else if constexpr (std::is_same_v<VertexType, glm::i32vec2>)
            return VertexFormat::I32Vec2;
        else if constexpr (std::is_same_v<VertexType, glm::i32vec3>)
            return VertexFormat::I32Vec3;
        else if constexpr (std::is_same_v<VertexType, glm::i32vec4>)
            return VertexFormat::I32Vec4;

        else if constexpr (std::is_same_v<VertexType, int64_t>)
            return VertexFormat::I64;
        else if constexpr (std::is_same_v<VertexType, glm::i64vec2>)
            return VertexFormat::I64Vec2;
        else if constexpr (std::is_same_v<VertexType, glm::i64vec3>)
            return VertexFormat::I64Vec3;
        else if constexpr (std::is_same_v<VertexType, glm::i64vec4>)
            return VertexFormat::I64Vec4;

        else
            return VertexFormat::Custom;
    }
} // namespace Examples::Assets