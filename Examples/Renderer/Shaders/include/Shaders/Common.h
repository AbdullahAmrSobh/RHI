#pragma once

#ifdef __cplusplus
    #include <glm/glm.hpp>

    #define F32 float
    #define F32_2 alignas(sizeof(glm::vec2)) glm::vec2
    #define F32_3 alignas(sizeof(glm::vec4)) glm::vec3
    #define F32_4 alignas(sizeof(glm::vec4)) glm::vec4

    #define I32 int32_t
    #define I32_2 alignas(sizeof(glm::ivec2)) glm::ivec2
    #define I32_3 alignas(sizeof(glm::ivec4)) glm::ivec3
    #define I32_4 alignas(sizeof(glm::ivec4)) glm::ivec4

    #define U32 uint32_t
    #define U32_2 alignas(sizeof(glm::uvec2)) glm::uvec2
    #define U32_3 alignas(sizeof(glm::uvec4)) glm::uvec3
    #define U32_4 alignas(sizeof(glm::uvec4)) glm::uvec4

    #define F32_2x2 alignas(sizeof(glm::vec4)) glm::mat2x2
    #define F32_2x3 alignas(sizeof(glm::vec4)) glm::mat2x3
    #define F32_2x4 alignas(sizeof(glm::vec4)) glm::mat2x4
    #define F32_3x2 alignas(sizeof(glm::vec4)) glm::mat3x2
    #define F32_3x3 alignas(sizeof(glm::vec4)) glm::mat3x3
    #define F32_3x4 alignas(sizeof(glm::vec4)) glm::mat3x4
    #define F32_4x2 alignas(sizeof(glm::vec4)) glm::mat4x2
    #define F32_4x3 alignas(sizeof(glm::vec4)) glm::mat4x3
    #define F32_4x4 alignas(sizeof(glm::vec4)) glm::mat4x4

#else
    #define F32 float
    #define F32_2 float2
    #define F32_3 float3
    #define F32_4 float4

    #define I32 int
    #define I32_2 int2
    #define I32_3 int3
    #define I32_4 int4

    #define U32 uint
    #define U32_2 uint2
    #define U32_3 uint3
    #define U32_4 uint4

    #define F32_2x2 float2x2
    #define F32_2x3 float2x3
    #define F32_2x4 float2x4
    #define F32_3x2 float3x2
    #define F32_3x3 float3x3
    #define F32_3x4 float3x4
    #define F32_4x2 float4x2
    #define F32_4x3 float4x3
    #define F32_4x4 float4x4
#endif


#ifndef __cplusplus
    #define POSITION : SV_Position
    #define TEXCOORD(x) : TEXCOORD##x
#else
    #define POSITION
    #define TEXCOORD(x)
#endif
