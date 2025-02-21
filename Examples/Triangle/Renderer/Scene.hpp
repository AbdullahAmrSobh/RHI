#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>
#include <TL/Span.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Shaders/Public/GpuScene.h"

// namespace Engine
// {
//     class MeshResource;
//     class Material;

//     static constexpr uint32_t DrawArgumentBufferStrideSize = sizeof(RHI::DrawIndexedParameters);

//     struct Transform
//     {
//         glm::vec3 position = {0.0f, 0.0f, 0.0f};
//         glm::quat rotation = {0.0f, 0.0f, 0.0f, 1.0f};
//         glm::vec3 scale    = {1.0f, 1.0f, 1.0f};

//         inline glm::mat3x4 GetWorldMatrix() const
//         {
//             glm::mat4 modelToWorld = glm::translate(glm::identity<glm::mat4>(), position);
//             modelToWorld *= rotation;
//             modelToWorld *= glm::scale(modelToWorld, scale);
//             return modelToWorld;
//         }
//     };

//     class ModelComponent final
//     {
//     public:
//     private:
//         MeshResource* m_mesh      = nullptr;
//         Material*     m_material  = nullptr;
//         Transform     m_transform = {};
//     };

//     class Scene
//     {
//         friend class Renderer;

//     public:
//         uint32_t GetDrawCount() const;

//         ModelComponent* AddModel(MeshResource* mesh, Transform transform);

//         void RemoveModel(ModelComponent* model);

//     private:
//     private:
//         // Scene Components
//         TL::Vector<glm::mat3x4>                m_modelToWorldMatsData;
//         TL::Vector<Shader::ObjectID>           m_modelDrawIdsData;
//         TL::Vector<Shader::Light>              m_lightsData;
//         TL::Vector<Shader::Scene>              m_viewsData;
//         TL::Vector<RHI::DrawIndexedParameters> m_drawParameters;

//     private:
//         RHI::Device* m_device;
//     };
// } // namespace Engine