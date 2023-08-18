#version 450
precision highp float;
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_separate_shader_objects : enable

// Outputs
layout (location = 0) in vec4 accumulation;

layout(location = 0) out vec4 outColor;


// Simply write out the position with no transformation.
void main() {
    outColor = accumulation;
}