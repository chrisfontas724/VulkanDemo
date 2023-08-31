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
    bool alive;
};

layout(buffer_reference, scalar) buffer ObjMaterial { Material m; }; // Current object material
layout(set = 2, binding = 0, scalar) buffer ObjDesc_ { ObjDesc i[]; } objDesc;


layout(location = 0) rayPayloadInEXT Payload payload;


void main()
{
    ObjDesc  objResource = objDesc.i[gl_InstanceCustomIndexEXT];
    ObjMaterial material  = ObjMaterial(objResource.materialAddress);

    vec3 localPos = gl_ObjectRayOriginEXT + gl_ObjectRayDirectionEXT * gl_HitTEXT;
    vec3 worldCenter = vec3(gl_ObjectToWorldEXT * vec4(0,0,0,1));
    vec3 worldPos = vec3(gl_ObjectToWorldEXT * vec4(localPos, 1.0));
	vec3 worldNrm = normalize(worldPos - worldCenter);

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
}