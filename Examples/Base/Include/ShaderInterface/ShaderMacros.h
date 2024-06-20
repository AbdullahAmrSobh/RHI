#pragma once

/*
 * These macro definitions allows the creation of shared structures between C++ and Shader code.
 * Define your shared structure using these macros to ensure compatibility and reduce redundancy.
 */

#ifdef __cplusplus

    #include <glm/glm.hpp>

    #define re_ref(type) type&

    #define re_shader_struct struct alignas(16)
    #define re_float float
    #define re_vec2 alignas(8) glm::vec2
    #define re_vec3 alignas(16) glm::vec3
    #define re_vec4 alignas(16) glm::vec4
    #define re_int int
    #define re_uint unsigned int
    #define re_mat4 alignas(16) glm::mat4
#else // HLSL

    #define re_ref(type) type

    #define re_shader_struct struct
    #define re_float float
    #define re_vec2 float2
    #define re_vec3 float3
    #define re_vec4 float4
    #define re_int int
    #define re_uint uint
    #define re_mat4 float4x4
#endif

#ifndef re_namespace_name
    #define re_namespace_name Shader
#endif
