#include "Assets/SceneGraph.hpp"

namespace Examples::Assets
{
    SceneGraph::Node* SceneGraph::AddNode(Node* parent)
    {
        NodeID newID = m_nodes.size();
        m_nodes.emplace_back();

        Node* newNode = &m_nodes.back();
        newNode->parent = (parent) ? static_cast<NodeID>(parent - &m_nodes[0]) : m_rootNode;

        if (parent)
        {
            parent->children.push_back(newID);
        }
        else if (m_nodes.size() == 1)
        {
            m_rootNode = newID;
        }

        return newNode;
    }

    SceneGraph::Node* SceneGraph::GetNode(NodeID id)
    {
        if (id >= m_nodes.size())
            return nullptr;
        return &m_nodes[id];
    }

    const SceneGraph::Node* SceneGraph::GetNode(NodeID id) const
    {
        if (id >= m_nodes.size())
            return nullptr;
        return &m_nodes[id];
    }

    SceneGraph::Node* SceneGraph::GetRootNode()
    {
        return GetNode(m_rootNode);
    }

    glm::mat4 SceneGraph::Node::GetGlobalTransform(const SceneGraph& sceneGraph) const
    {
        glm::mat4 globalTransform = relativeTransform;
        NodeID currentParent = parent;

        while (currentParent != static_cast<NodeID>(-1))
        {
            const Node* parentNode = sceneGraph.GetNode(currentParent);
            globalTransform = parentNode->relativeTransform * globalTransform;
            currentParent = parentNode->parent;
        }

        return globalTransform;
    }
} // namespace Examples::Assets
