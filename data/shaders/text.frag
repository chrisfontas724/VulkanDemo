layout(location = 0) in vec2 uvCoords;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D bitmap;

// layout(std140, push_constant) uniform PushConstant {
//   layout(offset=64) vec4 color;
// };


void main() {
  outColor = texture(bitmap, uvCoords);
}