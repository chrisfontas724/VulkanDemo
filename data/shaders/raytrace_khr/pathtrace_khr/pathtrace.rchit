#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

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


layout(location = 0) rayPayloadInEXT vec3 hitValue;


layout(buffer_reference, scalar) buffer ObjMaterial { Material m; }; // Current object material

layout(set = 1, binding = 0, scalar) buffer ObjDesc_ { ObjDesc i[]; } objDesc;


hitAttributeEXT vec2 attribs;

void main()
{

  ObjDesc  objResource = objDesc.i[gl_InstanceCustomIndexEXT];
  ObjMaterial material  = ObjMaterial(objResource.materialAddress);
  hitValue = material.m.diffuse_color.xyz; 
}
