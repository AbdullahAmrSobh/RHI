#pragma once

#include "Assets/Export.hpp"
#include "Assets/SerializeGLM.hpp"

#include <TL/Containers.hpp>
#include <TL/UniquePtr.hpp>
#include <TL/Serialization/Binary.hpp>

#include <glm/glm.hpp>

#include <queue>
#include <functional>

namespace Examples::Assets
{
    // /// @brief Represents a node in the scene graph that references multiple assets.
    // /// A SceneNode can reference an array of asset names (meshes, materials, etc.), without owning the actual data.
    // class ASSETS_EXPORT SceneNode
    // {
    // public:
    //     SceneNode() = default;

    //     SceneNode(const char* name)
    //         : m_name(name)
    //     {
    //     }

    //     /// @brief Adds a child node to this node.
    //     /// @param child The child node to be added.
    //     void AddChild(SceneNode* child)
    //     {
    //         m_children.push_back(child);
    //     }

    //     /// @brief Retrieves the name of the node.
    //     /// @return The name of the node.
    //     const char* GetName() const { return m_name.c_str(); }

    //     /// @brief Adds a reference to a mesh by name.
    //     void AddMesh(const char* meshName) { m_meshes.push_back(meshName); }

    //     /// @brief Adds a reference to a material by name.
    //     void AddMaterial(const char* materialName) { m_materials.push_back(materialName); }

    //     /// @brief Gets the list of mesh names.
    //     const TL::Vector<TL::String>& GetMeshes() const { return m_meshes; }

    //     /// @brief Gets the list of material names.
    //     const TL::Vector<TL::String>& GetMaterials() const { return m_materials; }

    //     /// @brief Gets the node local transform matrix.
    //     const glm::mat4 GetTransform() const { return m_transform; }

    //     /// @brief Sets the node local transform matrix.
    //     void SetTransform(glm::mat4 transform) { m_transform = transform; }

    //     /// @brief Serializes the node and its children.
    //     template<typename Archive>
    //     void Serialize(Archive& archive) const
    //     {
    //         TL::Encode(archive, m_name);
    //         TL::Encode(archive, m_transform);
    //         TL::Encode(archive, m_meshes);
    //         TL::Encode(archive, m_materials);
    //         TL::Encode(archive, m_children);
    //     }

    //     /// @brief Deserializes the node and its children.
    //     template<typename Archive>
    //     void Deserialize(Archive& archive)
    //     {
    //         TL::Decode(archive, m_name);
    //         TL::Decode(archive, m_transform);
    //         TL::Decode(archive, m_meshes);
    //         TL::Decode(archive, m_materials);
    //         TL::Decode(archive, m_children);
    //     }

    // private:
    //     TL::String m_name; ///< The name of the node.
    //     glm::mat4 m_transform;
    //     TL::Vector<TL::String> m_meshes;    ///< Array of mesh references by name.
    //     TL::Vector<TL::String> m_materials; ///< Array of material references by name.
    //     TL::Vector<SceneNode*> m_children;  ///< Child nodes.
    // };

    // /// @brief Represents the scene graph itself, managing a collection of scene nodes.
    // class ASSETS_EXPORT SceneGraph
    // {
    // public:
    //     SceneGraph() = default;

    //     /// @brief Applies a function to this node and its descendants using breadth-first traversal, only if the node has meshes.
    //     /// @param func The function to apply to each node with meshes.
    //     void Traverse(const std::function<void(SceneNode*)>& func)
    //     {
    //         for (auto& node : m_nodes)
    //         {
    //             if (node.GetMeshes().empty() == false)
    //             {
    //                 func(&node);
    //             }
    //         }
    //     }

    //     /// @brief Adds a node to the scene graph.
    //     /// @param node The node to be added.
    //     void AddNode(SceneNode* node)
    //     {
    //         m_nodes.push_back(*node);
    //     }

    //     /// @brief Serializes the entire scene graph.
    //     template<typename Archive>
    //     void Serialize(Archive& archive) const
    //     {
    //         TL::Encode(archive, m_nodes);
    //     }

    //     /// @brief Deserializes the entire scene graph.
    //     template<typename Archive>
    //     void Deserialize(Archive& archive)
    //     {
    //         TL::Decode(archive, m_nodes);
    //     }

    // private:
    //     TL::Vector<SceneNode> m_nodes; ///< List of scene nodes.
    // };

} // namespace Examples::Assets
