#version 450

out gl_PerVertex {
	vec4 gl_Position;
};

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) in vec3 inVertPosition;
layout(location = 1) in vec3 inVertColor;

layout(location = 0) out vec3 outPixelColor;

void vertexFn() {
}

void pixelFn() {
}