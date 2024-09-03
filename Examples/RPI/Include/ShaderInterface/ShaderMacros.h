#pragma once

/*
 * These macro definitions allows the creation of shared structures between C++ and Shader code.
 * Define your shared structure using these macros to ensure compatibility and reduce redundancy.
 */

#ifdef __cplusplus
    #include <glm/glm.hpp>

    #define re_alignas(x) alignas(sizeof(x))

// struct re_alignas(glm::vec2) re_vec2 : glm::vec2 {  using base = glm::vec2; using base::base; };
// struct re_alignas(glm::vec4) re_vec3 : glm::vec3 {  using base = glm::vec3; using base::base; };
// struct re_alignas(glm::vec4) re_vec4 : glm::vec4 {  using base = glm::vec4; using base::base; };

// struct re_alignas(glm::mat2) re_mat2 : glm::mat2 {  using base = glm::mat2; using base::base; };
// struct re_alignas(glm::mat4) re_mat3 : glm::mat3 {  using base = glm::mat3; using base::base; };
// struct re_alignas(glm::mat4) re_mat4 : glm::mat4 {  using base = glm::mat4; using base::base; };

    #define re_vec2 re_alignas(glm::vec2) glm::vec2
    #define re_vec3 re_alignas(glm::vec4) glm::vec3
    #define re_vec4 re_alignas(glm::vec4) glm::vec4
    #define re_mat2 re_alignas(glm::vec4) glm::mat2
    #define re_mat3 re_alignas(glm::vec4) glm::mat3
    #define re_mat4 re_alignas(glm::vec4) glm::mat4

    #define re_struct struct re_alignas(16)

#else // slang

typedef vector<float, 2> re_vec2;
typedef vector<float, 3> re_vec3;
typedef vector<float, 4> re_vec4;

typedef matrix<float, 2, 2> re_mat2;
typedef matrix<float, 3, 3> re_mat3;
typedef matrix<float, 4, 4> re_mat4;

#define re_struct struct

#endif

#ifndef re_namespace_name
    #define re_namespace_name Shader
#endif
