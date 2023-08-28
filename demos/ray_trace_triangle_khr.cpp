// Copyright 2023 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.


#include "ray_trace_triangle_khr.hpp"
#include <VulkanWrappers/acceleration_structure.hpp>


void RayTraceTriangleKHR::setup(gfx::LogicalDevicePtr logical_device, int32_t num_swap, int32_t width, int32_t height) {
    CXL_DCHECK(logical_device);
    std::vector<float> positions = {552.8,   0.0,   0.0,
                                    549.6,   0.0, 559.2,
                                    556.0, 548.8, 559.2};
    std::vector<uint32_t> indices = {0,1,2};                                

    gfx::Geometry geometry; 
    vk::BufferUsageFlags flags = vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
    geometry.attributes[gfx::VTX_POS] = gfx::ComputeBuffer::createFromVector(logical_device, positions, flags);
    geometry.indices = gfx::ComputeBuffer::createFromVector(logical_device, indices, flags);
    geometry.flags = gfx::VTX_POS_FLAG;
    geometry.num_indices = 6;
    geometry.num_vertices = 4;
    geometry.identifier = 5;
    as_ = std::make_shared<gfx::AccelerationStructure>(logical_device);
    as_->buildTopLevel({geometry.identifier}, {geometry});
    CXL_DCHECK(as_);
    CXL_LOG(INFO) << "Built AS!!";
}

void RayTraceTriangleKHR::resize(uint32_t width, uint32_t height) {

}

gfx::ComputeTexturePtr
RayTraceTriangleKHR::renderFrame(gfx::CommandBufferPtr command_buffer, 
                uint32_t image_index, 
                uint32_t frame,
                std::vector<vk::Semaphore>* signal_semaphores,
                std::vector<vk::PipelineStageFlags>* signal_wait_stages) {
    return nullptr;
}