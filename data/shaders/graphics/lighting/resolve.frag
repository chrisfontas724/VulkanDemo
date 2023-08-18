#version 450
precision highp float;
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_separate_shader_objects : enable

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput input_color;

layout(location = 0) out vec4 out_color;

layout(std140, push_constant) uniform PushBlock {
    layout(offset=0) int num_samples;
};

void main() {
   out_color = vec4(subpassLoad(input_color).rgb / vec3(num_samples), 1.0);
}