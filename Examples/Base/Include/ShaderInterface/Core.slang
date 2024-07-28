#include "ShaderInterface/ShaderMacros.h"

#define re_max_point_lights_count 16

re_shader_struct SceneTransform
{
    re_mat4 viewProjectionMatrix;
    re_mat4 projectionMatrix;
    re_mat4 viewMatrix;
    re_mat4 inverseViewMatrix;
};

re_shader_struct DirectionalLight
{
    re_vec3 direction;
    re_float intensity;

    re_vec3 ambientColor;
    re_vec3 diffuseColor;
    re_vec3 specularColor;
};

re_shader_struct PointLight
{
    re_vec3 position;
    re_float intensity;
    re_vec3 color;
    re_float radius;

    re_vec3 ambientColor;
    re_vec3 diffuseColor;
    re_vec3 specularColor;
    re_float attuation;
};

re_shader_struct SpotLight
{
    re_vec3 position;
    re_float radius;
    re_vec3 direction;
    re_float innerAngle;
    re_vec3 ambientColor;
    re_vec3 specularColor;
    re_vec3 diffuseColor;
    re_float attuation;
};

re_shader_struct ObjectTransform
{
    re_mat4 modelMatrix;
};