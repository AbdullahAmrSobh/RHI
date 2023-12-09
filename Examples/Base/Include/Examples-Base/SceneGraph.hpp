#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

struct AABB
{
    glm::vec3 min, max;
};

template<typename Data>
class SceneNode
{
public:
    AABB m_aabb;
    glm::mat4 m_transformation;

    uint32_t m_numChildrenNodes;
    SceneNode** children;

    Data m_content;
};



struct Material
{
    uint32_t materialId;


};


struct Mesh
{
    
};