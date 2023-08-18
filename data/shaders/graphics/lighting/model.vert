#version 450
precision highp float;
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_uvs;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec2 out_uvs;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    out_color = in_color;
    out_uvs = in_uvs;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_position.xyz, 1.0);
}