layout(location = 0) in vec2 uvCoords;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D bitmap;

layout(std140, push_constant) uniform PushConstantFragment {
  layout(offset=64) vec4 color;
};


void main() {
  outColor = color * texture(bitmap, uvCoords);
}