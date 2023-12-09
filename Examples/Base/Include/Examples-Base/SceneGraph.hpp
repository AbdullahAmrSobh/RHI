#pragma once

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>

#include "Examples-Base/Camera.hpp"

#include <vector>

struct AABB
{
    glm::vec3 min, max;
};

template<typename Data>
class SceneNode
{
public:
    Data m_content;

    // transformation relative to the parent node.
    glm::mat4 m_relativeTransformation;

    // world space transformation
    glm::mat4 m_absoluteTransformation;

    // Axis aligned bounding box (AABB) used during frustum culling
    AABB m_aabb;

    // indicats if the current node is occluded by another object.
    bool m_isCulled;

    // indicats if the current node and all of its children are occluded by another object.
    bool m_isAllChildrenCulled;

    // indicates that the current node is marked as hidden and should not be renrederd.
    bool m_isHidden;
};

class SceneNodeStorage {};

class Scene
{
public:
    // Preforms orthographic frustum culling on the scene graph
    void FrustumCullOrthographic(float left, float right, float bottom, float top, float near, float far);

    // Preforms perspective frustum culling on the scene graph
    void FrustumCullPerspective(float fovy, float aspect, float near);

    // Updates the transformation of all nodes in the scene graph
    void UpdateTransformation();

private:
    // Pointer to the active scene camera.
    SceneNode<Camera>* m_activeCamera;

    // A collection of all objects in the scene.
    std::vector<SceneNodeStorage> m_sceneGraph;
};
