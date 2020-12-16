layout(location = 0) in vec4 in_color;
layout(location = 1) in vec2 in_uvs;
layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform sampler2D image;

void main() {
    out_color = in_color * texture(image, in_uvs);
}