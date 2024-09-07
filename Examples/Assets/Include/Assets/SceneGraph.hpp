#pragma once

#include "Assets/Export.hpp"
#include "Assets/Name.hpp"
#include "Assets/SerializeGLM.hpp"

#include <TL/Containers.hpp>
#include <TL/UniquePtr.hpp>
#include <TL/Serialization/Binary.hpp>

#include <glm/glm.hpp>

namespace Examples::Assets
{
    class Mesh;

    struct Light
    {
        enum class Type
        {
            Directional,
            Point,
            Spot,
        };

        Type type;
        glm::vec3 color;
        glm::vec3 direction; // Used for directional and spotlights
        glm::vec3 position;  // Used for point and spotlights
        float intensity;
        float range;     // Used for point and spotlights
        float spotAngle; // Used for spotlights

        template<typename Archive>
        void Serialize(Archive& archive) const
        {
            TL::Encode(archive, (int)type);
            TL::Encode(archive, color);
            TL::Encode(archive, direction);
            TL::Encode(archive, position);
        }

        template<typename Archive>
        void Deserialize(Archive& archive)
        {
            TL::Decode(archive, (int&)type);
            TL::Decode(archive, color);
            TL::Decode(archive, direction);
            TL::Decode(archive, position);
        }
    };

    struct Model
    {
        Name mesh;
        Name material;
        glm::mat4 transform; // Local transform of the model

        template<typename Archive>
        void Serialize(Archive& archive) const
        {
            TL::Encode(archive, mesh);
            TL::Encode(archive, material);
            TL::Encode(archive, transform);
        }

        template<typename Archive>
        void Deserialize(Archive& archive)
        {
            TL::Decode(archive, mesh);
            TL::Decode(archive, material);
            TL::Decode(archive, transform);
        }
    };

    struct Camera
    {
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        float fov;
        float nearClip;
        float farClip;

        template<typename Archive>
        void Serialize(Archive& archive) const
        {
            TL::Encode(archive, viewMatrix);
            TL::Encode(archive, projectionMatrix);
            TL::Encode(archive, fov);
            TL::Encode(archive, nearClip);
            TL::Encode(archive, farClip);
        }

        template<typename Archive>
        void Deserialize(Archive& archive)
        {
            TL::Decode(archive, viewMatrix);
            TL::Decode(archive, projectionMatrix);
            TL::Decode(archive, fov);
            TL::Decode(archive, nearClip);
            TL::Decode(archive, farClip);
        }
    };

    class ASSETS_EXPORT SceneGraph
    {
    public:
        SceneGraph();

        struct Node;

        using Pool = TL::Vector<Node>;
        using NodeID = Pool::size_type;

        Node* AddNode(Node* parent = nullptr);

        Node* GetNode(NodeID id);

        const Node* GetNode(NodeID id) const;

        Node* GetRootNode();

        TL::Span<Node> GetAllNodes();

        template<typename Archive>
        void Serialize(Archive& archive) const
        {
            TL::Encode(archive, m_rootNode);
            TL::Encode(archive, m_nodes);
        }

        template<typename Archive>
        void Deserialize(Archive& archive)
        {
            TL::Decode(archive, m_rootNode);
            TL::Decode(archive, m_nodes);
        }

    private:
        NodeID m_rootNode;
        Pool m_nodes;
    };

    struct SceneGraph::Node
    {
        TL::Vector<NodeID> children;
        TL::Vector<Light> lights;
        TL::Vector<Model> models;
        TL::Vector<Camera> cameras;

        glm::mat4 relativeTransform = glm::mat4(1.0f); // Identity matrix as the default

        NodeID parent = static_cast<NodeID>(-1); // Invalid parent ID as default

        glm::mat4 GetGlobalTransform(const SceneGraph& sceneGraph) const;

        template<typename Archive>
        void Serialize(Archive& archive) const
        {
            TL::Encode(archive, children);
            TL::Encode(archive, lights);
            TL::Encode(archive, models);
            TL::Encode(archive, cameras);
        }

        template<typename Archive>
        void Deserialize(Archive& archive)
        {
            TL::Decode(archive, children);
            TL::Decode(archive, lights);
            TL::Decode(archive, models);
            TL::Decode(archive, cameras);
        }
    };

    inline SceneGraph::SceneGraph()
    {
        m_nodes.reserve(10000);
    }

    inline TL::Span<SceneGraph::Node> SceneGraph::GetAllNodes()
    {
        return m_nodes;
    }

} // namespace Examples::Assets
