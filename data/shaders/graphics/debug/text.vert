#version 450
precision highp float;
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_separate_shader_objects : enable

// This shader avoids the need for binding index and vertex buffers; this array
// plays a key role.
//
// Index buffers are avoided by using vkCmdDraw() instead of vkCmdDrawIndexed().
// This array is used to remap gl_VertexIndex into the range [0-3], because a
// rectangle has 4 vertices, not 6.
//
// Vertex buffers are avoided by using the remapped index to look up the
// position/UV coords corresponding to the current vertex. Position coords
// are hardcoded into the shader, and UV coords are provided as push constants.
uint remapped_indices[6] = uint[](
  0u,2u,1u,0u,3u,2u
);

// Push constants needed to render each glyph.
layout(push_constant) uniform PushConstant {
  // Vertex positions in clockwise order, starting at top left.
  layout(offset=0) vec2 positions[4];
  
  // UV coords in clockwise order, starting at top left.
  layout(offset=32) vec2 uvs[4];
};

// UV Coordinates used for the glyph bitmap in
// the fragment shader.
layout(location = 0) out vec2 outUVCoords;

// Use gl_VertexIndex to index into the hardcoded rectangle
// positions array. These positions are then transformed
// into clip-space based on the PushConstant data for a
// particular rectangle instance.
void main() {
    const uint index = remapped_indices[gl_VertexIndex];

    // We copy to a local array because gl_VertexIndex isn't
    // dynamically uniform, therefore neither is the remapped index,
    // so it can't be used as an index into the push constant array.
    vec2 local_positions[4] = vec2[](positions[0], positions[1], positions[2], positions[3]);
    gl_Position = vec4(local_positions[index], 0.0, 1.0);

    vec2 local_uvs[4] = vec2[](uvs[0], uvs[1], uvs[2], uvs[3]);
    outUVCoords = local_uvs[index];
}