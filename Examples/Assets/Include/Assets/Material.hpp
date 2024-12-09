#pragma once

#include "Assets/Export.hpp"
#include "Assets/Name.hpp"
#include "Assets/SerializeGLM.hpp"

#include <TL/Flags.hpp>
#include <TL/Block.hpp>
#include <TL/UniquePtr.hpp>
#include <TL/Containers.hpp>
#include <TL/Serialization/Binary.hpp>
#include <TL/Span.hpp>

#include <glm/glm.hpp>

#include <filesystem>
#include <optional>

namespace Examples::Assets
{
    // struct MaterialProperty
    // {
    //     enum class PropertyType
    //     {
    //         None,
    //         uint8_t,
    //         uint16_t,
    //         uint32_t,
    //         uint64_t,
    //         int8_t,
    //         int16_t,
    //         int32_t,
    //         int64_t,
    //         f32,
    //         f64,
    //         vec2,
    //         vec3,
    //         vec4,
    //     };

    //     union ValueType
    //     {
    //         uint8_t as_uint8;
    //         uint16_t as_uint16;
    //         uint32_t as_uint32;
    //         uint64_t as_uint64;
    //         int8_t as_int8;
    //         int16_t as_int16;
    //         int32_t as_int32;
    //         int64_t as_int64;
    //         float as_f32;
    //         double as_f64;
    //         glm::vec2 as_vec2;
    //         glm::vec3 as_vec3;
    //         glm::vec4 as_vec4;

    //         ValueType()
    //         {
    //             this->as_vec4 = {};
    //         }
    //     };

    //     PropertyType type;
    //     ValueType value;

    //     MaterialProperty()
    //         : type(PropertyType::None)
    //     {
    //     }

    //     // clang-format off
    //     MaterialProperty(uint8_t value) : type(PropertyType::uint8_t) { this->value.as_uint8 = value; }
    //     MaterialProperty(uint16_t value) : type(PropertyType::uint16_t) { this->value.as_uint16 = value; }
    //     MaterialProperty(uint32_t value) : type(PropertyType::uint32_t) { this->value.as_uint32 = value; }
    //     MaterialProperty(uint64_t value) : type(PropertyType::uint64_t) { this->value.as_uint64 = value; }

    //     MaterialProperty(int8_t value) : type(PropertyType::int8_t) { this->value.as_int8 = value; }
    //     MaterialProperty(int16_t value) : type(PropertyType::int16_t) { this->value.as_int16 = value; }
    //     MaterialProperty(int32_t value) : type(PropertyType::int32_t) { this->value.as_int32 = value; }
    //     MaterialProperty(int64_t value) : type(PropertyType::int64_t) { this->value.as_int64 = value; }

    //     MaterialProperty(float value) : type(PropertyType::f32) { this->value.as_f32 = value; }
    //     MaterialProperty(double value) : type(PropertyType::f64) { this->value.as_f64 = value; }

    //     MaterialProperty(glm::vec2 value) : type(PropertyType::vec2) { this->value.as_vec2 = value; }
    //     MaterialProperty(glm::vec3 value) : type(PropertyType::vec3) { this->value.as_vec3 = value; }
    //     MaterialProperty(glm::vec4 value) : type(PropertyType::vec4) { this->value.as_vec4 = value; }

    //     // clang-format on

    //     template<typename Archive>
    //     void Serialize(Archive& archive) const
    //     {
    //         TL::Encode(archive, type);
    //         TL::Encode(archive, value);
    //     }

    //     template<typename Archive>
    //     void Deserialize(Archive& archive)
    //     {
    //         TL::Decode(archive, type);
    //         TL::Decode(archive, value);
    //     }
    // };

    class ASSETS_EXPORT Material
    {
    public:
        Material() = default;

        // Material(const char* name, TL::Span<const MaterialProperty> properties, TL::Span<Name> textures)
        //     : m_name(name)
        // {
        // }
        Material(const char* name)
            : m_name(name)
        {
        }

        Material(Material&& other) = default;
        Material(const Material& other) = delete;
        ~Material() = default;

        Material& operator=(Material&& other) = default;
        Material& operator=(const Material& other) = delete;

        std::optional<glm::vec3> colorDiffuse;     // Diffuse color of the material
        std::optional<glm::vec3> colorSpecular;    // Specular color of the material
        std::optional<glm::vec3> colorAmbient;     // Ambient color of the material
        std::optional<glm::vec3> colorEmissive;    // Emissive color of the material
        std::optional<glm::vec3> colorTransparent; // Transparent color of the material
        std::optional<glm::vec3> colorReflective;  // Reflective color of the material
        std::optional<float> reflectivity;         // Scales the reflective color
        std::optional<int> wireframe;              // Wireframe rendering flag (0: false, !0: true)
        std::optional<int> twosided;               // Backface culling flag (0: false, !0: true)
        std::optional<int> shadingModel;           // Shading model (gouraud, phong, etc.)
        std::optional<int> blendFunc;              // Alpha blending mode
        std::optional<float> opacity;              // Opacity of the material (0..1)
        std::optional<float> shininess;            // Shininess of the material (Phong exponent)
        std::optional<float> shininessStrength;    // Scales the specular color
        std::optional<float> refracti;             // Index of Refraction

        /// @todo based on assimp
        /// TEXTURE(t,n) and related fields would likely be part of an array or vector of texture structs
        /// aiString TEXTURE[t][n];   // Path to the n’th texture on stack ‘t’
        /// float TEXBLEND[t][n];     // Strength of the n’th texture on stack ‘t’
        /// int TEXOP[t][n];          // Arithmetic operation for the n’th texture on stack ‘t’
        /// int MAPPING[t][n];        // Mapping coordinates for the n’th texture on stack ‘t’
        /// int UVWSRC[t][n];         // UV channel for sampling the n’th texture on stack ‘t’
        /// int MAPPINGMODE_U[t][n];  // Texture wrapping mode on the x axis for n’th texture on stack ‘t’
        /// int MAPPINGMODE_V[t][n];  // Texture wrapping mode on the v axis for n’th texture on stack ‘t’

