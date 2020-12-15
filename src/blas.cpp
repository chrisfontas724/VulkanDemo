#include <vulkan/vulkan.hpp>


//--------------------------------------------------------------------------------------------------
// Convert an OBJ model into the ray tracing geometry used to build the BLAS
//
nvvk::RaytracingBuilderKHR::BlasInput HelloVulkan::objectToVkGeometryKHR(const ObjModel& model) {
  // BLAS builder requires raw device addresses.
  vk::DeviceAddress vertexAddress = m_device.getBufferAddress({model.vertexBuffer.buffer});
  vk::DeviceAddress indexAddress  = m_device.getBufferAddress({model.indexBuffer.buffer});

  uint32_t maxPrimitiveCount = model.nbIndices / 3;

  // Describe buffer as array of VertexObj.
  vk::AccelerationStructureGeometryTrianglesDataKHR triangles;
  triangles.setVertexFormat(vk::Format::eR32G32B32Sfloat); // vec3 vertex position data.
  triangles.setVertexData(vertexAddress);
  triangles.setVertexStride(sizeof(VertexObj));
  // Describe index data (32-bit unsigned int)
  triangles.setIndexType(vk::IndexType::eUint32);
  triangles.setIndexData(indexAddress);
  // Indicate identity transform by setting transformData to null device pointer.
  triangles.setTransformData({});
  triangles.setMaxVertex(model.nbVertices);

  // Identify the above data as containing opaque triangles.
  vk::AccelerationStructureGeometryKHR asGeom;
  asGeom.setGeometryType(vk::GeometryTypeKHR::eTriangles);
  asGeom.setFlags(vk::GeometryFlagBitsKHR::eOpaque);
  asGeom.geometry.setTriangles(triangles);

  // The entire array will be used to build the BLAS.
  vk::AccelerationStructureBuildRangeInfoKHR offset;
  offset.setFirstVertex(0);
  offset.setPrimitiveCount(maxPrimitiveCount);
  offset.setPrimitiveOffset(0);
  offset.setTransformOffset(0);

  // Our blas is made from only one geometry, but could be made of many geometries
  nvvk::RaytracingBuilderKHR::BlasInput input;
  input.asGeometry.emplace_back(asGeom);
  input.asBuildOffsetInfo.emplace_back(offset);

  return input;
}