layout(location = 0) in vec2 uv_coord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D scene_texture;
void main() {
   vec3 tex_col = max(vec3(0), texture(scene_texture, uv_coord).rgb);
   float lum = dot(tex_col, vec3(.3, .6, .1));
   outColor = vec4(vec3(lum), 1.0);
}