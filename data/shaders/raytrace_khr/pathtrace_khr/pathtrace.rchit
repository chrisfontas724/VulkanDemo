#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_ARB_separate_shader_objects : enable

#include "sampling/sampling.comp"

// Information of a obj model when referenced in a shader
struct ObjDesc  {
  int      txtOffset;             // Texture index offset in the array of textures
  uint64_t vertexAddress;         // Address of the Vertex buffer
  uint64_t indexAddress;          // Address of the index buffer
  uint64_t materialAddress;       // Address of the material buffer
  uint64_t materialIndexAddress;  // Address of the triangle material index buffer
};

struct Material {
  vec4 diffuse_color;
  vec4 emissive_color;
};


struct Payload{
  mwc64x_state_t seed;
  vec3 hitValue;
  vec3 hitWeight;
  vec3 origin;
  vec3 direction;
};

layout(location = 0) rayPayloadInEXT Payload payload;


layout(buffer_reference, scalar) buffer ObjMaterial { Material m; }; // Current object material
layout(buffer_reference, scalar) buffer Indices { int i[]; };       // Triangle indices
layout(buffer_reference, scalar) buffer Vertices { float v[]; };       // Positions of an object


layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 1, binding = 0, scalar) buffer ObjDesc_ { ObjDesc i[]; } objDesc;


hitAttributeEXT vec2 attribs;

void main()
{
  ObjDesc  objResource = objDesc.i[gl_InstanceCustomIndexEXT];
  ObjMaterial material  = ObjMaterial(objResource.materialAddress);
  Indices    indices     = Indices(objResource.indexAddress);
  Vertices   vertices    = Vertices(objResource.vertexAddress);

  // Indices of the triangle
  int ind = indices.i[gl_PrimitiveID * 3];
  int ind2 = indices.i[gl_PrimitiveID * 3 + 1];
  int ind3 = indices.i[gl_PrimitiveID * 3 + 2];

  // Vertex of the triangle
  vec3 v0 = vec3(vertices.v[3*ind], vertices.v[3*ind + 1], vertices.v[3*ind + 2]);
  vec3 v1 = vec3(vertices.v[3*ind2], vertices.v[3*ind2 + 1], vertices.v[3*ind2 + 2]);
  vec3 v2 = vec3(vertices.v[3*ind3], vertices.v[3*ind3 + 1], vertices.v[3*ind3 + 2]);

  const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

  // Computing the coordinates of the hit position
  const vec3 pos      = v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
  const vec3 worldPos = vec3(gl_ObjectToWorldEXT * vec4(pos, 1.0));  // Transforming the position to world space

  // Computing the normal at hit position
  //const vec3 nrm      = v0.nrm * barycentrics.x + v1.nrm * barycentrics.y + v2.nrm * barycentrics.z;
  const vec3 nrm = normalize(cross(v0-v1, v0-v2));
  const vec3 worldNrm = normalize(vec3(nrm * gl_WorldToObjectEXT));  // Transforming the normal to world space


  // Calculate new ray here
  float xi1 = uniformRandomVariable(payload.seed); 
  float xi2 = uniformRandomVariable(payload.seed); 

  float theta = acos(sqrt(1.0 -xi1));
  float phi = 2.0 * 3.14159265 * xi2;

  float xs = sin(theta) * cos(phi);
  float ys = cos(theta);
  float zs = sin(theta) * sin(phi);

  vec3 y = vec3(worldNrm.xyz);
  vec3 h = y;

  if (abs(h.x) <= abs(h.y) && abs(h.x) <= abs(h.z)) {
      h.x = 1.0;
  } else if (abs(h.y) <= abs(h.x) && abs(h.y) <= abs(h.z)) {
      h.y = 1.0;
  } else {
      h.z = 1.0;
  }

  vec3 x = normalize(cross(h,y));
  vec3 z = normalize(cross(x,y));

  vec3 new_dir = normalize(xs*x + ys*y + zs*z);
  float pdf = dot(new_dir, worldNrm.xyz) / 3.14159265;

  vec3 new_pos = worldPos + 0.001 * new_dir;

  // Add a small epsilon to the new ray starting point to prevent self-intersection
  // with the object its already on.
  payload.hitValue += payload.hitWeight * material.m.emissive_color.xyz;

  // The new weight is BRDF * cosTheta / pdf.
  vec3 brdf = material.m.diffuse_color.xyz / vec3(3.14159265);
  float cos_theta = dot(new_dir.xyz, worldNrm.xyz);
  payload.hitWeight *= brdf * cos_theta / pdf;

  payload.origin = new_pos;
  payload.direction = new_dir;

  float tmin     = 0.001;
  float tmax     = 10000.0;

  //payload.hitValue = material.m.diffuse_color.xyz * abs(dot(worldNrm, gl_WorldRayDirectionEXT));
  // traceRayEXT(topLevelAS, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, new_pos, tmin, new_dir, tmax, 0);
}
