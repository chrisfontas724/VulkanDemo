#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "sampling/sampling.comp"

struct Payload {
  mwc64x_state_t seed;
  vec3 hitValue;
  vec3 hitWeight;
  vec3 origin;
  vec3 direction;
  bool alive;
};

layout(location = 0) rayPayloadEXT Payload payload;

void main()
{
    payload.hitValue = vec3(0.0, 0.0, 0.0);
    payload.hitWeight = vec3(0.0, 0.0, 0.0);
    payload.alive = false;
}