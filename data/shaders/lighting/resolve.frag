

layout (set = 0, binding = 0) uniform sampler2D input_color;

layout(location = 0) in vec2 uv_coord;
layout(location = 0) out vec4 out_color;

layout(std140, push_constant) uniform PushBlock {
    layout(offset=0) int num_samples;
};

void main() {
   out_color = vec4(texture(input_color, uv_coord).xyz / vec3(num_samples), 1.0);
}