        std::filesystem::path specularMap;
        std::filesystem::path diffuseMap;
        std::filesystem::path ambientMap;
        std::filesystem::path emissiveMap;
        std::filesystem::path heightMap;
        std::filesystem::path normalsMap;
        std::filesystem::path shininessMap;
        std::filesystem::path opacityMap;
        std::filesystem::path displacementMap;
        std::filesystem::path lightmapMap;
        std::filesystem::path reflectionMap;
        std::filesystem::path baseColorMap;
        std::filesystem::path normalCameraMap;
        std::filesystem::path emissionColorMap;
        std::filesystem::path metalnessMap;
        std::filesystem::path diffuseRoughnessMap;
        std::filesystem::path ambientOcclusionMap;
        std::filesystem::path sheenMap;
        std::filesystem::path clearcoatMap;
        std::filesystem::path transmissionMap;
        std::filesystem::path unknownMap;

        template<typename T>
        void AddProperty(const char* name, T value);

        void AddTexture(const char* name, Name textureName);

        template<typename T>
        T GetProperty(const char* name) const;

        Name GetTexture(const char* name) const;

        inline const TL::String& GetName() const { return m_name; }

        inline const TL::Map<TL::String, Name>& GetTextures() const { return m_textures; }

        // inline const TL::Map<TL::String, MaterialProperty>& GetProperties() const { return m_properties; }

        template<typename Archive>
        void Serialize(Archive& archive) const
        {
            TL::Encode(archive, m_name);
            TL::Encode(archive, m_textures);
            // TL::Encode(archive, m_properties);

            TL::Encode(archive, colorDiffuse);
            TL::Encode(archive, colorSpecular);
            TL::Encode(archive, colorAmbient);
            TL::Encode(archive, colorEmissive);
            TL::Encode(archive, colorTransparent);
            TL::Encode(archive, colorReflective);
            TL::Encode(archive, reflectivity);
            TL::Encode(archive, wireframe);
            TL::Encode(archive, twosided);
            TL::Encode(archive, shadingModel);
            TL::Encode(archive, blendFunc);
            TL::Encode(archive, opacity);
            TL::Encode(archive, shininess);
            TL::Encode(archive, shininessStrength);
            TL::Encode(archive, refracti);
            TL::Encode(archive, specularMap);
            TL::Encode(archive, diffuseMap);
            TL::Encode(archive, ambientMap);
            TL::Encode(archive, emissiveMap);
            TL::Encode(archive, heightMap);
            TL::Encode(archive, normalsMap);
            TL::Encode(archive, shininessMap);
            TL::Encode(archive, opacityMap);
            TL::Encode(archive, displacementMap);
            TL::Encode(archive, lightmapMap);
            TL::Encode(archive, reflectionMap);
            TL::Encode(archive, baseColorMap);
            TL::Encode(archive, normalCameraMap);
            TL::Encode(archive, emissionColorMap);
            TL::Encode(archive, metalnessMap);
            TL::Encode(archive, diffuseRoughnessMap);
            TL::Encode(archive, ambientOcclusionMap);
            TL::Encode(archive, sheenMap);
            TL::Encode(archive, clearcoatMap);
            TL::Encode(archive, transmissionMap);
            TL::Encode(archive, unknownMap);
        }

        template<typename Archive>
        void Deserialize(Archive& archive)
        {
            TL::Decode(archive, m_name);
            TL::Decode(archive, m_textures);
            // TL::Decode(archive, m_properties);

            TL::Decode(archive, colorDiffuse);
            TL::Decode(archive, colorSpecular);
            TL::Decode(archive, colorAmbient);
            TL::Decode(archive, colorEmissive);
            TL::Decode(archive, colorTransparent);
            TL::Decode(archive, colorReflective);
            TL::Decode(archive, reflectivity);
            TL::Decode(archive, wireframe);
            TL::Decode(archive, twosided);
            TL::Decode(archive, shadingModel);
            TL::Decode(archive, blendFunc);
            TL::Decode(archive, opacity);
            TL::Decode(archive, shininess);
            TL::Decode(archive, shininessStrength);
            TL::Decode(archive, refracti);
            TL::Decode(archive, specularMap);
            TL::Decode(archive, diffuseMap);
            TL::Decode(archive, ambientMap);
            TL::Decode(archive, emissiveMap);
            TL::Decode(archive, heightMap);
            TL::Decode(archive, normalsMap);
            TL::Decode(archive, shininessMap);
            TL::Decode(archive, opacityMap);
            TL::Decode(archive, displacementMap);
            TL::Decode(archive, lightmapMap);
            TL::Decode(archive, reflectionMap);
            TL::Decode(archive, baseColorMap);
            TL::Decode(archive, normalCameraMap);
            TL::Decode(archive, emissionColorMap);
            TL::Decode(archive, metalnessMap);
            TL::Decode(archive, diffuseRoughnessMap);
            TL::Decode(archive, ambientOcclusionMap);
            TL::Decode(archive, sheenMap);
            TL::Decode(archive, clearcoatMap);
            TL::Decode(archive, transmissionMap);
            TL::Decode(archive, unknownMap);
        }

    private:
        TL::String m_name;
        TL::Map<TL::String, Name> m_textures;
        // TL::Map<TL::String, MaterialProperty> m_properties;
    };
} // namespace Examples::Assets
