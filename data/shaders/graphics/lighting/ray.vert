#version 450
precision highp float;
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_separate_shader_objects : enable

#include "types/ray.comp"
#include "types/intersection.comp"

layout(std430, set = 0, binding = 0) buffer buf {
    Ray rays[];
};


// Outputs
layout(location = 0) out vec4 accumulation;


// Simply write out the position with no transformation.
void main() {
    Ray ray = rays[gl_VertexIndex];

    accumulation = ray.accumulation;

    gl_Position = vec4(ray.coord, 1.0, 1.0);
    gl_PointSize = 1.0;
}