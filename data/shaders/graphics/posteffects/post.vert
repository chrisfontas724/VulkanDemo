#version 450
precision highp float;
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_separate_shader_objects : enable

// Harded values for a fullscreen quad in clip space coordinates.
// Instead of actually rendering a quad, we render an oversized triangle
// that fits over the rendered screen.
vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0, 3.0),
    vec2(3.0, -1.0)
);

vec2 uv_coords[3] = vec2[](
    vec2(0, 0),
    vec2(0, 2),
    vec2(2, 0)
);

layout(location = 0) out vec2 uv_coord;

// Simply write out the position with no transformation.
void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    uv_coord = uv_coords[gl_VertexIndex];
}